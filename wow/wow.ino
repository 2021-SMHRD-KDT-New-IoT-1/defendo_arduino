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
