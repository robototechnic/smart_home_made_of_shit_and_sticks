/**************************************************************************/
/*!
@file     OTA.h
@author   robototechnic
@license  GNU LGPLv3

Library for autoreset arduino and write sketch OTA.

@section  HISTORY

v1.0 - First release
*/
/**************************************************************************/

#ifndef OTA_h
#define OTA_h
#include "Arduino.h"

void OTA(unsigned int timer_OTA, byte pin_reset);

#endif
