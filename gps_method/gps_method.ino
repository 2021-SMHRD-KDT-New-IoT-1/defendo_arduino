#include <SoftwareSerial.h>
SoftwareSerial gps(3, 4);

char c = "";
String str = "";
String targetStr = "GPGGA";

void setup() {
  Serial.begin(9600);
  gps.begin(9600);
}

void loop() {
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
      }
      str = "";
    } else {
      str += c;
    }
  }
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
