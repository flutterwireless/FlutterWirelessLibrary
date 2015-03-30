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

#include "Queue.h"

Queue::Queue()
{
	end = 0;
}

Queue::~Queue()
{
	delete []array;
}


boolean Queue::write(byte data)
{
	if (bytesAvailable() < 1)
	{
		return false;
	}

	array[end] = data;
	end++;
	return true;
}

void Queue::clear()
{
	end = 0;
}


int Queue::read()
{
	if (end == 0)
	{
		return -1;
	}

	byte next = array[0];

	for (int i = 0; i < end; i++)
	{
		array[i] = array[i + 1]; //yes this is a dumb but quick way of dealing with the array. Feel free to improve it...
	}

	end--;
	return next;
}

byte Queue::bytesAvailable()
{
	return QUEUESIZE - end;
}

byte Queue::bytesEnd()
{
	return end;
}


byte Queue::capacity()
{
	return QUEUESIZE;
}
