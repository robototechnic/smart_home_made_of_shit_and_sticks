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
bool recheck = false;
unsigned long timer;
unsigned int timer_OTA = 1000;

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
  pinMode(pin_reset,OUTPUT);
  Serial.begin(19200);     // the speed of this board when write a sketch, you can see it in "boards.txt"
    
  for(int i = 0; i < 3; i++) {
    digitalWrite(pin_led, HIGH);
    delay(100);
    digitalWrite(pin_led, LOW);
    delay(100);
  }
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
  for (unsigned int i = 0; i <= 15000; i = i + 6) {
    tone(pin_sound, i);
  }
  noTone(pin_sound);
}

void sound_off() {
  for (unsigned int i = 15000; i >= 4; i = i - 6) {
    tone(pin_sound, i);
  }
  noTone(pin_sound);
}

bool wait(unsigned int timeMS, byte pin_sensor) {     // standby function 
  timer = millis();
  while (millis() - timer <= timeMS) {
    if (digitalRead(pin_sensor) == LOW) {     // interrupt
      return false;
    }
  }
  return true;
}

void loop() {
  OTA(timer_OTA, pin_reset);
  start:
  if (digitalRead(pin_sensor_on) == LOW) {     // (1)bring hands to the tap
    tone(pin_sound, 4000, 50);
    if (wait(1000, pin_sensor_off) == true) {     // (2)hold hands near
      water_on();
      wait(30000, pin_sensor_off);     // wait until the timer expires or the interrupt is triggered
      water_off();
    } else {     // (2)take hands off the tap
      sound_on();
      if (wait(3000, pin_sensor_on) == true) {     // (3)hold hands far
        sound_off();
        goto start;
      } else {     // (3)bring hands to the tap
        water_on();
        wait(1500, pin_sensor_off);     // wait until the timer expires or the interrupt is triggered
        water_off();
        digitalWrite(pin_soap, LOW);     // soap ON
        delay(1000);
        //wait(5000, pin_sensor_off);
        digitalWrite(pin_soap, HIGH);     // soap OFF
        delay(3000);
        if (wait(10000, pin_sensor_on) == true) {     // (4)don`t bring hands to the tap
          sound_off();
          goto start;
        }
        water_on();
        wait(20000, pin_sensor_off);
        water_off();
      }
    }
  }
}
