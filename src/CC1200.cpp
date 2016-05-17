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

/**********  System Includes ************************************************/
#include <math.h>

/**********  Project Includes ***********************************************/
#include "CC1200.h"


/******* Private global variables *******************************************/

/******* Private Defines and data types *************************************/

#define FREQ_ADJ 0

/****** Public Class Functions **********************************************/
CC1200::CC1200()
{
	//do any class initialization here
	chipStatus = 0;
	asleep = false;
}

// tx random data, or just tx the preamble if _bytes = 0
boolean CC1200::txBytes(byte _bytes)
{
	if (asleep == true)
	{
		return false;
	}

	if (_bytes > 0)
	{
		byte data[_bytes];
		data[0] = _bytes - 1;

		for (int i = 1; i < _bytes; i++)
		{
			data[i] = ((byte)rand());
		}

		transmit(data, 0, _bytes);
	}
	else
	{
		setState(CC1200_TX); //just TX the preamble forever
	}
}

boolean CC1200::sleep(boolean _sleep)
{
	asleep = _sleep;

	if (asleep)
	{
		setState(CC1200_IDLE);
	}
	else
	{
		setState(CC1200_RX);
	}

	return true;
}

boolean CC1200::init()
{
	SPI.begin();//Mode 0, user controls CS pin
	SPI.setClockDivider(20, 12); //5.33 MHz - pushing it closer to 10MHz caused problems, but I didn't check the timing so it may be possible to operate faster than this
	uint16_t errors = 0;

	do
	{
		errors = registerConfig();

		if (errors)
		{
			delay(5);
			reset();
		}
	}
	while (errors != 0);

	//printMARC();
	//readConfigRegs();
	//SetFrequency(915);
	setState(CC1200_RX);
	//rxTestMode();
	//txTestMode();
	return true;
}

void CC1200::setAddress(byte address)
{
	WriteReg(REG_DEV_ADDR, address);
}

//sets the frequency for the radio oscillator.
//int frequency = desired center frequency in Hz
//returns true if success, or false if failure. Should not fail.
boolean CC1200::SetFrequency(uint32_t frequency)
{
	//digitalWrite(8,HIGH);
	setState(CC1200_IDLE);
	SendStrobe(SFRX);
	int freq = 0.0065536f * frequency; //magic number comes from simplification of calculations from the datasheet with 40MHz crystal and LO Divider of 4.
	byte f0 = 0xFF & freq;
	byte f1 = (freq & 0xFF00) >> 8;
	byte f2 = (freq & 0xFF0000) >> 16;
//#define DEBUG_FREQUENCY
#ifdef DEBUG_FREQUENCY
	SerialUSB.print("Programming Frequency ");
	SerialUSB.print(frequency);
	SerialUSB.print(" MHz. Registers 0, 1, and 2 are 0x");
	SerialUSB.print(f0, HEX);
	SerialUSB.print(" 0x");
	SerialUSB.print(f1, HEX);
	SerialUSB.print(" 0x");
	SerialUSB.println(f2, HEX);
#endif
	WriteReg(REG_FREQ0, f0);
	WriteReg(REG_FREQ1, f1);
	WriteReg(REG_FREQ2, f2);
	setState(CC1200_RX);

	if (ReadReg(REG_FREQ0) == f0 && ReadReg(REG_FREQ1) == f1 && ReadReg(REG_FREQ2) == f2)
	{
		//digitalWrite(8,LOW);
		return true;
	}
	else
	{
		//digitalWrite(8,LOW);
		return false;
	}
}


