/*
  Flutter is a lightweight wireless library for embedded systems.
  Copyright (c) 2015 Flutter Wireless. All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  The license is also available here:
  https://www.gnu.org/licenses/lgpl-2.1.html
*/

#include "Radio.h"

CC1200 ccradio;

//Constructor
Radio::Radio(){
  //do any class initialization here
	enabled = false;
	awake = false;
}

boolean Radio::init()
{
	return ccradio.init();
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
	return ccradio.readRX(rxBuffer,bytesToRead);
}

void Radio::setAddress(byte address)
{
  ccradio.setAddress(address);
}

void Radio::clearRXFIFO()
{
	ccradio.clearRXFIFO();
}