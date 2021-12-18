#include <MQUnifiedsensor.h>
#include <SoftwareSerial.h>

#define         Board                   ("Arduino UNO")
#define         Pin                     (A0)  //Analog input 4 of your arduino
#define         Type                    ("MQ-9") //MQ9
#define         Voltage_Resolution      (5)
#define         ADC_Bit_Resolution      (10) // For arduino UNO/MEGA/NANO
#define         RatioMQ9CleanAir        (9.6) //RS / R0 = 60 ppm 

MQUnifiedsensor MQ9(Board, Voltage_Resolution, ADC_Bit_Resolution, Pin, Type);

const String SSID = "JDG";
const String PASSWORD = "12345678";
const String SERVER_IP = "218.149.140.27";
const String SERVER_PORT = "8087";

boolean sori = false;
char c = "";
String str = "";
String targetStr = "GPGGA";

// AT 명령 저장
String cmd = "";
// 전송 데이터 저장
String sendData = "";
// WiFi 접속 실패 여부
boolean FAIL_8266 = false;

SoftwareSerial esp(2, 3); // TX, RX
SoftwareSerial gps(4, 5);

void setup() {
  Serial.begin(9600);
  gps.begin(9600);

  Serial.println("Start module connection");  // 출력
  do {
    esp.begin(9600);  // 와이파이 모듈
    // ESP8266 모듈 재시작
    esp.println("AT+RST");  // 와이파이 모듈에게 다시 시작하겠다고 명령
    delay(1000);
    // 만약 재시작되었다면
    if (esp.find("ready")) {
      Serial.println("Module ready");
      // ESP8266 모듈의 모드를 듀얼모드로 설정 (클라이언트)
      esp.println("AT+CWMODE=1");
      delay(1000);
      // AP에 접속되면
      if (cwJoinAP()) {
        Serial.println("AP successful");
        FAIL_8266 = false;
        delay(1000);
        Serial.println("Start buffer initialization");
        while (esp.available() > 0) {
          char a = esp.read();
          Serial.write(a);
        }
        Serial.println();
        Serial.println("Buffer initialization terminated");
      } else {
        Serial.println("AP connection failure");
        delay(1000);
        FAIL_8266 = true;
      }
    } else {
      Serial.println("Module connection failure");
      delay(1000);
      FAIL_8266 = true;
    }
  } while (FAIL_8266);
  Serial.println("Module connection complete");

  mqSetup();
}

int sound_delay = 0;

void loop() {
  int attack = digitalRead(8);
  int btn = digitalRead(6);

  MQ9.update();
  float LPG = getLPG();
  float CH4 = getCHfour();
  float CO = getCO();

  if (attack == 1) {
    sori = true;
  }
  if (LPG >= 1000 || CH4 >= 5000 || CO >= 7000) {
    sori = true;
  }
  if (btn == 1) {
    sori = false;
  }
  sound();

  if (gps.available()) {
    c = gps.read();
    if (c == '\n') {
      if (targetStr.equals(str.substring(1, 6))) {
        int first = str.indexOf(",");
        int two = str.indexOf(",", first + 1);
        int three = str.indexOf(",", two + 1);
        int four = str.indexOf(",", three + 1);
        int five = str.indexOf(",", four + 1);

        float r_LatF = getLat(two, three);
        float r_LongF = getLong(four, five);

        Serial.print("위도 : ");
        Serial.println(r_LatF, 15);
        Serial.print("경도 : ");
        Serial.println(r_LongF, 15);

        float LatF = (r_LatF, 15);
        float LongF = (r_LongF, 15);

        sendDataToServer();
      }
      str = "";
    } else {
      str += c;
    }
  }
}

void sound() {
  if (sori) {
    sound_delay++;
    Serial.println(sound_delay);
    if (sound_delay % 3 != 0) {
      tone(13, 1000, 1000);
    }
    else if (sound_delay % 3 == 0) {
      noTone(13);
    }
  }
  else if (!sori) {
    noTone(13);
  }
}

void mqSetup() {
  MQ9.setRegressionMethod(1);
  MQ9.init();
  Serial.print("Calibrating please wait.");
  float calcR0 = 0;
  for (int i = 1; i <= 10; i ++)
  {
    MQ9.update();
    calcR0 += MQ9.calibrate(RatioMQ9CleanAir);
    Serial.print(".");
  }
  MQ9.setR0(calcR0 / 10);
  Serial.println("  done!.");
  if (isinf(calcR0)) {
    Serial.println("Warning...");
    while (1);
  }
  if (calcR0 == 0) {
    Serial.println("Warning...");
    while (1);
  }
}

float getLPG() {
  MQ9.setA(1000.5);
  MQ9.setB(-2.186);
  float LPG = MQ9.readSensor();
  return LPG;
}

float getCHfour() {
  MQ9.setA(4269.6);
  MQ9.setB(-2.648);
  float CH4 = MQ9.readSensor();
  return CH4;
}

float getCO() {
  MQ9.setA(599.65);
  MQ9.setB(-2.244);
  float CO = MQ9.readSensor();
  return CO;
}

float getLat(int two, int three) {
  String Lat = str.substring(two + 1, three);
  String Lat1 = Lat.substring(0, 2);
  String Lat2 = Lat.substring(2);
  float LatF = Lat1.toDouble() + Lat2.toDouble() / 60;
  return LatF;
}

float getLong(int four, int five) {
  String Long = str.substring(four + 1, five);
  String Long1 = Long.substring(0, 3);
  String Long2 = Long.substring(3);
  float LongF = Long1.toFloat() + Long2.toFloat() / 60;
  return LongF;
}

void sendDataToServer() {
  Serial.println("Start the data transfer part");
  cmd = "AT+CIPSTART=\"TCP\",\"" + SERVER_IP + "\"," + SERVER_PORT + "\r\n";
  Serial.println("Attempt to connect to server");
  esp.println(cmd);
  // 웹 서버에 접속되면
  if (esp.find("OK")) {
    Serial.println("Server connection successful");
  } else {
    Serial.println("Server connection failed");
  }

  // 서버로 GET 메시지 전송
  cmd = "GET /ArduinoServer/GetSensor"; // 요청하는 url //보내는거
  cmd += "\r\nConnection: close\r\n\r\n";

  Serial.println(cmd);
  esp.print("AT+CIPSEND=");
  esp.println(cmd.length());
  if (esp.find("OK")) {
    Serial.println("Ready to send to server");
  } else {
    Serial.println("Failed to prepare to send to server");
  }
  esp.println(cmd);

  //데이터 전송이 완료되면
  if (esp.find("OK")) {
    Serial.println("Data transfer successful");
  } else {
    Serial.println("Data transfer failed");
    sendDataToServer();
  }
  delay(1000);
}

boolean cwJoinAP() {
  String cmd = "AT+CWJAP=\"" + SSID + "\",\"" + PASSWORD + "\"";
  esp.println(cmd);
  if (esp.find("OK")) {
    return true;
  } else {
    return false;
  }
}