uint16_t CC1200::registerConfig(void)
{
	byte writeByte;
	byte verifyVal;
	uint16_t errors = 0;
	// Reset radio
	reset();
	byte registerCount = (sizeof(settings) / sizeof(registerSettings));

	// Write registers to radio
	for (uint16_t i = 0; i < registerCount; i++)
	{
		writeByte = settings[i].data;
		ccWriteReg(settings[i].addr, &writeByte, 1);
		ccReadReg(settings[i].addr, &verifyVal, 1);

		if (verifyVal != writeByte)
		{
			errors++;

			if (Serial)
			{
				Serial.print("Error writing to address 0x");
				Serial.print(settings[i].addr, HEX);
				Serial.print(". Value set as 0x");
				Serial.print(writeByte, HEX);
				Serial.print(" but read as 0x");
				Serial.println(verifyVal, HEX);
			}
		}
	}

	if (Serial)
	{
		Serial.print("Wrote ");
		Serial.print(registerCount - errors);
		Serial.print(" of ");
		Serial.print(registerCount);
		Serial.println(" register values.");
	}

	return errors;
}




#ifdef DEBUG1

byte CC1200::printMARC()
{
	byte marcState = ReadReg(REG_MARCSTATE);

	switch (marcState >> 5)
	{
		case 0:
			Serial.println("Marc Settling");
			break;

		case 1:
			Serial.println("Marc TX");
			break;

		case 2:
			Serial.println("Marc IDLE");
			break;

		case 3:
			Serial.println("Marc RX");
			break;
	}

	Serial.print("Marc State: 0x");
	Serial.println(marcState & 0x1F, HEX);
	return marcState;
}


void CC1200::printRadioState(byte state)
{
	switch (state)
	{
		case CC1200_IDLE:
			Serial.print("CC1200_IDLE");
			break;

		case CC1200_RXWAIT:
			Serial.print("CC1200_RXWAIT");
			break;

		case CC1200_RXACTIVE:
			Serial.print("CC1200_RXACTIVE");
			break;

		case CC1200_RXREAD:
			Serial.print("CC1200_RXREAD");
			break;

		case CC1200_TXSTART:
			Serial.print("CC1200_TXSTART");
			break;

		case CC1200_TXACTIVE:
			Serial.print("CC1200_TXACTIVE");
			break;

		default:
			break;
	}
}
#endif


int8_t CC1200::readRSSI()
{
	return ReadReg(REG_RSSI1);
}

int32_t CC1200::getRSSI()
{
	return Helper::calculateRSSI(readRSSI());
}



//*** Basic Radio Functions ***//
//*****************************//
//*****************************//

void CC1200::reset()
{
	SendStrobe(SRES);
	delay(5);
	SendStrobe(SRES); //is twice necessary? Probably not. Does it cause problems? You tell me...
}


boolean CC1200::transmit(byte *txBuffer, byte start, byte length)
{
	if (asleep == true)
	{
		return false;
	}

	//check FIFO and radio state here
	byte txBytesUsed = 0;
	// Read number of bytes in TX FIFO
	ccReadReg(REG_NUM_TXBYTES, &txBytesUsed, 1);

	if (txBytesUsed > 0)
	{
		// Make sure that the radio is in IDLE state before flushing the FIFO
		setState(CC1200_IDLE);
		SendStrobe(SFTX); //flush buffer if needed
	}

	setState(CC1200_FSTX); //spin up frequency synthesizer
	byte data[length];

	for (int i = 0; i < length; i++)
	{
		data[i] = txBuffer[i + start]; //TODO: alter fifo access functions to allow passing txBuffer directly to read or write from a specific chunk of it
	}

	ccWriteTxFifo(data, length); //TODO: do error checking for underflow, CCA, etc.
#ifdef DEBUG1
	Serial.print("Writing ");
	Serial.print(length);
	Serial.println(" bytes: ");
	printBuffer(txBuffer, length);
#endif
	setState(CC1200_TX); //spin up frequency synthesizer
	return true;
}

byte CC1200::getState()
{
	byte state = (SendStrobe(0x00) >> 4) & 0x7;
	return (state);
}


