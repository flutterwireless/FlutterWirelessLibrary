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


#include "Network.h"


Radio radio;
volatile bool hopNow = false;
volatile bool updateSysTick = false;
static const uint32_t bands[] = FREQ_BANDS;
static const uint32_t numChannels[] = NUM_CHANNELS;
volatile int syncError = 0;

volatile bool queueAck = false;

volatile int pin = 6;

#define HOPTIME 40 //ms per hop
#define TXTIME1 1 //ms into hop
#define TXTIME2 20 //ms into hop
#define TIMING_ADJUST 6900-3320 //us to add for timing adjustment
//#define TIMING_ADJUST -4000 //us to add for timing adjustment

//Constructor
Network::Network()
{
	//do any class initialization here
	networkName = "";
	networkStatus = DISABLED;
	networkSeed = 0;
	address = 0;
	pendingRxPacket = 0;
	band = 0;
	channel = 0;
	channelIndex = 0;
	scheduledMicros = 0;
	scheduledTask = NO_TASK;
	txBlocked = false;
	queuedTXCommands = 0;
	radioState = IDLE;
	time.micros = 0;
	time.millis = 0;
	time.seconds = 0;
	paused = false;
	frequency_adjustment = 0;
	frequency_calibration = 0;
	awaiting_ack = false;
	received_ack = false;
}

//configuration functions

void Network::setNetworkName(String _networkName)
{
	networkName = _networkName;
}

void Network::setAddress(int _address)
{
	address = _address;
	radio.setAddress(address);
}

boolean Network::sleep(boolean sleep)
{
	return radio.sleep(sleep);
}


void Network::enterTestMode()
{
	networkStatus = TEST_MODE;
}

//initialization

boolean Network::init(uint8_t _band, int32_t calibration, Logging _logger) //intialize radio.
{
	band = _band;

	logger = _logger;

	frequency_calibration = calibration;

	if (!radio.enabled)
	{
		radio.enabled = radio.init(logger);
	}

	radioState = RXIDLE;
#ifdef PIN_DEBUG
	pinMode(6, OUTPUT);
	pinMode(7, OUTPUT);
	pinMode(8, OUTPUT);
	pinMode(9, OUTPUT);
	pinDebug(6, LOW);
	pinDebug(7, LOW);
	pinDebug(8, LOW);
	pinDebug(9, LOW);
#endif
	return radio.enabled;
}

void Network::connect(uint16_t devices)
{
	if (networkName == "")
	{
		setChannel(ORPHAN_CH);
		networkStatus = ORPHAN_SEARCH;
	}
	else
	{
		generateChannelList(networkName);
		setChannelByIndex(TIMING_CH_INDEX);

		if (address == MASTER_ADDRESS)
		{
			networkStatus = NORMAL_OPERATION;
		}
		else
		{
			networkStatus = SYNC_WAIT;
		}

		//printChannelList();
	}

	//requestHost(devices);
}

void Network::requestHost(uint16_t devices)
{
	byte txData[6] = {5, BROADCAST_ADDRESS, address, CMD_HOST_REQUEST, (byte)(devices >> 8), (byte)(devices && 0xFF)};
	radio.transmit(txData, 6);
	radioState = TXPENDING;
}


//internal work functions

void Network::generateChannelList(String name)
{
	networkSeed = generateSeed(name);
	generateChannelList(networkSeed);
}

void Network::generateChannelList(uint16_t seed)
{
	int i = 0;
	int temp = 0;
	randomSeed(seed); //initialize Arduino pseudorandom number generator with our seed.

	while (i < 51)
	{
		temp = random(numChannels[band]);
		int j = 0;

		//this loop makes sure the new channel value is not already in use.
		while (j < i)
		{
			if (channelList[j] == temp || temp == ORPHAN_CH)
			{
				temp = random(numChannels[band]);
				j = 0;
			}
			else
			{
				j++;
			}
		}

		channelList[i] = temp;
		i++;
	}
}

uint16_t Network::generateSeed(String name)
{
	uint16_t i, seed = 0;
	seed = name[0];
	i++;

	while (name[i] != '\0')
	{
		seed = (seed >> 8) | ((uint16_t)(seed << 8)); //swap low and high byte, just for kicks.
		seed = seed ^ name[i];
		i++;
		//generate a seed. Maybe XOR some bytes?
	}

	return seed;
}

