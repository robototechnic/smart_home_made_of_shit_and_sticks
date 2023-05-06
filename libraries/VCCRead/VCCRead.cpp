/**************************************************************************/
/*!
@file     VCCRead.cpp
@author   robototechnic
@license  GNU LGPLv3

Library for reading the supply voltage of the board. Can be used to control battery charge

@section  HISTORY

v1.0 - First release
*/
/**************************************************************************/

#include "Arduino.h"
#include "VCCRead.h"
  
int VCCRead() {
  // read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(200);     // wait for Vref to settle
  ADCSRA |= _BV(ADSC);     // start conversion
  while (bit_is_set(ADCSRA, ADSC));    // measuring
  byte low  = ADCL;     // must read ADCL first - it then locks ADCH
  byte high = ADCH;     // unlocks both
  long result = (high << 8) | low;
  result = 1125300L / result;     // calculate Vcc (in mV); 1125300 = 1.1 * 1023 * 1000
  return result;     // Vcc in millivolts
}