CC1200_STATE CC1200::setState(CC1200_STATE setState)
{
	byte currentState = getState(); //0 = IDLE, 1 = RX, 2 = TX, 3 = FastTX ready
	//4 = CALIBRATE (synthesizer is calibrating), 5 = SETTLING, 6 = RX FIFO Error, 7 = TX Fifo Error

	while (currentState != setState)
	{
#ifdef DEBUG_STATES
		Serial.print("setState = ");
		Serial.print(setState);
		Serial.print(" currentState = ");
		Serial.println(currentState);
#endif

		switch (currentState)
		{
			case CC1200_IDLE:
				switch (setState)
				{
					case CC1200_RX:
						SendStrobe(SRX);
						break;

					case CC1200_TX:
						SendStrobe(STX);
						break;

					case CC1200_FSTX:
						SendStrobe(SFSTXON);
						break;
				}

				currentState = getState();
				break;

			case CC1200_RX:
				switch (setState)
				{
					case CC1200_IDLE:
						SendStrobe(SIDLE);
						break;

					case CC1200_TX:
						SendStrobe(STX);
						break;

					case CC1200_FSTX:
						SendStrobe(SFSTXON);
						break;
				}

				currentState = getState();
				break;

			case CC1200_TX:
				switch (setState)
				{
					case CC1200_IDLE:
						SendStrobe(SIDLE);
						break;

					case CC1200_RX:
						SendStrobe(SRX);
						break;

					case CC1200_FSTX:
						SendStrobe(SFSTXON);
						break;
				}

				currentState = getState();
				break;

			case CC1200_FSTX:
				switch (setState)
				{
					case CC1200_IDLE:
						SendStrobe(SIDLE);
						break;

					case CC1200_TX:
						SendStrobe(STX);
						break;

					case CC1200_RX:
						SendStrobe(SRX);
						break;
				}

				currentState = getState();
				break;

			case CC1200_CALIBRATE:
				delayMicroseconds(50);
				currentState = getState();
				break;

			case CC1200_SETTLING:
				delayMicroseconds(50);
				currentState = getState();
				break;

			case CC1200_RXERROR:
				SendStrobe(SFRX);
				currentState = getState();
				break;

			case CC1200_TXERROR:
				SendStrobe(SFTX);
				currentState = getState();
				break;
		}
	}

#ifdef DEBUG_STATES
	Serial.print("State is now ");
	Serial.println(currentState);
#endif
	return (CC1200_STATE)currentState;
}


void CC1200::WriteReg(uint16_t addr, byte value)
{
#ifdef DEBUG1
	byte origValue;
	origValue = ReadReg(addr); //capture old value for debugging
#endif
	ccWriteReg(addr, &value, 1);
	byte result = ReadReg(addr);
#ifdef DEBUG1
	Serial.print("Address 0x");
	Serial.print(addr, HEX);
	Serial.print(" was set to 0x");
	Serial.print(origValue, HEX);
	Serial.print(". Now set to: 0x");
	Serial.print(result, HEX);

	if (result != value)
	{
		Serial.print(" Read value does not match set value of 0x");
		Serial.print(value, HEX);
	}

	Serial.println();
#endif
}

byte CC1200::ReadReg(uint16_t addr)
{
	byte data;
	ccReadReg(addr, &data, 1);
	return data;
}


// NEW FUNCTIONS
/*========================================================*/


/*******************************************************************************
 * @fn          trx8BitRegAccess
 *
 * @brief       This function performs a read or write from/to a 8bit register
 *              address space. The function handles burst and single read/write
 *              as specfied in addrByte. Function assumes that chip is ready.
 *
 * input parameters
 *
 * @param       accessType - Specifies if this is a read or write and if it's
 *                           a single or burst access. Bitmask made up of
 *                           RADIO_BURST_ACCESS/RADIO_SINGLE_ACCESS/
 *                           RADIO_WRITE_ACCESS/RADIO_READ_ACCESS.
 * @param       addrByte - address byte of register.
 * @param       pData    - data array
 * @param       len      - Length of array to be read(TX)/written(RX)
 *
 * output parameters
 *
 * @return      chip status
 */