void Network::printChannelList()
{
	while (!SerialUSB)
	{
		//SerialUSB.begin(115200);
	}

	//SerialUSB.print("Network Name: ");
	//SerialUSB.print(networkName);
	//SerialUSB.print(", Seed: ");
	//SerialUSB.println(networkSeed);

	for (int i = 0; i < 50; i++)
	{
		////SerialUSB.print("Ch Index ");
		//SerialUSB.print(i);
		//SerialUSB.print(", ");
		//SerialUSB.println(channelList[i]);
	}
}



// Radio interface functions

boolean Network::setChannel(uint32_t _channel)
{
	if (_channel >= numChannels[band])
	{
		return false;
	}

	channel = _channel;
	return radio.setFrequency(bands[band] + (CHANNEL_SPACE / 2) + (int32_t)_channel * CHANNEL_SPACE + frequency_adjustment + frequency_calibration);

}

boolean Network::setChannelByIndex(uint32_t _channel_index)
{
	channel = channelList[_channel_index];
	channelIndex = _channel_index;
	return setChannel(channel);
}

void Network::printTime()
{
	////SerialUSB.print("Time: ");
	////SerialUSB.print(time.seconds);
	////SerialUSB.print("s, ");
	////SerialUSB.print(time.millis);
	////SerialUSB.println("ms.");
}


boolean Network::softInt() //software interrupt
{
	interrupts();


	////SerialUSB.println("SoftInt");

	if (time.millis == 0)
	{
		// printTime();
	}

	//if process was only fired to run a scheduled task, the following condition will be true
	if (scheduledMicros == 0 && scheduledTask != NO_TASK)
	{
		//run our task.
		if (scheduledTask == TX_TASK)
		{
			if (txBlocked == false)
			{
				scheduledTask = NO_TASK;
				txNext();
			}
		}
	}

	if(queueAck == true)
	{
		queueAck = false;
		if ((channelIndex == TIMING_CH_INDEX && time.millis % HOPTIME == TXTIME1) || networkStatus != NORMAL_OPERATION)
		{
			radio.queueAck(255); //disables ack
			radioState = RXIDLE;
			////SerialUSB.println("NOAck");
		} else
		{
			radio.queueAck(address); //enables ack
			////SerialUSB.println("QueueAck");
		}
	}


	if (rxPending == true)
	{
		rxPending = false;
#ifdef DEBUG
		////SerialUSB.println("Read RX");
#endif
		int packetLength = readPacket();

		if (packetLength > 0)
		{
//#ifdef DEBUG
			////SerialUSB.print("Read ");
			////SerialUSB.print(packetLength);
			////SerialUSB.print(" bytes from radio.");
//#endif
			processRXPacket(packetLength);
			////SerialUSB.println("ReadRX");
		}
		else if (packetLength < 0)
		{
			queuedRXPackets = 0;
			rxBuffer.clear();
		}
	}

	if(received_ack == true)
	{
		Log.notice(F("Got ACK\n"));
		awaiting_ack = false;
		received_ack = false;
	}

	switch (networkStatus)
	{
		case DISABLED:

		////SerialUSB.println("Network Disabled");
			return true;
			break;

		case ORPHAN_SEARCH:
			if (time.millis == 0 || time.millis == 500) //every 500mS, we send out an orphan search packet
			{
				transmitOrphanSearchPacket();
			}

			break;

		case ORPHAN_WAIT:
			break;

		case SYNC_WAIT:
			processNormalOperation();
			////SerialUSB.println("Network SYNC_WAIT");
			break;

		case NORMAL_OPERATION:
			processNormalOperation();
			////SerialUSB.println("Network NORMAL_OPERATION");
			break;

		case TEST_MODE:
			processNormalOperation();
			break;
	}

	return true;
}

void Network::txNext()
{
	if (queuedTXCommands > 0)
	{
		if (commandQueueTX[0][0] == QUEUE_TX)
		{
			awaiting_ack = true;
			received_ack = false;
			transmitPacket(0);
			Log.notice(F("Transmit Queued TX packet\n"));
		}
	}
}

