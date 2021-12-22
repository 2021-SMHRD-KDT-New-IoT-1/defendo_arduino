#include <MQUnifiedsensor.h>
#include <SoftwareSerial.h>

#define         Board                   ("Arduino UNO")
#define         Pin                     (A0)  //Analog input 4 of your arduino
#define         Type                    ("MQ-9") //MQ9
#define         Voltage_Resolution      (5)
#define         ADC_Bit_Resolution      (10) // For arduino UNO/MEGA/NANO
#define         RatioMQ9CleanAir        (9.6) //RS / R0 = 60 ppm

MQUnifiedsensor MQ9(Board, Voltage_Resolution, ADC_Bit_Resolution, Pin, Type);

//SoftwareSerial gps(10, 12);

const String SSID = "SMHRD_강의실A";
const String PASSWORD = "aaaaa11111";
const String SERVER_IP = "172.30.1.57";
const String SERVER_PORT = "8085";

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

void setup() {
  Serial.begin(9600);
  mqSetup();
  Serial.println("Start module connection");  // 출력
  do {
    Serial2.begin(9600);  // 와이파이 모듈
    // ESP8266 모듈 재시작
    Serial2.println("AT+RST");  // 와이파이 모듈에게 다시 시작하겠다고 명령
    delay(1000);
    // 만약 재시작되었다면
    if (Serial2.find("ready")) {
      Serial.println("Module ready");
      // ESP8266 모듈의 모드를 듀얼모드로 설정 (클라이언트)
      Serial2.println("AT+CWMODE=1");
      delay(1000);
      // AP에 접속되면
      if (cwJoinAP()) {
        Serial.println("AP successful");
        FAIL_8266 = false;
        delay(1000);
        Serial.println("Start buffer initialization");
        while (Serial2.available() > 0) {
          char a = Serial2.read();
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
  Serial1.begin(9600);
}

float LatF = 0.00000;
float LongF = 0.00000;
String r_LatF = "";
String r_LongF = "";

int sound_delay = 0;
String r_alram = "";
String r_attack = "";
int alram = 0;
String lock = "0";

void loop() {
  boolean gps_check = true;

  int attack = digitalRead(11);
  int btn = digitalRead(10);

  MQ9.update();
  float LPG = getLPG();
  float CH4 = getCHfour();
  float CO = getCO();

  Serial.print("LPG : "); Serial.println(LPG);
  Serial.print("CH4 : "); Serial.println(CH4);
  Serial.print("Co : "); Serial.println(CO);
  delay(500);

  if (attack == 1) {
    sori = true;
  }
  if (LPG >= 1000 || CH4 >= 5000 || CO >= 7000) {
    alram = 1;
    sori = true;
  }
  if (btn == 1) {
    sori = false;
    alram = 0;
  }
  sound();
  while (gps_check) {
    if (Serial1.available()) {
      c = Serial1.read();
      if (c == '\n') {
        if (targetStr.equals(str.substring(1, 6))) {
          int first = str.indexOf(",");
          int two = str.indexOf(",", first + 1);
          int three = str.indexOf(",", two + 1);
          int four = str.indexOf(",", three + 1);
          int five = str.indexOf(",", four + 1);


          float LatF = getLat(two, three);
          float LongF = getLong(four, five);
          
          Serial.print("위도 : ");
          Serial.println(LatF, 5);
          Serial.print("경도 : ");
          Serial.println(LongF, 5);
          gps_check = false;

          r_LatF = String(LatF, 5); 
          r_LongF = String(LongF, 5);
        }
        str = "";
      } else {
        str += c;
      }
    }
  }
  r_attack = String(attack);
  r_alram = String(alram);

  sendDataToServer();

  //  Serial.println(r_attack);
  //  Serial.println(r_alram);
  Serial.println(r_LatF);
  Serial.println(r_LongF);
}

void sound() {
  if (sori) {
    sound_delay++;
    if (sound_delay % 3 != 0) {
      tone(12, 1000, 1000);
    }
    else if (sound_delay % 3 == 0) {
      noTone(12);
    }
  }
  else if (!sori) {
    noTone(12);
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
  Serial2.println(cmd);
  // 웹 서버에 접속되면
  if (Serial2.find("OK")) {
    Serial.println("Server connection successful");
  } else {
    Serial.println("Server connection failed");
  }

  // 서버로 GET 메시지 전송
  cmd = "GET /Test02/test?attack=" + r_attack;
  cmd += "&alram=" + r_alram;
  cmd += "&r_LatF=" + r_LatF;
  cmd += "&r_LongF=" + r_LongF; // 요청하는 url //보내는거
  cmd += "&lock=" + lock;
  cmd += "\r\nConnection: close\r\n\r\n";

  Serial.println(cmd);
  Serial2.print("AT+CIPSEND=");
  Serial2.println(cmd.length());
  if (Serial2.find("OK")) {
    Serial.println("Ready to send to server");
  } else {
    Serial.println("Failed to prepare to send to server");
  }
  Serial2.println(cmd);

  //데이터 전송이 완료되면
  if (Serial2.find("OK")) {
    Serial.println("Data transfer successful");
  } else {
    Serial.println("Data transfer failed");
    sendDataToServer();
  }
  delay(1000);
}

boolean cwJoinAP() {
  String cmd = "AT+CWJAP=\"" + SSID + "\",\"" + PASSWORD + "\"";
  Serial2.println(cmd);
  if (Serial2.find("OK")) {
    return true;
  } else {
    return false;
  }
}
