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


byte myData = 0;
boolean _running = false;
Flutter flutter;


void setup()
{
	Serial.begin(115200);
	flutter.setNetworkName("Home network");
	//pinMode(6,OUTPUT);
	//pinMode(7,OUTPUT);
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
	flutter.setAddress(2);
	flutter.connect(1); //form a network with this and one other device
}



void loop()
{
	while (flutter.getState() != NORMAL_OPERATION) //if we aren't synchronized with another radio, just loop and blink lights.
	{
		flutter.setLED(RED);
		delay(200);
		flutter.setLED(BLUE);
		delay(200);
	}

	//send a byte over the radio
	flutter.sendData(myData, 0); //0 is broadcast address
	myData++;

	//read the radio to see if there is data available
	while (flutter.dataAvailable() > 0)
	{
		byte data = flutter.next();

		if (data % 2 == 1) //if our data is an odd number
		{
			flutter.setLED(0, 255, 0); //light up the green LED
		}
		else
		{
			flutter.setLED(0, 0, 0);
		}
	}

	delay(200); //spend some time smelling the roses
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