void Network::transmitPacket(int index)
{
	radio.queueAck(255); //disables Ack on RX so we do not Ack the Ack.
	radio.transmit(txBuffer.array, commandQueueTX[index][1], commandQueueTX[index][2]);
	radioState = TXPENDING;
	dequeueTXBytes(index);
}

void Network::dequeueTXBytes(int index)
{
	if (index == 0)
	{
		int length = commandQueueTX[index][2];
		int i = 0;

		for (i = 0; i < commandQueueTX[index][2]; i++)
		{
			txBuffer.read();
		}

		for (i = 0; i < queuedTXCommands - 1; i++)
		{
			commandQueueTX[i][1] = commandQueueTX[i + 1][1] - length;
			commandQueueTX[i][2] = commandQueueTX[i + 1][2];
		}

		queuedTXCommands--;
	}
}


void Network::dequeueRXPacket(int index)
{
	if (index == 0 && queuedRXPackets != 0)
	{
		int length = packetQueueRX[index][2];
		int i = 0;

		for (i = 0; i < packetQueueRX[index][2]; i++)
		{
			userBuffer.read();
		}

		for (i = 0; i < queuedRXPackets - 1; i++)
		{
			packetQueueRX[i][1] = packetQueueRX[i + 1][1] - length;
			packetQueueRX[i][2] = packetQueueRX[i + 1][2];
		}

		queuedRXPackets--;
	}
}

void Network::processNormalOperation()
{
// //SerialUSB.println("blip");
// SerialUSB.print("networkStatus: ");
// SerialUSB.println(networkStatus);
// SerialUSB.print("radioState: ");
// SerialUSB.println(radioState);

	if (hopNow == true && networkStatus!=TEST_MODE)
	{
		hopNow = false;
		//SerialUSB.println("Hop");
		hop();
		//Log.notice(F("Hopped\n"));
	}

	if(address == MASTER_ADDRESS && networkStatus==TEST_MODE)
	{
			radio.txBytes(0);
			networkStatus=DISABLED;
			return;
	}

	if (time.millis % HOPTIME == TXTIME1 || time.millis % HOPTIME == TXTIME2)
	{
#ifdef TXTESTMODE

		switch (txTestMode)
		{
			case 1:
				radio.txBytes(0);
				break;

			case 2:
				radio.txBytes(TESTBYTES);
				break;
		}

#endif

		if (channelIndex == TIMING_CH_INDEX && address == MASTER_ADDRESS && time.millis % HOPTIME == TXTIME1)
		{
			//Log.notice(F("Transmit Sync\n"));
			transmitSyncPacket(0x00);
			//Log.notice(F("Transmit Sync Success\n"));
		}

		if (channelIndex != TIMING_CH_INDEX || time.millis % HOPTIME == TXTIME2) // && time.micros%HOPTIME <=45000)
		{
			// ////SerialUSB.println("Processing Queue");
			// processQueue();
			//Log.notice(F("Transmit Next\n"));
			awaiting_ack = false;
			txNext();
		}

	}

#ifdef DEBUG1

	if (time.millis % 1000 == 0)
	{
		printChipStatus();
		radio.printMARC();
	}

#endif
}

int Network::readPacket()
{
	byte count = radio.bytesAvailable();
#ifdef DEBUG
	////SerialUSB.println("");
	////SerialUSB.println("----------------");
	////SerialUSB.println("");
#endif

//  ////SerialUSB.print("Count: ");
//  ////SerialUSB.print(count);

	if (count > 0)
	{
		if (count == 0xFF) //The radio only has a 128 byte FIFO, so 255 represents a FIFO error (typically due to a bit flip in the length byte of the packet)
		{
			radio.clearRXFIFO();
			return -1;
		}

		if (radio.readRX(rxBuffer, count))
		{
			return count;
		}
		else
		{
			return -1;
		}
	}

	return 0;
}

