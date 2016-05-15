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


#ifndef CC1200_H
#define CC1200_H


#include "Flutter.h"
#include "REG_CC1200.h"
#include "CC1200_Flutter_reg_config_915mhz_20ksps_GFSK_12_5khz.h"
//#include "CC1200_Flutter_reg_config_915mhz_1ksps.h"
//#include "CC1200_Flutter_reg_config_915mhz_1000ksps.h"
#include <SPI.h>
#include "Queue.h"

//radio states from chip status byte
typedef enum _CC1200_STATE_
{
    CC1200_IDLE = 0, /// < Radio in standby
    CC1200_RX, /// 1 transmit ready
    CC1200_TX,  /// 2 transmitting active
    CC1200_FSTX, /// 3 fast transmit ready
    CC1200_CALIBRATE, /// 4 = CALIBRATE (synthesizer is calibrating)
    CC1200_SETTLING,  /// 5 = Radio SETTLING
    CC1200_RXERROR,   /// 6 = RX FIFO Error
    CC1200_TXERROR, /// 7 = TX Fifo Error
}
CC1200_STATE;

#warning deprecate in favor of typedef enum
#define RADIO_IDLE (byte)0
#define RADIO_RX (byte)1
#define RADIO_TX (byte)2
#define RADIO_FSTX (byte)3
#define RADIO_CALIBRATE (byte)4
#define RADIO_SETTLING (byte)5
#define RADIO_RXERROR (byte)6
#define RADIO_TXERROR (byte)7


#define REG_TXFIFO 0x3F

#define COMMAND_QUEUE_SIZE 8 //radio commands are queued in an array of two byte arrays.

#define RADIO_SINGLE_ACCESS  0x00
#define RADIO_WRITE_ACCESS   0x00
#define RADIO_BURST_ACCESS   0x40
#define RADIO_READ_ACCESS    0x80

#define RX_FIFO_ERROR       0x11

#define PKTBUFSIZE 8  //5 packets for circular buffer
#define CCPKTLENGTH 128 //64 + 2 status + 1 length + 1 flutterState(rx,tx,rxSucceed, txSucceed, rxFailed, txFailed)




class CC1200
{

public:

	CC1200_STATE state;

	volatile byte chipStatus;
	boolean asleep;

	CC1200();

	//void setLED(byte red, byte green, byte blue);



	boolean init();
	byte getState();
	void flushRX();
	void flushTX();
	void reset();
	void enterSleep();
	void exitSleep();
	boolean transmit(byte data[], byte start, byte length);
	boolean readRX(Queue& rxBuffer, byte bytesToRead);
	byte bytesAvailable();
  int32_t getRSSI();

	boolean sleep(boolean _sleep);

	byte ccGetTxStatus(void);
	byte ccGetRxStatus(void);
	boolean SetFrequency(uint32_t frequency);
	void setAddress(byte address);
	boolean txBytes(byte _bytes);
	void setBand(byte _band);

	byte printMARC();

	void clearRXFIFO();

private:


	CC1200_STATE setState(CC1200_STATE setState);

	int8_t readRSSI();

	void WriteReg(uint16_t, byte);
	byte ReadReg(uint16_t);
	void ReadBurstReg(byte addr, byte *buffer, byte count);
	void WriteBurstReg(byte addr, byte *buffer, byte count);
	void readConfigRegs(void);


	//SPI communications
	void ccReadWriteBurstSingle(byte addr, byte *pData, uint16_t len, uint16_t dataStart);
	byte cc16BitRegAccess(byte accessType, byte extAddr, byte regAddr, byte *pData, uint16_t len);
	byte cc8BitRegAccess(byte accessType, byte addrByte, byte *pData, uint16_t len, uint16_t dataStart);
	byte SendStrobe(byte);
	byte SPItransfer(byte dataOut);

	//register access
	byte ccWriteReg(uint16_t addr, byte *pData, byte len);
	byte ccReadReg(uint16_t addr, byte *pData, byte len);
	byte ccWriteTxFifo(byte *pData, byte len);
	byte ccReadRxFifo(byte * pData, byte len, uint16_t dataStart);


	//configutation
	uint16_t registerConfig(void);
#ifdef DEBUG
	//debugging
	void printBuffer(byte *buffer, byte length);
	void printReg(uint16_t reg, String name);

 #endif /* DEBUG */
#ifdef RADIOTESTMODE
    void rxTestMode();
	void txTestMode();
#endif /*RADIOTESTMODE*/


};






#endif /* CC1200_H */
