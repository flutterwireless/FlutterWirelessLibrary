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


#ifndef Network_h
#define Network_h

#include "Radio.h"
#include "Flutter.h"
#include "Queue.h"

//radio configuration
#define EU 0 //use this setting for most countries inside the EU. Flutter will operate from 865-868 MHz on 50 pseudorandomly chosen channels
#define NORTH_AMERICA 1 //use this setting for the US and Canada. Flutter will operate from 902-928 MHz on 50 pseudorandomly chosen channels
#define US2 2 //Flutter will operate from 915-928 MHz on 50 pseudorandomly chosen channels
#define AUS_NZ 3 //use this setting for the Australia and New Zealand. Flutter will operate from 925-928 MHz on 50 pseudorandomly chosen channels

//Frequency Selection
//WARNING! This setting must be properly set for your region or you will be breaking the law.
//Falure to properly set this value can result in serious harmful operation.
//If not properly configured, the device may cause harmful interference to critical emergency services. DO NOT SET IT TO A REGION OTHER THAN YOUR OWN. It will not improve operation and will cause harm.
//***** TO SET YOUR REGION, REMOVE THE LEADING SLASHES ON ONE OF THE FOLLOWING LINES *****//
//#define BAND EU
#define BAND NORTH_AMERICA
//#define BAND US2
//#define BAND AUS_NZ


#if !defined BAND
#error " *************** ERROR: No region defined for Flutter. In the Flutter library, in the "src" folder, see line 34 of "Network.h" and set to your region to resolve the issue. This only needs to be done once. ***************"
#endif

#define FREQ_BANDS {865000000,902000000,915000000,925000000} //hz
#define NUM_CHANNELS {60,520,260,60} //hz
#define NUM_BANDS 4
#define CHANNEL_SPACE 50000 //hz//

#define ORPHAN_CH 0 //this is the physical radio channel used for orphans. It does not use the generated channel list but is a fixed channel to be used for orphans. Any devices with the same value here can be paired together.

#define TIMING_CH_INDEX 1 //channel index used for timing. This uses the channel list to determine which physical radio channel this will be.

#define DISABLED 0
#define ORPHAN_SEARCH 1
#define ORPHAN_WAIT 2
#define SYNC_WAIT 3
#define NORMAL_OPERATION 4
#define TEST_MODE 5

#define BASE_CHANNEL 0

#define BROADCAST_ADDRESS 0

#define CMD_USER_ARRAY 0x20

#define CMD_ORPHAN_REQUEST 0x40
#define CMD_SYNCTIME 0x42
#define CMD_HOST_REQUEST 0x43

#define COMMAND_QUEUE_SIZE 5
#define RX_PACKET_QUEUE_SIZE 5
#define QUEUE_TX 1
#define QUEUE_RX 2

//tasks
#define NO_TASK 0
#define TX_TASK 1
#define OTHER_TASK 2

#define BUFFER_SIZE 128

//Packet States. Attached to each packet in the packet queue so we can identify their state.
// first bit is TX/RX (1/0), next is success, next is failed
// all 3 bits high is for marking invalid or processed packets
#define RX_PACKET   ((0<<0)|(0<<1)|(0<<2)) // 0
#define TX_PACKET   ((1<<0)|(0<<1)|(0<<2)) // 1
#define RXSUCCEED   ((0<<0)|(1<<1)|(0<<2)) // 2
#define TXSUCCEED   ((1<<0)|(1<<1)|(0<<2)) // 3
#define RXFAILED    ((0<<0)|(0<<1)|(1<<2)) // 4
#define TXFAILED    ((1<<0)|(0<<1)|(1<<2)) // 5
#define INVALID     ((1<<0)|(1<<1)|(1<<2)) // 7


typedef struct
{
	int_fast16_t micros;
	int_fast16_t millis;
	int_fast32_t seconds;
} timeType;



class Network
{
public:
	Network();
	boolean init(uint8_t _band, int32_t calibration);
	volatile byte address;
	volatile byte band;
	uint16_t channelIndex;
	uint16_t channel;
	uint16_t channelList[51];
	String networkName;
	byte networkStatus;
	volatile timeType time;
	volatile timeType lastPacketTime;
	volatile int radioState;
	volatile boolean rxPending;
	boolean synchronized;
	boolean paused;
	int32_t frequency_adjustment;
	int32_t frequency_calibration;
	void enterTestMode();
	// byte txBuffer[BUFFER_SIZE];
	// byte rxBuffer[BUFFER_SIZE];

	// byte txBufferIndex;
	// byte rxBufferIndex;

	boolean asleep;
	boolean sleep(boolean sleep);
	volatile boolean txBlocked;

	void setAddress(int _address);
	void setNetworkName(String _NetworkName);
	boolean setChannel(uint32_t _channel);
	boolean setChannelByIndex(uint32_t _channel);
	int radioInterrupt();
	boolean tickInterrupt();
	boolean softInt();
	void connect(uint16_t devices);

	int queueDataPacket(byte command, byte *tx, byte length, byte destaddress);


	int readBytes(byte *array, int packetSize);

	Queue rxBuffer;
	Queue txBuffer;
	Queue userBuffer;

	int dataAvailable();
	int nextPacket();
	int nextPacketLength();

	int32_t getRSSI();

private:




	void pinDebug(int pin, int value);

	int16_t calculateFrequencyOffset(int16_t index);

	byte queuedTXCommands;
	byte commandQueueTX[COMMAND_QUEUE_SIZE][3];

	byte queuedRXPackets;
	byte packetQueueRX[RX_PACKET_QUEUE_SIZE][3];

	void printTime();

	uint16_t scheduledMicros;
	byte scheduledTask;
	void scheduleTask(uint16_t micros, byte task);

	byte pendingRxPacket;
	void printChannelList();
	void generateChannelList(String name);
	void generateChannelList(uint16_t seed);
	uint16_t generateSeed(String name);
	uint16_t networkSeed;
	void setMicros(uint32_t value) ;
	void processNormalOperation();
	boolean hop();
	void transmitOrphanSearchPacket();
	void transmitSyncPacket(byte destaddress);
	int readPacket();
	byte processRXPacket(byte packetLength);

	byte queueRXPacket(byte packetLength);

	void dequeueTXBytes(int index);
	void dequeueRXPacket(int index);

	void queueCommand(byte cmd, byte start, byte length);

	void requestHost(uint16_t devices);

	void updateLastPacketTime();
	void syncTime(byte packetLength);

	void txNext();
	void transmitPacket(int index);
};


#endif