byte Network::processRXPacket(byte packetLength)
{
	int commandByteIndex = rxBuffer.bytesEnd() - packetLength + 3;
	int command = rxBuffer.array[commandByteIndex];
//#ifdef DEBUG
	////SerialUSB.print("Command Byte. 0x");
	////SerialUSB.println(command, HEX);
//#endif

	switch (command)
	{
		case CMD_SYNCTIME:
			syncTime(packetLength);
			////SerialUSB.println("SyncTime");
			break;

		case CMD_USER_ARRAY:
			queueRXPacket(packetLength);
			////SerialUSB.println("CMD_USER_ARRAY");
			break;
	}

	return 0;
}

byte Network::queueRXPacket(byte packetLength)
{
	int i = rxBuffer.bytesEnd() - packetLength; //start at the beginning of the packet

	while (i < rxBuffer.bytesEnd()) //read to the end of the packet
	{
		userBuffer.write(rxBuffer.array[i]); //copy values into the user buffer
		i++;
	}

	rxBuffer.end -= packetLength; // shift rxBuffer queue end pointer to before our now-copied packet
	packetQueueRX[queuedRXPackets][0] = RX_PACKET;
	packetQueueRX[queuedRXPackets][1] = userBuffer.bytesEnd() - packetLength;
	packetQueueRX[queuedRXPackets][2] = packetLength;
#ifdef DEBUG
	////SerialUSB.print("Received Packet, Index #");
	////SerialUSB.print(queuedRXPackets);
	////SerialUSB.print(", RX Buffer bytes: ");
	////SerialUSB.println(rxBuffer.bytesEnd());
#endif
	queuedRXPackets++;

	if (userBuffer.bytesEnd() > 100 || queuedRXPackets == RX_PACKET_QUEUE_SIZE)
	{
		queuedRXPackets = 0;
		userBuffer.clear();
	}

	return 0;
}