byte CC1200::cc8BitRegAccess(byte accessType, byte addrByte, byte *pData, uint16_t len, uint16_t dataStart)
{
	byte readValue;
	/* Pull CS_N low and wait for SO to go low before communication starts */
	digitalWrite(SS, LOW);

	while (digitalRead(MISO) == 1);

	/* send register address byte */
	/* Storing chip status */
	readValue = SPItransfer(accessType | addrByte);
// Serial.print("[0x")
// Serial.print(accessType|addrByte,HEX);
// Serial.print(",[0x")
	ccReadWriteBurstSingle(accessType | addrByte, pData, len, dataStart);
	digitalWrite(SS, HIGH);
	/* return the status byte value */
	return (readValue);
}

byte CC1200::SPItransfer(byte dataOut)
{
	byte returnval = SPI.transfer(SS, dataOut);
#ifdef DEBUGSPI
	Serial.print("<<< 0x");
	Serial.print(returnval, HEX);
	Serial.print(" | >>> 0x");
	Serial.println(dataOut, HEX);
#endif
	return returnval;
}


/******************************************************************************
 * @fn          trx16BitRegAccess
 *
 * @brief       This function performs a read or write in the extended adress
 *              space of CC112X.
 *
 * input parameters
 *
 * @param       accessType - Specifies if this is a read or write and if it's
 *                           a single or burst access. Bitmask made up of
 *                           RADIO_BURST_ACCESS/RADIO_SINGLE_ACCESS/
 *                           RADIO_WRITE_ACCESS/RADIO_READ_ACCESS.
 * @param       extAddr - Extended register space address = 0x2F.
 * @param       regAddr - Register address in the extended address space.
 * @param       *pData  - Pointer to data array for communication
 * @param       len     - Length of bytes to be read/written from/to radio
 *
 * output parameters
 *
 * @return      rfStatus_t
 */
byte CC1200::cc16BitRegAccess(byte accessType, byte extAddr, byte regAddr, byte *pData, uint16_t len)
{
	byte readValue;
	/* Pull CS_N low and wait for SO to go low before communication starts */
	digitalWrite(SS, LOW);

	while (digitalRead(MISO) == 1);

	/* send extended address byte with access type bits set */
	/* Storing chip status */
	readValue = SPItransfer(accessType | extAddr);
	SPItransfer(regAddr);
	/* Communicate len number of bytes */
	ccReadWriteBurstSingle(accessType | regAddr, pData, len, 0);
	digitalWrite(SS, HIGH);
	/* return the status byte value */
	return (readValue);
}

/*******************************************************************************
 * @fn          ccSpiCmdStrobe
 *
 * @brief       Send command strobe to the radio. Returns status byte read
 *              during transfer of command strobe. Validation of provided
 *              is not done. Function assumes chip is ready.
 *
 * input parameters
 *
 * @param       cmd - command strobe
 *
 * output parameters
 *
 * @return      status byte
 */
byte CC1200::SendStrobe(byte cmd)
{
	byte rc;
	/* Pull CS_N low and wait for SO to go low before communication starts */
	digitalWrite(SS, LOW);

	while (digitalRead(MISO) == 1);

	rc = SPItransfer(cmd);
	digitalWrite(SS, HIGH);
	return (rc);
}

/*******************************************************************************
 * @fn          trxReadWriteBurstSingle
 *
 * @brief       When the address byte is sent to the SPI slave, the next byte
 *              communicated is the data to be written or read. The address
 *              byte that holds information about read/write -and single/
 *              burst-access is provided to this function.
 *
 *              Depending on these two bits this function will write len bytes to
 *              the radio in burst mode or read len bytes from the radio in burst
 *              mode if the burst bit is set. If the burst bit is not set, only
 *              one data byte is communicated.
 *
 *              NOTE: This function is used in the following way:
 *
 *              TRXEM_SPI_BEGIN();
 *              while(TRXEM_PORT_IN & TRXEM_SPI_MISO_PIN);
 *              ...[Depending on type of register access]
 *              trxReadWriteBurstSingle(uint8 addr,uint8 *pData,uint16 len);
 *              TRXEM_SPI_END();
 *
 * input parameters
 *
 * @param       none
 *
 * output parameters
 *
 * @return      void
 */
