#include <GetCheckSum.h>

struct MyData {
  byte val_x;
  int val_y;
  float val_z;
  byte checksum;
} data;

void setup() {
  Serial.begin(9600);
  data.val_x = 10;
  data.val_y = 11;
  data.val_z = 12.9;
  data.checksum = 0;
  byte tempchecksum = GetCheckSum((byte*)&data, sizeof(data));
  data.checksum = tempchecksum;
  Serial.println(tempchecksum);
}

void loop() {
  
}