void Network::syncTime(byte packetLength)
{
// byte txData[6] = { (byte)((micros()%1000)/4),(byte)(((time.millis%1000)>>2) & 0xFF) ,(byte)((time.millis%1000<<6) & 0xC0) | (time.seconds>>24 & 0x3F) ,(byte)(time.seconds>>16 & 0xFF) ,(byte)(time.seconds>>8 & 0xFF) ,(byte)(time.seconds & 0xFF)};
	timeType temp;
	networkStatus = NORMAL_OPERATION;
	syncError=0;
	int16_t sentMicros = ((int16_t)rxBuffer.array[rxBuffer.bytesEnd() - packetLength + 5]) * 4;
	int32_t sentMillis = ((int32_t)rxBuffer.array[rxBuffer.bytesEnd() - packetLength + 6]) << 2 | ((int32_t)(rxBuffer.array[rxBuffer.bytesEnd() - packetLength + 7]) >> 6);
	int32_t sentSeconds = ((int32_t)(rxBuffer.array[rxBuffer.bytesEnd() - packetLength + 7]) & 0x3F) << 24 | ((int32_t)rxBuffer.array[rxBuffer.bytesEnd() - packetLength + 8]) << 16 | ((int32_t)rxBuffer.array[rxBuffer.bytesEnd() - packetLength + 9]) << 8 | rxBuffer.array[rxBuffer.bytesEnd() - packetLength + 10];
	int32_t microsNow = micros() % 1000;
	int32_t millisNow = time.millis;
	int32_t microsElapsed = micros() % 1000 - lastPacketTime.micros;
	int32_t millisElapsed = time.millis - lastPacketTime.millis;
	int32_t secondsElapsed = time.seconds - lastPacketTime.seconds;
	rxBuffer.end -= packetLength; // shift rxBuffer queue end pointer to before our now-processed packet

	//pinDebug(9,LOW);

	if (microsElapsed < 0)
	{
		microsElapsed += 1000;
		millisElapsed -= 1;
	}

	if (millisElapsed < 0)
	{
		millisElapsed += 1000;
		secondsElapsed -= 1;
	}

// time now = time sent + time elapsed
// uint16_t error = micros()%1000 - (tempMicros+microsSinceReception);
	int32_t newMicros = sentMicros + microsElapsed + TIMING_ADJUST; //7900;//+44400;
	int32_t newMillis = sentMillis + millisElapsed;
	int32_t newSeconds = sentSeconds + secondsElapsed;

	while (newMicros >= 1000)
	{
		newMicros -= 1000;
		newMillis++;
	}

	while (newMicros < 0)
	{
		newMicros += 1000;
		newMillis--;
	}

	while (newMillis >= 1000)
	{
		newMillis -= 1000;
		newSeconds++;
	}

	while (newMillis < 0)
	{
		newMillis += 1000;
		newSeconds--;
	}

	setMicros(1000 - newMicros);
	time.micros = 1000 - newMicros;
	time.millis = newMillis;
	time.seconds = newSeconds;
//#ifdef DEBUG_TIME
	////SerialUSB.print("Local time when recieved: ");
	////SerialUSB.print((lastPacketTime.micros));
	////SerialUSB.print(" uS, ");
	////SerialUSB.print((lastPacketTime.millis));
	////SerialUSB.print(" mS, ");
	////SerialUSB.print((lastPacketTime.seconds));
	////SerialUSB.println(" S.");
	////SerialUSB.print("Time recieved over the air: ");
	////SerialUSB.print((sentMicros));
	////SerialUSB.print(" uS, ");
	////SerialUSB.print((sentMillis));
	////SerialUSB.print(" mS, ");
	////SerialUSB.print((sentSeconds));
	////SerialUSB.println(" S.");
	////SerialUSB.print("Time elapsed: ");
	////SerialUSB.print((microsElapsed));
	////SerialUSB.print(" uS, ");
	////SerialUSB.print((millisElapsed));
	////SerialUSB.print(" mS, ");
	////SerialUSB.print((secondsElapsed));
	////SerialUSB.println(" S.");
	////SerialUSB.print("Time now: ");
	////SerialUSB.print((microsNow));
	////SerialUSB.print(" uS, ");
	////SerialUSB.print((millisNow));
	////SerialUSB.println(" mS, ");
	////SerialUSB.print("Setting to: ");
	////SerialUSB.print((newMicros));
	////SerialUSB.print(" uS, ");
	////SerialUSB.print((newMillis));
	////SerialUSB.println(" mS, ");
	////SerialUSB.println("");
//#endif
// ////SerialUSB.print("Time Error: ");
// ////SerialUSB.println((error)/1000);
	//////SerialUSB.print("Read Packet Lag: ");
	//////SerialUSB.println((microsSinceReception)/1000);
	//////SerialUSB.print("Packet Index: ");
	//////SerialUSB.println(index);
//  ////SerialUSB.println();
	//////SerialUSB.print("RX Error: ");
// ////SerialUSB.println((temp.micros-microsRecieved/1000)/1000.0f);
//channel = 0;
#ifdef DEBUG
	////SerialUSB.print("Time: ");
	////SerialUSB.print(time.seconds + (time.millis) / 1000.0f);
	////SerialUSB.println(" seconds.");
#endif
// ////SerialUSB.print(time.micros/1000);
// ////SerialUSB.println("ms.");
}



//user data functions
int Network::dataAvailable()
{
	return userBuffer.bytesEnd();
}

int Network::nextPacketLength()
{
	return packetQueueRX[0][2];
}

int Network::nextPacket()
{
	dequeueRXPacket(0);
	return packetQueueRX[0][2];
}

int Network::readBytes(byte *array, int packetSize)
{
	int i;

	for (i = 0; i < packetSize; i++)
	{
		array[i] = userBuffer.array[i];
	}

	return packetSize;
}


//system work functions

