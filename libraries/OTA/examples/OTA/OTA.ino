#include <OTA.h>
#define pin_led 13
#define pin_reset 17
unsigned long timer;
unsigned int timer_OTA = 0;     // how long after start-up can you flash the board
bool ledState = LOW; 

void setup() {
  Serial.begin(19200);     // the speed of this board when write a sketch, you can see it in "boards.txt"
  pinMode(pin_reset,OUTPUT);
  pinMode(pin_led, OUTPUT);
  timer = millis();
}

void loop() {
  OTA(timer_OTA, pin_reset);

  // your code
  if (millis() - timer >= 1000) {
    timer = millis();
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }
  }
  digitalWrite(pin_led, ledState);
}
