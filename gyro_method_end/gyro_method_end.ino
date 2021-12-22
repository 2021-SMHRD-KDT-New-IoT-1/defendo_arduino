#include<Wire.h>

const int MPU_addr = 0x68; // MPU-6050의 주소

float AcX, AcY, AcZ;

void setup() {
  Wire.begin();       //I2C 통신 시작
  Wire.beginTransmission(MPU_addr); // 데이터 전송 시작
  Wire.write(0x6b);     // register PWR_MGMT_1에 6b
  Wire.write(0);        // 0을 전송하면 MPU-6050에 활성화 됌
  Wire.endTransmission(true); // 데이터 전송 끝
  Serial.begin(9600);
}

void loop() {
  get6050();
  Serial.print(AcX);
  Serial.print(" | ");

  Serial.print(AcY);
  Serial.print(" | ");

  Serial.print(AcZ);
  Serial.print(" | ");

  Serial.println();

  delay(1000);
}

void get6050() {
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr, 14, true);

  AcX = (Wire.read() << 8 | Wire.read())/100;
  AcY = (Wire.read() << 8 | Wire.read())/100;
  AcZ = (Wire.read() << 8 | Wire.read())/100;
}