boolean Network::hop()
{
	//channel = (time.micros*200)/1000000;
	if (channelIndex == TIMING_CH_INDEX && networkStatus != NORMAL_OPERATION && address != MASTER_ADDRESS)
	{
		//SerialUSB.print("Don't Hop. Channel is: ");
		//SerialUSB.println(channelIndex);
		return false; //don't hop if sync not recieved
	}

	pinDebug(7, HIGH);
	channelIndex++;

	if (channelIndex >= 50)
	{
		channelIndex = 0;
	}

	if (address != MASTER_ADDRESS && channelIndex == TIMING_CH_INDEX)
	{
		syncError++; //in normal operation, this will get reset to zero shortly after this line, when a timing packet arrives. We will check this next hop to see if timing did arrive.
	}

	if(address != MASTER_ADDRESS && channelIndex == TIMING_CH_INDEX+1)
	{
		if(syncError>=2)
		{
			networkStatus = SYNC_WAIT;
			radioState = RXIDLE;
		  channelIndex = TIMING_CH_INDEX;
		}
	}

//#ifdef DEBUG1
	//SerialUSB.print("Setting radio to channel ");
	//SerialUSB.println(channelIndex);
//#endif

	if (paused == true && channelIndex != TIMING_CH_INDEX)
	{
		return false;
	}

	//Log.notice(F("Set channel\n"));
	setChannelByIndex(channelIndex);
	radioState = RXIDLE;
	pinDebug(7, LOW);
	return true;
}


boolean Network::tickInterrupt()
{
	interrupts();
	pinDebug(6, HIGH);
	//deal with task scheduling
	/* if(scheduledTask!=NO_TASK)
	 {
	   if(scheduledMicros<=1000) //less than 1000 means we are in the middle of a milisecond and it is time to fire. exactly 1000 means we hit a new millisecond and we must tick down to zero and fire.
	   {
	     if(scheduledMicros!=1000) setMicros(1000-scheduledMicros); //we are in the middle of a millisecond tick. Tell the system to finish that tick.
	     scheduledMicros=0;
	     softInterrupt();
	     pinDebug(6,LOW);
	     return false; //tell system not to update its own millisecond timer for this tick
	   }

	   scheduledMicros-=1000;

	   if(scheduledMicros<1000)
	   {
	     setMicros(scheduledMicros); //set the timer to fire at the scheduled time
	     updateSysTick = false;
	   }

	 }*/
	time.millis += 1;

	if (time.millis >= 1000) // increment millis and reset if it rolls over
	{
		time.millis = 0;
		time.seconds++;
	}

	if (updateSysTick == true)
	{
		setMicros(1000); //reset system tick to 1ms if it was changed to adjust timing
		updateSysTick = false;
	}

	int tickTime = time.millis % HOPTIME;

	switch (networkStatus)
	{
		case DISABLED:
			break;

		case ORPHAN_SEARCH:
			if (time.millis == 0 || time.millis == 500)
			{
				softInterrupt();
			}

			break;

		case ORPHAN_WAIT:
			break;

		case TEST_MODE:
		case SYNC_WAIT:
		case NORMAL_OPERATION:
			switch (tickTime)
			{
				case 0:
					hopNow = true;

					if (queuedTXCommands > 0 && channelIndex != 49)
					{
						//   scheduledTask=TX_TASK;
						//  scheduledMicros=3000;
					}

				case TXTIME1:
					softInterrupt();
				case TXTIME2:
					softInterrupt();
					break;
			}
			break;

	}

	if(received_ack == true)
	{
		softInterrupt();
	}

	pinDebug(6, LOW);
	return true; //system will update its own millisecond timer if this is true.
}

void Network::pinDebug(int pin, int value)
{
#ifdef PIN_DEBUG

	if (value == HIGH)
	{
		g_APinDescription[pin].pPort->PIO_SODR = g_APinDescription[pin].ulPin;
	}
	else
	{
		g_APinDescription[pin].pPort->PIO_CODR = g_APinDescription[pin].ulPin;
	}

#endif
}

int Network::radioInterrupt()
{
	interrupts();

	if (digitalRead(GDO0_PIN) == HIGH)
	{
		// lastPacketTime.micros=time.micros;
		// lastPacketTime.seconds=time.seconds;
		//////SerialUSB.print(" HIGH ");
		switch (radioState)
		{
			case RXIDLE:
				radioState = RXACTIVE;
				updateLastPacketTime();
				pinDebug(9, HIGH);
				queueAck = true;
				break;

			case ACK_TX:
				radioState = ACK_ACTIVE;
				pinDebug(9, HIGH);
				break;

			case ACK_WAIT:
				radioState = ACK_ACTIVE;
				break;

			case TXPENDING:
				radioState = TXACTIVE;
				//if(_printTXDelay==true)
				// {
				//   _printTXDelay = false;
				//   printTXDelay();
				// }
				break;

			default:
				//this is some kind of error
				// errors++;
				radioState = RXIDLE;
				break;
		}
	}
	else
	{
		//////SerialUSB.print(" LOW ");
		switch (radioState)
		{
			case RXACTIVE:
				radioState = ACK_TX;
				rxPending = true;
				pinDebug(9, LOW);
				softInterrupt();
				break;

			case TXACTIVE:
				radioState = ACK_WAIT;
				pinDebug(9, LOW);
				break;

			case ACK_ACTIVE:
				radioState = RXIDLE;
				received_ack = true;
				pinDebug(9, LOW);
				break;

			default:
				//this is some kind of error
				// errors++;
				radioState = RXIDLE;
				break;
		}
	}

// ////SerialUSB.println(radioState);
	return 0;
}