void CC1200::ccReadWriteBurstSingle(byte addr, byte *pData, uint16_t len, uint16_t dataStart)
{
	uint16_t i;
	pData += dataStart; //increment pdata pointer to the next available data slot in our queue array

	// Serial.print("Length: ");
	// Serial.println(len);

	/* Communicate len number of bytes: if RX - the procedure sends 0x00 to push bytes from slave*/
	if (addr & RADIO_READ_ACCESS)
	{
		if (addr & RADIO_BURST_ACCESS)
		{
			for (i = 0; i < len; i++)
			{
				byte data = SPItransfer(0x00);     /* Store pData from last pData RX */
				*pData = data;
				pData++;
			}
		}
		else
		{
			*pData = SPItransfer(0x00);
		}
	}
	else
	{
		if (addr & RADIO_BURST_ACCESS)
		{
			/* Communicate len number of bytes: if TX - the procedure doesn't overwrite pData */
			for (i = 0; i < len; i++)
			{
				SPItransfer(*pData);
				pData++;
			}
		}
		else
		{
			SPItransfer(*pData);
		}
	}

	return;
}

/*=================*/


/******************************************************************************
 * @fn          cc120xSpiReadReg
 *
 * @brief       Read value(s) from config/status/extended radio register(s).
 *              If len  = 1: Reads a single register
 *              if len != 1: Reads len register values in burst mode
 *
 * input parameters
 *
 * @param       addr   - address of first register to read
 * @param       *pData - pointer to data array where read bytes are saved
 * @param       len   - number of bytes to read
 *
 * output parameters
 *
 * @return      rfStatus_t
 */
byte CC1200::ccReadReg(uint16_t addr, byte *pData, byte len)
{
	byte tempExt  = (byte)(addr >> 8);
	byte tempAddr = (byte)(addr & 0x00FF);
	byte rc;

	/* Checking if this is a FIFO access -> returns chip not ready  */
	if ((SINGLE_TXFIFO <= tempAddr) && (tempExt == 0))
	{
		return STATUS_CHIP_RDYn_BM;
	}

	/* Decide what register space is accessed */
	if (tempExt == 0)
	{
		rc = cc8BitRegAccess((RADIO_BURST_ACCESS | RADIO_READ_ACCESS), tempAddr, pData, len, 0); //burst access is okay if we raise chip select after transfer (though I'm not sure why TI used burst here).
	}
	else if (tempExt == 0x2F)
	{
		rc = cc16BitRegAccess((RADIO_BURST_ACCESS | RADIO_READ_ACCESS), tempExt, tempAddr, pData, len);
	}

	chipStatus = rc;
	return (rc);
}

/******************************************************************************
 * @fn          cc120xSpiWriteReg
 *
 * @brief       Write value(s) to config/status/extended radio register(s).
 *              If len  = 1: Writes a single register
 *              if len  > 1: Writes len register values in burst mode
 *
 * input parameters
 *
 * @param       addr   - address of first register to write
 * @param       *pData - pointer to data array that holds bytes to be written
 * @param       len    - number of bytes to write
 *
 * output parameters
 *
 * @return      rfStatus_t
 */
byte CC1200::ccWriteReg(uint16_t addr, byte *pData, byte len)
{
	byte tempExt  = (byte)(addr >> 8);
	byte tempAddr = (byte)(addr & 0x00FF);
	byte rc;

	/* Checking if this is a FIFO access - returns chip not ready */
	if ((SINGLE_TXFIFO <= tempAddr) && (tempExt == 0))
	{
		return STATUS_CHIP_RDYn_BM;
	}

	/* Decide what register space is accessed */
	if (tempExt == 0)
	{
		rc = cc8BitRegAccess((RADIO_BURST_ACCESS | RADIO_WRITE_ACCESS), tempAddr, pData, len, 0);
	}
	else if (tempExt == 0x2F)
	{
		rc = cc16BitRegAccess((RADIO_BURST_ACCESS | RADIO_WRITE_ACCESS), tempExt, tempAddr, pData, len);
	}

	chipStatus = rc;
	return (rc);
}

