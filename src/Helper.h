/*
  Flutter is a lightweight wireless library for embedded systems.
  Copyright (c) 2015 Flutter Wireless. All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Affero General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with this program. If not, the license is also available here:
  http://www.gnu.org/licenses/
*/


#ifndef Helper_h
#define Helper_h

#include "Arduino.h"
#include <extEEPROM.h>    //http://github.com/JChristensen/extEEPROM/tree/dev
#include "Wire.h"

#define RSSI_OFFSET 81

namespace Helper
{
	int32_t calculateRSSI(int8_t rssiByte);
	void dumpString(uint32_t startAddr, byte array[], uint32_t nBytes);
}

#endif
