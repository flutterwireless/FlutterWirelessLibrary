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


#include "Helper.h"

namespace Helper
{

  int32_t calculateRSSI(int8_t rssiByte)
  {
    int dbm = rssiByte;
    if (dbm == -128)
    {
      return -200;
    }
    dbm = dbm - RSSI_OFFSET;
    return dbm;
  }

  void dumpString(uint32_t startAddr, byte array[], uint32_t nBytes)
  {

    extEEPROM eep(kbits_32, 1, 32, 0x50);

      uint32_t nRows = (nBytes + 15) >> 4;

      uint8_t d[16];
      for (uint32_t r = 0; r < nRows; r++) {
          uint32_t a = startAddr + 16 * r;
          eep.read(a, d, 16);

          for ( int c = 0; c < 16; c++ ) {

            uint16_t index = c+16*r;
            if(index<nBytes)
            {
              array[index]=d[c];
            }
          }


      }
  }

}
