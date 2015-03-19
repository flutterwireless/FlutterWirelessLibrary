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

#include "Flutter.h"


Network network;


void softInt();
void radioInterrupt();
void button1();
void button2();

Flutter::Flutter() {

  synchronized = false;
  initialized = false;

  setFlutterPins();
  setRadioPins();
  setInterrupts();
}

boolean Flutter::init() {

	initialized = network.init(BAND);
  return initialized;
  
}

void Flutter::connect(uint16_t devices)
{
  network.connect(devices);
}

int Flutter::getState()
{
  return network.networkStatus;
}

int Flutter::dataAvailable()
{
  return network.dataAvailable();
}

int Flutter::nextPacket()
{
  return network.nextPacket();
}


int Flutter::nextPacketLength()
{
  return network.nextPacketLength();
}


int Flutter::readBytes(byte *array,int packetSize)
{
  return network.readBytes(array,packetSize);
}

void Flutter::ledLightShow()
{
  int i=0;
  int j=0;
  int k=1;

  while(j<4)
  {
    switch(j)
    {
      case 0:
      setLED(i,0,0);
      break;
      case 1:
      setLED(0,i,0);
      break;
      case 2:
      setLED(0,0,i);
      break;
      case 3:
      setLED(i,i,i);
      break;
    }

    i+=k;

    if(i>255)
    {
      k=-1;
    }
     if(i==0 && k==-1)
    {
      j++;
      k=1;
    }
    delayMicroseconds(500);

  }
  setLED(0,0,0);
}

void Flutter::setNetworkName(String name)
{
  network.setNetworkName(name);
}

void Flutter::setAddress(int address)
{
  network.setAddress(address);
}


byte Flutter::sendData(byte data, byte address)
{
  return network.queueDataPacket(CMD_USER_ARRAY, (byte*)data, 1, address);
}

byte Flutter::sendData(byte data[], byte length, byte address)
{

  return network.queueDataPacket(CMD_USER_ARRAY, data, length, address);
}

byte Flutter::next()
{
  return 0;
}

int Flutter::packetRSSI(byte *array,int packetSize) 
{

int val = array[packetSize-2];

  int dbm = 0;

  if(val>=128)
  {
    dbm = 128-val;
  }else
  {
    dbm = val;
  }
  dbm = dbm - RSSI_OFFSET;
  return dbm;

}
/*
int Flutter::getRSSI() 
{
  byte val = radio.readRSSI();
  return(getRSSI(val));
}
*/

void Flutter::setLED(int red, int green, int blue) {
  analogWrite(LED_R, (byte)(255-red));
  #ifdef FLUTTER_R2
  analogWrite(LED_G, (byte)(255-green));
  analogWrite(LED_B, (byte)(blue));
  #else
  analogWrite(LED_G, (byte)(green));
  analogWrite(LED_B, (byte)(255-blue));
  #endif
}

void Flutter::setRadioPins()
{
  pinMode(SS, OUTPUT);
  pinMode(MOSI, OUTPUT);
  pinMode(MISO, INPUT);
  pinMode(SCK, OUTPUT);
  pinMode(GDO0_PIN, INPUT);
  
}

void Flutter::setFlutterPins()
{
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
  pinMode(BUTTON1, INPUT);
  pinMode(BUTTON2, INPUT);
}

void Flutter::setInterrupts()
{
  attachInterrupt(255, softInt,0);
  attachInterrupt(BUTTON1, button1, CHANGE);
  attachInterrupt(GDO0_PIN, radioInterrupt, CHANGE);
  attachInterrupt(BUTTON2, button2, CHANGE);
}

int Flutter::interrupt()
{
  return network.radioInterrupt();
}
boolean Flutter::processSoftInt()
{
  return network.softInt();
}
boolean Flutter::tickInt()
{
  return network.tickInterrupt();
}