/*******************************************************************************
 * @fn          cc120xSpiWriteTxFifo
 *
 * @brief       Write pData to radio transmit FIFO.
 *
 * input parameters
 *
 * @param       *pData - pointer to data array that is written to TX FIFO
 * @param       len    - Length of data array to be written
 *
 * output parameters
 *
 * @return      rfStatus_t
 */
byte CC1200::ccWriteTxFifo(byte *pData, byte len)
{
	byte rc;
	rc = cc8BitRegAccess(0x00, BURST_TXFIFO, pData, len, 0);
	chipStatus = rc;
	return (rc);
}

/*******************************************************************************
 * @fn          cc120xSpiReadRxFifo
 *
 * @brief       Reads RX FIFO values to pData array
 *
 * input parameters
 *
 * @param       *pData - pointer to data array where RX FIFO bytes are saved
 * @param       len    - number of bytes to read from the RX FIFO
 *
 * output parameters
 *
 * @return      rfStatus_t
 */
byte CC1200::ccReadRxFifo(byte * pData, byte len, uint16_t dataStart)
{
	byte rc;
// Serial.print("Length1: ");
// Serial.println(len);
	rc = cc8BitRegAccess(0x00, BURST_RXFIFO, pData, len, dataStart);
	chipStatus = rc;
	return (rc);
}

/******************************************************************************
 * @fn      cc120xGetTxStatus(void)
 *
 * @brief   This function transmits a No Operation Strobe (SNOP) to get the
 *          status of the radio and the number of free bytes in the TX FIFO.
 *
 *          Status byte:
 *
 *          ---------------------------------------------------------------------------
 *          |          |            |                                                 |
 *          | CHIP_RDY | STATE[2:0] | FIFO_BYTES_AVAILABLE (free bytes in the TX FIFO |
 *          |          |            |                                                 |
 *          ---------------------------------------------------------------------------
 *
 *
 * input parameters
 *
 * @param   none
 *
 * output parameters
 *
 * @return  rfStatus_t
 *
 */
byte CC1200::ccGetTxStatus(void)
{
	SendStrobe(SNOP);
	return (SendStrobe(SNOP));
}

/******************************************************************************
 *
 *  @fn       cc120xGetRxStatus(void)
 *
 *  @brief
 *            This function transmits a No Operation Strobe (SNOP) with the
 *            read bit set to get the status of the radio and the number of
 *            available bytes in the RXFIFO.
 *
 *            Status byte:
 *
 *            --------------------------------------------------------------------------------
 *            |          |            |                                                      |
 *            | CHIP_RDY | STATE[2:0] | FIFO_BYTES_AVAILABLE (available bytes in the RX FIFO |
 *            |          |            |                                                      |
 *            --------------------------------------------------------------------------------
 *
 *
 * input parameters
 *
 * @param     none
 *
 * output parameters
 *
 * @return    rfStatus_t
 *
 */
byte CC1200::ccGetRxStatus(void)
{
	SendStrobe(SNOP | RADIO_READ_ACCESS);
	return (SendStrobe(SNOP | RADIO_READ_ACCESS));
}

byte CC1200::bytesAvailable()
{
	byte rxBytes = 0;
	// Read number of bytes in RX FIFO
	ccReadReg(REG_NUM_RXBYTES, &rxBytes, 1);
	return rxBytes;
}

void CC1200::clearRXFIFO()
{
	setState(CC1200_IDLE);
	SendStrobe(SFRX);
	setState(CC1200_RX);
#ifdef DEBUG
	Serial.println("PACKET ERROR, CLEARED RX FIFO");
#endif
}

