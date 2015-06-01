/*
This example code for Flutter is
Copyright 2015, Taylor Alexander and Flutter Wireless, Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "Flutter.h"

boolean _running = false;
Flutter flutter;

byte mydata = 0;

// To test this program, flash one board as a transmitter and then comment out this line (with //) to flash another board as a receiver
#define TRANSMITTER


void setup()
{
	Serial.begin(115200);
	flutter.band = BAND;
	flutter.setNetworkName("Range Test");
	Serial.println("Initializing...");

	if (flutter.init() == true)
	{
		Serial.println("Init success.");
		flutter.ledLightShow();
		delay(500);
		//analogWrite(LED_R, 128);
	}
	else
	{
		flutter.setLED(RED);
		Serial.println("Init failed.");

		while (true);
	}

	//flutter.setAddress(1);
#ifdef TRANSMITTER
	flutter.setAddress(1);
#else
	flutter.setAddress(2);
#endif
	flutter.connect(1); //form a network with this and one other device
}



void loop()
{

		if(flutter.getState()!=NORMAL_OPERATION) //if we aren't synchronized with another radio, just loop and blink lights.
		{
			if(millis()%400<200)
			{
				flutter.setLED(RED);
			}
			
			else
			{
				flutter.setLED(BLUE);
			}
			
		}

#ifdef TRANSMITTER
	mydata++;
	byte array[3] = {0x63, mydata, 0};
	//send a byte over the radio
	flutter.sendData(array, 3, 2); //legth is 3, 2 is car's address

	switch ((mydata - 1) % 3)
	{
		case 0:
			flutter.setLED(RED);
			break;

		case 1:
			flutter.setLED(BLUE);
			break;

		case 2:
			flutter.setLED(GREEN);
			break;

		default:
			break;
	}

	delay(40); //spend some time smelling the roses
#else

	if (flutter.dataAvailable() > 0)
	{
		int packetSize = flutter.nextPacketLength();
		byte array[packetSize];
		flutter.readBytes(array, packetSize);

		if (array[5] == 0x63)
		{
			Serial.print("Packet Size: ");
			Serial.println(packetSize);

			for (int i = 0; i < packetSize; i++)
			{
				Serial.print("[0x");
				Serial.print(array[i], HEX);
				Serial.print("]");
			}

			Serial.println();
			mydata = array[6];
			int rssi = flutter.packetRSSI(array, packetSize);
			Serial.print("RSSI: ");
			Serial.print(rssi);
			Serial.println(" dBm");

			for (int j = 0; j > rssi; j--)
			{
				Serial.print("=");
			}

			Serial.println("");

			switch (mydata % 3)
			{
				case 0:
					flutter.setLED(RED);
					break;

				case 1:
					flutter.setLED(BLUE);
					break;

				case 2:
					flutter.setLED(GREEN);
					break;

				default:
					break;
			}
		}

		flutter.nextPacket();
	}

#endif
}


void button1()
{
	interrupts();
	int val = digitalRead(BUTTON1); //top button

	if (val == HIGH)
	{
		// _button1=255;
	}
	else
	{
		//  _button1=0;
	}

// buttonsChanged=true;
}

void button2()
{
	interrupts();
	int val = digitalRead(BUTTON2);
#ifdef FLUTTER_R2

	if (val == HIGH)
#else
	if (val == LOW)
#endif
	{
		//_button2=255;
	}
	else
	{
		//_button2=0;
	}

// buttonsChanged=true;
}

void systemReset()
{
	flutter.setLED(0, 0, 255);
	delayMicroseconds(16000);
	delayMicroseconds(16000);
	flutter.setLED(0, 0, 0);
	delayMicroseconds(16000);
	delayMicroseconds(16000);
	flutter.setLED(0, 255, 0);
	delayMicroseconds(16000);
	delayMicroseconds(16000);
	flutter.setLED(0, 0, 0);
	delayMicroseconds(16000);
	delayMicroseconds(16000);
	flutter.setLED(255, 0, 0);
	delayMicroseconds(16000);
	delayMicroseconds(16000);
	flutter.setLED(0, 0, 0);
	initiateReset(1);
	tickReset();
}



void radioInterrupt()
{
	flutter.interrupt();
}
void softInt()
{
	flutter.processSoftInt();
}

extern boolean tickInterrupt()
{
	if (!flutter.initialized)
	{
		return true;
	}

	return flutter.tickInt();
}



