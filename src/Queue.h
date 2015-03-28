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