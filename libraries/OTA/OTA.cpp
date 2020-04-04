/**************************************************************************/
/*!
@file     OTA.cpp
@author   robototechnic
@license  GNU LGPLv3

Library for autoreset arduino and write sketch OTA.
@section  HISTORY

v1.0 - First release
*/
/**************************************************************************/

#include "OTA.h"

void OTA(unsigned int timer_OTA, byte pin_reset) {
  if (timer_OTA == 0 || millis() <= timer_OTA) {
    if (Serial.available()) {
      if(Serial.peek()=='0') {
        for(int i=0; i<100; i++) {
          if(Serial.read() == '0' && Serial.read() == ' ') {
            digitalWrite(pin_reset,HIGH);
          }
          delay(10);
        }
      }
    }
  }
}
