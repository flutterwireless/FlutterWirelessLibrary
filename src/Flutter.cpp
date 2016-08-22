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

#include "Flutter.h"

Network network;


void softInt();
void radioInterrupt();
void button1();
void button2();

Flutter::Flutter()
{
	synchronized = false;
	initialized = false;
	setFlutterPins();
	setRadioPins();
	setInterrupts();
}

boolean Flutter::init()
{
	int32_t cal = readCalibration();
	initialized = network.init(band, (int32_t)cal);
	return initialized;
}

void Flutter::connect(uint16_t devices)
{
	network.connect(devices);
}

int Flutter::readCalibration()
{
	byte toRead[1];
	Helper::dumpString(32/8*1024-80, toRead, 1);
	int32_t cal = 0;

	if(toRead[0]=='F') //lazy way of checking if calibration was written.
	{
		int length = 7;
		byte toRead2[length];
		Helper::dumpString(32/8*1024-80+30, toRead2, length);

		cal = 0-atoi((char*)toRead2);
		//SerialUSB.print("Cal value is: ");
		//SerialUSB.println(cal);
	}
	return cal;
}

int Flutter::getState()
{
	return network.networkStatus;
}

int Flutter::dataAvailable()
{
	return network.dataAvailable();
}

boolean Flutter::sleep(boolean sleep)
{
	return network.sleep(sleep);
}


int Flutter::nextPacket()
{
	return network.nextPacket();
}


int Flutter::nextPacketLength()
{
	return network.nextPacketLength();
}


int Flutter::readBytes(byte *array, int packetSize)
{
	return network.readBytes(array, packetSize);
}

void Flutter::ledLightShow()
{
	int i = 0;
	int j = 0;
	int k = 1;

	while (j < 4)
	{
		switch (j)
		{
			case 0:
				setLED(i, 0, 0);
				break;

			case 1:
				setLED(0, i, 0);
				break;

			case 2:
				setLED(0, 0, i);
				break;

			case 3:
				setLED(i, i, i);
				break;
		}

		i += k;

		if (i > 255)
		{
			k = -1;
		}

		if (i == 0 && k == -1)
		{
			j++;
			k = 1;
		}

		delayMicroseconds(500);
	}

	setLED(0, 0, 0);
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

int Flutter::packetRSSI(byte *array, int packetSize)
{
	return Helper::calculateRSSI(array[packetSize - 2]);
}

int32_t Flutter::getRSSI()
{
	return network.getRSSI();
}

void Flutter::setLED(int red, int green, int blue)
{
	analogWrite(LED_R, (byte)(255 - red));
#ifdef FLUTTER_R2
	analogWrite(LED_G, (byte)(255 - green));
	analogWrite(LED_B, (byte)(blue));
#else
	analogWrite(LED_G, (byte)(green));
	analogWrite(LED_B, (byte)(255 - blue));
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
	attachInterrupt(255, softInt, 0);
	attachInterrupt(BUTTON1, button1, CHANGE);
	attachInterrupt(GDO0_PIN, radioInterrupt, CHANGE);
	attachInterrupt(BUTTON2, button2, CHANGE);
}

void Flutter::pause()
{
	network.paused = true;
}

void Flutter::resume()
{
	network.paused = false;
}

void Flutter::setFrequencyAdjustment(int32_t adjustment)
{
	network.frequency_adjustment = adjustment;
}

int32_t Flutter::getFrequencyAdjustment()
{
	return network.frequency_adjustment;
}

void Flutter::enterTestMode()
{
	network.enterTestMode();
}

void Flutter::setChannel(int channel)
{
	network.setChannel(channel);
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
