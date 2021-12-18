#include <MQUnifiedsensor.h>

#define         Board                   ("Arduino UNO")
#define         Pin                     (A0)  //Analog input 4 of your arduino
#define         Type                    ("MQ-9") //MQ9
#define         Voltage_Resolution      (5)
#define         ADC_Bit_Resolution      (10) // For arduino UNO/MEGA/NANO
#define         RatioMQ9CleanAir        (9.6) //RS / R0 = 60 ppm 

MQUnifiedsensor MQ9(Board, Voltage_Resolution, ADC_Bit_Resolution, Pin, Type);

boolean sori = false;

void setup() {
  Serial.begin(9600);
  mqSetup();
}

void loop() {
  int val = digitalRead(9);
  int btn = digitalRead(5);

  MQ9.update();
  float LPG = getLPG();
  float CH4 = getCHfour();
  float CO = getCO();
  Serial.print("LPG : "); Serial.println(LPG);
  Serial.print("CH4 : "); Serial.println(CH4);
  Serial.print("Co : "); Serial.println(CO);

  Serial.print("val : "); Serial.println(val);
  Serial.print("btn : "); Serial.println(btn);
  Serial.println("------------------------------------------------------------------------");
  delay(500);

  if (sori == false) {
    if (val == 1) {
      sori = true;
    }
  }
  else if(sori == true){
    sound();
    if(btn == 1){
      sori == false;
    }
  }
}

void sound() {
  tone(13, 1000, 1000);
  delay(5000);
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