void Network::setLED(int red, int green, int blue)
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

void Network::updateLastPacketTime() //sends packet with just data (no commands added)
{
	lastPacketTime.micros = micros() % 1000;
	lastPacketTime.millis = time.millis;
	lastPacketTime.seconds = time.seconds;
}


void Network::transmitOrphanSearchPacket() //sends packet with just data (no commands added)
{
	byte txData[4] = {3, BROADCAST_ADDRESS, address, CMD_ORPHAN_REQUEST};
	radio.transmit(txData, 4);
	radioState = TXPENDING;
}

void Network::transmitSyncPacket(byte destaddress) //sends packet with just data (no commands added)
{
	uint32_t _micros = micros() % 1000;
	uint32_t _millis = time.millis;
	uint32_t _seconds = time.seconds;
	Log.notice(F("Transmit Sync start\n"));
	byte txData[11] = {10, BROADCAST_ADDRESS, address, CMD_SYNCTIME, 0, (byte)((_micros) / 4), (byte)(((_millis) >> 2) & 0xFF) , (byte)((_millis << 6) & 0xC0) | (_seconds >> 24 & 0x3F) , (byte)(_seconds >> 16 & 0xFF) , (byte)(_seconds >> 8 & 0xFF) , (byte)(_seconds & 0xFF)};
	radio.transmit(txData, 11);
	radioState = TXPENDING;
	Log.notice(F("Transmit Sync Success\n"));
}

int Network::queueDataPacket(byte command, byte *tx, byte length, byte destaddress) //sends packet with just data (no commands added)
{
	length += 5; //We add 5 bytes to the packet
	queueCommand(TX_PACKET, txBuffer.bytesEnd(), length);
	txBuffer.write(length - 1); //packet length byte not included in length calculation
	txBuffer.write(destaddress);
	txBuffer.write(address);
	txBuffer.write(command);
	txBuffer.write(0); //an extra byte, currently not in use.
	byte i;

	for (i = 5; i < length; i++)
	{
		txBuffer.write(tx[i - 5]);
	}

	return length;
}

void Network::queueCommand(byte cmd, byte start, byte length)
{
	if (cmd == QUEUE_TX)
	{
		if (queuedTXCommands + 1 < COMMAND_QUEUE_SIZE)
		{
			commandQueueTX[queuedTXCommands][0] = cmd;
			commandQueueTX[queuedTXCommands][1] = start;
			commandQueueTX[queuedTXCommands][2] = length;
			queuedTXCommands++;
#ifdef DEBUG
			////SerialUSB.print("Queuing Command ");
			////SerialUSB.print(queuedTXCommands, DEC);
			////SerialUSB.print(" = ");
			////SerialUSB.print(cmd, DEC);
			////SerialUSB.print(" ");
			////SerialUSB.print(start, DEC);
			////SerialUSB.print(" ");
			////SerialUSB.println(length, DEC);
#endif
		}
		else
		{
			SerialUSB.print("ERR: Cannot Queue Command, queue full.");
		}
	}
}

int32_t Network::getRSSI()
{
	return radio.getRSSI();
}


void Network::setMicros(uint32_t value)
{
	if (SysTick_Config((SystemCoreClock / 1000) * (value / 1000.0f)))
	{
		// Capture error
		while (true);
	}

	if (value != 1000)
	{
		updateSysTick = true;
	}

	return;
}
