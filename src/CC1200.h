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


#ifndef CC1200_h
#define CC1200_h


#include "Flutter.h"
#include "REG_CC1200.h"
#include "CC1200_Flutter_reg_config_915mhz_20ksps_GFSK_12_5khz.h"
//#include "CC1200_Flutter_reg_config_915mhz_1ksps.h"

//#include <libraries/SPI/SPI.h>
#include <SPI.h>
#include "Queue.h"

//radio states from chip status byte
#define RADIO_IDLE (byte)0
#define RADIO_RX (byte)1
#define RADIO_TX (byte)2
#define RADIO_FSTX (byte)3
#define RADIO_CALIBRATE (byte)4
#define RADIO_SETTLING (byte)5
#define RADIO_RXERROR (byte)6
#define RADIO_TXERROR (byte)7
// 0 = IDLE, 1 = RX, 2 = TX, 3 = FastTX ready
// 4 = CALIBRATE (synthesizer is calibrating), 5 = SETTLING, 6 = RX FIFO Error, 7 = TX Fifo Error

#define REG_TXFIFO 0x3F

#define COMMAND_QUEUE_SIZE 8 //radio commands are queued in an array of two byte arrays.

#define RADIO_BURST_ACCESS   0x40
#define RADIO_SINGLE_ACCESS  0x00
#define RADIO_READ_ACCESS    0x80
#define RADIO_WRITE_ACCESS   0x00

#define RX_FIFO_ERROR       0x11

#define PKTBUFSIZE 8 //5 packets for circular buffer
#define CCPKTLENGTH 128 //64 + 2 status + 1 length + 1 flutterState(rx,tx,rxSucceed, txSucceed, rxFailed, txFailed)




class CC1200
{
  
  public:
  	byte state;
    volatile byte flutterRadioState;
    volatile byte chipStatus;
    volatile boolean byteCapture;
    volatile byte power;
    volatile uint32_t channel;
    volatile int interruptCount;
    boolean asleep;
    void setLED(byte red, byte green, byte blue);
    void rxTestMode();
    void txTestMode();
    
  
	CC1200();
	boolean init();
	byte getState();
	void flushRX();
	void flushTX();
	void reset();
	byte setState(byte setState);
  void enterSleep();
  void exitSleep();
  boolean transmit(byte data[], byte start, byte length);
  boolean readRX(Queue& rxBuffer, byte bytesToRead);
  byte bytesAvailable();
  byte readRSSI();

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
  
  
  
  void WriteReg(uint16_t, byte);
  byte ReadReg(uint16_t);
  void ReadBurstReg(byte addr, byte *buffer, byte count);
  void WriteBurstReg(byte addr, byte *buffer, byte count);
  void readConfigRegs(void);
  
  
  //debugging
  void printBuffer(byte *buffer, byte length);
  void printReg(uint16_t reg, String name);
  

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
  
    
};






#endif
