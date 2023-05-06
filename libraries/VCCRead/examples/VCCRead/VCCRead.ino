#include <VCCRead.h>

void setup() {
  Serial.begin(9600);
}

void loop() {
  Serial.println(VCCRead());
  delay(1000);
}
