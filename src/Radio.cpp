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


#include "Radio.h"

CC1200 ccradio;

//Constructor
Radio::Radio()
{
	//do any class initialization here
	enabled = false;
	awake = false;
}

boolean Radio::init()
{
	return ccradio.init();
}

boolean Radio::sleep(boolean sleep)
{
	return ccradio.sleep(sleep);
}


boolean Radio::setFrequency(uint32_t frequency)
{
	return ccradio.SetFrequency(frequency);
}

boolean Radio::transmit(byte *data, byte start, byte length)
{
	return ccradio.transmit(data, start, length);
}

boolean Radio::transmit(byte *data, byte length)
{
	return ccradio.transmit(data, 0, length);
}

byte Radio::bytesAvailable()
{
	return ccradio.bytesAvailable();
}

boolean Radio::readRX(Queue& rxBuffer, byte bytesToRead)
{
	return ccradio.readRX(rxBuffer, bytesToRead);
}

boolean Radio::txBytes(byte _bytes)
{
	return ccradio.txBytes(_bytes);
}

void Radio::setAddress(byte address)
{
	ccradio.setAddress(address);
}

void Radio::clearRXFIFO()
{
	ccradio.clearRXFIFO();
}

int32_t Radio::getRSSI()
{
	return ccradio.getRSSI();
}
