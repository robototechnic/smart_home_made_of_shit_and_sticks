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
  digitalWrite(pin_led, HIGH);
  digitalWrite(pin_water_on, LOW);
  delay(100);
  digitalWrite(pin_water_on, HIGH);
  delay(100);
}

void water_off() {
  digitalWrite(pin_led, LOW);
  digitalWrite(pin_water_off, LOW);
  delay(100);
  digitalWrite(pin_water_off, HIGH);
  delay(100);
}

void sound_on() {
  for (unsigned int i = 0; i <= 15000; i = i + 7) {
    tone(pin_sound, i);
  }
  noTone(pin_sound);
}

void sound_off() {
  for (unsigned int i = 15000; i >= 7; i = i - 7) {
    tone(pin_sound, i);
  }
  noTone(pin_sound);
}

byte soap_cycle() {
  if (wait(2000, pin_sensor_on) == false) {     // bring hands to the tap
    digitalWrite(pin_soap, LOW);     // soap ON
    delay(500);
    wait(5000, pin_sensor_off);
    digitalWrite(pin_soap, HIGH);     // soap OFF
    delay(2000);
    return 0;
  } else {
    return 1;
  }
}

bool wait(unsigned int timeMS, byte pin_sensor) {     // standby function
  timer = millis();
  while (millis() - timer <= timeMS) {
    if (digitalRead(pin_sensor) == LOW) {     // interrupt
      return false;
    }
  }
  return true;     // standby ended
}

void loop() {
  //OTA(timer_OTA, pin_reset);
  start:
  if (digitalRead(pin_sensor_on) == LOW) {     // bring hands to the tap
    water_on();
    if (wait(3000, pin_sensor_off) == false) {     // wait 3 seconds to wet hands
      water_off();
      goto start;
    }
    sound_on();     // signal that you can turn on the soap
    if (wait(1000, pin_sensor_off) == false) {     // remove hands within 1 second  
      water_off();
      if (soap_cycle() == 1) {
        sound_off();
        goto start;
      }
      if (wait(10000, pin_sensor_on) == false) {
        water_on();
      } else {
        sound_off();
        goto start;
      }
    }
    wait(20000, pin_sensor_off);     // wait until the timer expires or the interrupt is triggered
    water_off();
  }
}
