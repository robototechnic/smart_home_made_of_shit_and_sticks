/**************************************************************************/
/*!
@file     GetCheckSum.h
@author   robototechnic
@license  GNU LGPLv3

Library for checksum calculation. The value is stored in one byte.

@section  HISTORY

v1.0 - First release
*/
/**************************************************************************/

#ifndef GetCheckSum_h
#define GetCheckSum_h
#include "Arduino.h"

byte GetCheckSum(byte* data, int length);

#endif
