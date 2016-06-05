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


#ifndef Radio_h
#define Radio_h

#include "CC1200.h"
#include "Flutter.h"
#include "Queue.h"




class Radio
{
public:
	Radio();
	boolean init();

	boolean enabled;
	boolean awake;
	uint32_t baseFrequency;
	boolean setFrequency(uint32_t frequency);
	void setAddress(byte address);
	boolean sleep(boolean sleep);

	byte bytesAvailable();
	boolean readRX(Queue& rxBuffer, byte bytesToRead);
	boolean txBytes(byte _bytes);

	boolean enable();
	boolean transmit(byte *data, byte start, byte length);
	boolean transmit(byte *data, byte length);
	void clearRXFIFO();
	int32_t getRSSI();

private:



};


#endif
