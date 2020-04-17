 /*water tap*/
#include <OTA.h>
#define pin_sensor_on 2
#define pin_sensor_off 3
#define pin_water_on 6
#define pin_water_off 5
#define pin_soap 4
#define pin_sound 7
#define pin_led 13
#define pin_reset 17
bool soap_or_wather_trigger = false;
unsigned long timer;
unsigned int timer_OTA = 0;

void setup() {
  pinMode(pin_sensor_on, INPUT_PULLUP);
  pinMode(pin_sensor_off, INPUT_PULLUP);
  digitalWrite(pin_water_on, HIGH);
  digitalWrite(pin_water_off, HIGH);
  digitalWrite(pin_soap, HIGH);
  pinMode(pin_water_on, OUTPUT);
  pinMode(pin_water_off, OUTPUT);
  pinMode(pin_soap, OUTPUT);
  pinMode(pin_sound, OUTPUT);
  pinMode(pin_led, OUTPUT);
  for(int i = 0; i < 3; i++) {
    digitalWrite(pin_led, HIGH);
    delay(100);
    digitalWrite(pin_led, LOW);
    delay(100);
  }
  Serial.begin(19200);     // the speed of this board when write a sketch, you can see it in "boards.txt"
  pinMode(pin_reset,OUTPUT);
  
}

void water_on() {
  digitalWrite(pin_water_on, LOW);
  digitalWrite(pin_led, HIGH);
  delay(100);
  digitalWrite(pin_water_on, HIGH);
  delay(100);
}

void water_off() {
  digitalWrite(pin_water_off, LOW);
  digitalWrite(pin_led, LOW);
  delay(100);
  digitalWrite(pin_water_off, HIGH);
  delay(100);
}

void sound_on() {
  for (unsigned int i = 0; i <= 15000; i = i + 4) {
    tone(pin_sound, i);
  }
  noTone(pin_sound);
}

void sound_off() {
  for (unsigned int i = 15000; i >= 4; i = i - 4) {
    tone(pin_sound, i);
  }
  noTone(pin_sound);
}

bool wait(unsigned int timeMS, byte pin_sensor) {
  timer = millis();
  while (millis() - timer <= timeMS) {
    if (digitalRead(pin_sensor) == LOW) {
      tone(pin_sound, 4000, 50);
      return false;
    }
  }
  return true;
}

void loop() {
  start:
  OTA(timer_OTA, pin_reset);
  if (digitalRead(pin_sensor_on) == LOW) {
    tone(pin_sound, 4000, 50);
    if (wait(1000, pin_sensor_off) == true) {
      water_on();
      delay(1500);
      water_off();
      delay(1000);
      digitalWrite(pin_soap, LOW);     // soap ON
      wait(2000, pin_sensor_off);
      digitalWrite(pin_soap, HIGH);     // soap OFF
      if (wait(10000, pin_sensor_on) == true) {
        sound_off();
        goto start;
      }
      water_on();
      wait(20000, pin_sensor_off);
      water_off();
    } else {
      sound_on();
      if (wait(5000, pin_sensor_on) == true) {
        sound_off();
      } else {
        water_on();
        wait(30000, pin_sensor_off);
        water_off();
      }
    }
  }
}
