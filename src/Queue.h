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

#ifndef Queue_h
#define Queue_h

#include "Arduino.h"

# define QUEUESIZE 128

class Queue
{

public:
	byte array[QUEUESIZE];
	byte end;
	Queue();
	~Queue();
	void insert(byte data);
	int read();
	boolean write(byte);
	void clear();
	byte bytesAvailable();
	byte bytesEnd();
	byte capacity();
};

#endif