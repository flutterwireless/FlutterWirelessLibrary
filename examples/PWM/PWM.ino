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


uint32_t pwmPin = 9;
uint32_t maxDutyCount = 128;
uint32_t clkAFreq = 0;
uint32_t pwmFreq = 10000000ul;
uint32_t channel = g_APinDescription[pwmPin].ulPWMChannel;
byte sineTable[256];

volatile unsigned long frequency = 0;
int i = 0;
int steps = 256;


void setup()
{
	pmc_enable_periph_clk(PWM_INTERFACE_ID);
	PWMC_ConfigureClocks(clkAFreq, 0, VARIANT_MCK);
	PIO_Configure(
	    g_APinDescription[pwmPin].pPort,
	    g_APinDescription[pwmPin].ulPinType,
	    g_APinDescription[pwmPin].ulPin,
	    g_APinDescription[pwmPin].ulPinConfiguration);
	PWMC_ConfigureChannel(PWM_INTERFACE, channel , pwmFreq, 0, 0);
	PWMC_SetPeriod(PWM_INTERFACE, channel, maxDutyCount);
	PWMC_EnableChannel(PWM_INTERFACE, channel);
	PWMC_SetDutyCycle(PWM_INTERFACE, channel, maxDutyCount / 2);
	pmc_mck_set_prescaler(2);

	for (int j = 0; j < 256; j++)
	{
		sineTable[j] = 64 + 63.0f * sin(2.0f * PI * (j / 255.0f));
	}
}

void loop()
{
	delay(500);
	playMario();
	delay(2000);
}

void playTone(int note, int duration, int delayTime)
{
	frequency = note;
	uint32_t startMillis = millis();

	while ((startMillis + duration * 2) > millis())
	{
		for (i = 0; i < steps; i++)
		{
			PWMC_SetDutyCycle(PWM_INTERFACE, channel, sineTable[i]);
			delayMicroseconds((1000000ul / (frequency / 2)) / (steps));
		}
	}

	delay(delayTime * 2 - duration * 2);
}

void playMario()

{
	playTone(660, 100, 150);
	playTone(660, 100, 300);
	playTone(660, 100, 300);
	playTone(510, 100, 100);
	playTone(660, 100, 300);
	playTone(770, 100, 550);
	playTone(380, 100, 575);
	playTone(510, 100, 450);
	playTone(380, 100, 400);
	playTone(320, 100, 500);
	playTone(440, 100, 300);
	playTone(480, 80, 330);
	playTone(450, 100, 150);
	playTone(430, 100, 300);
	playTone(380, 100, 200);
	playTone(660, 80, 200);
	playTone(760, 50, 150);
	playTone(860, 100, 300);
	playTone(700, 80, 150);
	playTone(760, 50, 350);
	playTone(660, 80, 300);
	playTone(520, 80, 150);
	playTone(580, 80, 150);
	playTone(480, 80, 500);
	playTone(510, 100, 450);
	playTone(380, 100, 400);
	playTone(320, 100, 500);
	playTone(440, 100, 300);
	playTone(480, 80, 330);
	playTone(450, 100, 150);
	playTone(430, 100, 300);
	playTone(380, 100, 200);
	playTone(660, 80, 200);
	playTone(760, 50, 150);
	playTone(860, 100, 300);
	playTone(700, 80, 150);
	playTone(760, 50, 350);
	playTone(660, 80, 300);
	playTone(520, 80, 150);
	playTone(580, 80, 150);
	playTone(480, 80, 500);
	playTone(500, 100, 300);
	playTone(760, 100, 100);
	playTone(720, 100, 150);
	playTone(680, 100, 150);
	playTone(620, 150, 300);
	playTone(650, 150, 300);
	playTone(380, 100, 150);
	playTone(430, 100, 150);
	playTone(500, 100, 300);
	playTone(430, 100, 150);
	playTone(500, 100, 100);
	playTone(570, 100, 220);
	playTone(500, 100, 300);
	playTone(760, 100, 100);
	playTone(720, 100, 150);
	playTone(680, 100, 150);
	playTone(620, 150, 300);
	playTone(650, 200, 300);
	playTone(1020, 80, 300);
	playTone(1020, 80, 150);
	playTone(1020, 80, 300);
	playTone(380, 100, 300);
	playTone(500, 100, 300);
	playTone(760, 100, 100);
	playTone(720, 100, 150);
	playTone(680, 100, 150);
	playTone(620, 150, 300);
	playTone(650, 150, 300);
	playTone(380, 100, 150);
	playTone(430, 100, 150);
	playTone(500, 100, 300);
	playTone(430, 100, 150);
	playTone(500, 100, 100);
	playTone(570, 100, 420);
	playTone(585, 100, 450);
	playTone(550, 100, 420);
	playTone(500, 100, 360);
	playTone(380, 100, 300);
	playTone(500, 100, 300);
	playTone(500, 100, 150);
	playTone(500, 100, 300);
	playTone(500, 100, 300);
	playTone(760, 100, 100);
	playTone(720, 100, 150);
	playTone(680, 100, 150);
	playTone(620, 150, 300);
	playTone(650, 150, 300);
	playTone(380, 100, 150);
	playTone(430, 100, 150);
	playTone(500, 100, 300);
	playTone(430, 100, 150);
	playTone(500, 100, 100);
	playTone(570, 100, 220);
	playTone(500, 100, 300);
	playTone(760, 100, 100);
	playTone(720, 100, 150);
	playTone(680, 100, 150);
	playTone(620, 150, 300);
	playTone(650, 200, 300);
	playTone(1020, 80, 300);
	playTone(1020, 80, 150);
	playTone(1020, 80, 300);
	playTone(380, 100, 300);
	playTone(500, 100, 300);
	playTone(760, 100, 100);
	playTone(720, 100, 150);
	playTone(680, 100, 150);
	playTone(620, 150, 300);
	playTone(650, 150, 300);
	playTone(380, 100, 150);
	playTone(430, 100, 150);
	playTone(500, 100, 300);
	playTone(430, 100, 150);
	playTone(500, 100, 100);
	playTone(570, 100, 420);
	playTone(585, 100, 450);
	playTone(550, 100, 420);
	playTone(500, 100, 360);
	playTone(380, 100, 300);
	playTone(500, 100, 300);
	playTone(500, 100, 150);
	playTone(500, 100, 300);
	playTone(500, 60, 150);
	playTone(500, 80, 300);
	playTone(500, 60, 350);
	playTone(500, 80, 150);
	playTone(580, 80, 350);
	playTone(660, 80, 150);
	playTone(500, 80, 300);
	playTone(430, 80, 150);
	playTone(380, 80, 600);
	playTone(500, 60, 150);
	playTone(500, 80, 300);
	playTone(500, 60, 350);
	playTone(500, 80, 150);
	playTone(580, 80, 150);
	playTone(660, 80, 550);
	playTone(870, 80, 325);
	playTone(760, 80, 600);
	playTone(500, 60, 150);
	playTone(500, 80, 300);
	playTone(500, 60, 350);
	playTone(500, 80, 150);
	playTone(580, 80, 350);
	playTone(660, 80, 150);
	playTone(500, 80, 300);
	playTone(430, 80, 150);
	playTone(380, 80, 600);
	playTone(660, 100, 150);
	playTone(660, 100, 300);
	playTone(660, 100, 300);
	playTone(510, 100, 100);
	playTone(660, 100, 300);
	playTone(770, 100, 550);
	playTone(380, 100, 575);
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



