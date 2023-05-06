/**************************************************************************/
/*!
@file     GetCheckSum.cpp
@author   robototechnic
@license  GNU LGPLv3

Library for checksum calculation. The value is stored in one byte.

@section  HISTORY

v1.0 - First release
*/
/**************************************************************************/

#include "Arduino.h"
#include "GetCheckSum.h"
  
byte GetCheckSum(byte* data, int length) {
  byte checksum = 0;
  int i = 0;
  while (length--) {
    checksum += *(data + i);
    i++;
  }
  return checksum;
}