boolean CC1200::readRX(Queue& rxBuffer, byte bytesToRead)
{
	byte marcState;
	boolean dataGood = false;

	//Serial.println("Bytes to Read: ");
    //Serial.println(bytesToRead);

	if (rxBuffer.bytesAvailable() < bytesToRead)
	{
#ifdef DEBUG
		Serial.print("QUEUE CAPACITY INSUFFICIENT. PACKET OF LENGTH ");
		Serial.print(bytesToRead);
		Serial.println(" BYTES WILL NOT FIT IN QUEUE.");
#endif
		clearRXFIFO();
		return false;
	}

	// Read MARCSTATE to check for RX FIFO error
	ccReadReg(REG_MARCSTATE, &marcState, 1);

	// Mask out MARCSTATE bits and check if we have a RX FIFO error
	if ((marcState & 0x1F) == RX_FIFO_ERROR)
	{
#ifdef DEBUG
		Serial.println("RX FIFO ERR");
#endif
		// Flush RX FIFO
		setState(CC1200_IDLE);
		SendStrobe(SFRX);
		setState(CC1200_RX);
	}
	else
	{
		// Read n bytes from RX FIFO
		ccReadRxFifo(rxBuffer.array, (uint16_t)bytesToRead, rxBuffer.end);

		// Check CRC ok (CRC_OK: bit7 in second status byte)
		// This assumes status bytes are appended in RX_FIFO
		// (PKT_CFG1.APPEND_STATUS = 1)
		// If CRC is disabled the CRC_OK field will read 1
		if (rxBuffer.array[rxBuffer.end + bytesToRead - 1] & 0x80)
		{
			//  Serial.println("CRC OK");
			// Update packet counter
			//  packetCounter++;
			dataGood = true;
			rxBuffer.end = rxBuffer.end + bytesToRead;
		}
		else
		{
#ifdef DEBUG
			Serial.println("CRC BAD");
#endif
			clearRXFIFO();
		}
	}

#ifdef DEBUG

	if (dataGood)
	{
		Serial.print("RX buffer contains ");
		Serial.print(bytesToRead);
		Serial.println(" bytes.");
		printBuffer(rxBuffer.array, bytesToRead);
	}

#endif
	return dataGood;
}

/****** Debug member functions **********************************************/

#ifdef DEBUG
/**
 * Print a register value for debugging
 **/
void CC1200::printReg(uint16_t reg, String name)
{
	byte val = ReadReg(reg);
	Serial.print("Register ");
	Serial.print(name);
	Serial.print(" at address 0x");
	Serial.print(reg, HEX);
	Serial.print(" is 0x");
	Serial.println(val, HEX);
}

void CC1200::printBuffer(byte *buffer, byte length)
{
	for (int i = 0; i < length; i++)
	{
		Serial.print("[0x");
		Serial.print(buffer[i], HEX);
		Serial.print("]");
	}
	Serial.println();
}

#endif /* DEBUG */


#ifdef RADIOTESTMODE
void CC1200::txTestMode()
{
	//TX test mode settings
	setState(CC1200_TX); //TX test mode ONLY

	while (1);
}

void CC1200::rxTestMode()
{
	//RX test mode settings
#define SYNCHRONOUS //comment this out for transparent mode (no timing recovery)
#ifdef SYNCHRONOUS
	WriteReg(REG_PREAMBLE_CFG1, 0); //No preamble
	WriteReg(REG_SYNC_CFG1, 0x0A); //No sync uint16_t
	WriteReg(REG_PKT_CFG2, 0x1); //Synchronous serial mode
#else
	WriteReg(REG_PKT_CFG2, 0x3); //Packet mode = transparent
	WriteReg(REG_MDMCFG0, 0x45); //Transparent mode on
#endif
	WriteReg(REG_MDMCFG1, 0x02); //FIFO OFF
	WriteReg(REG_IOCFG3, 0x09); //send symbols out pin D18
}
#endif /* RADIOTESTMODE */
