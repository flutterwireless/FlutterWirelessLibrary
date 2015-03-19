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

#ifndef Flutter_h
#define Flutter_h

#include "Arduino.h"
#include "Network.h"
#include "Reset.h"
#include "Queue.h"
//#include "SPI.h"


//System definitions
#define MASTER_ADDRESS 1

//Radio Configuration

//radio states
#define IDLE (byte)0
#define RXIDLE (byte)1
#define RXACTIVE (byte)2
#define TXPENDING (byte)3
#define TXWAIT (byte)3
#define TXACTIVE (byte)4
#define TXFAILED (byte)5
#define TXSUCCEEDED (byte)6


//board configuration
#define RED 255,0,0
#define GREEN 0,255,0
#define BLUE 0,0,255
#define PURPLE 255,0,200
#define YELLOW 255,255,0
#define AQUA 0,255,255
#define LIME 100,200,0
#define BROWN 40,10,10

#ifdef FLUTTER_R2
#define GDO0_PIN 15
#else
#define GDO0_PIN 17
#endif


#define RSSI_OFFSET 81


//test and debugging

#define DEBUG
//#define RADIOTESTMODE
#define PIN_DEBUG


class Flutter
{
    public:


        Flutter();
        volatile boolean initialized;
        boolean synchronized;
        

	    boolean init();
        void setLED(int red, int green, int blue);
    	byte sendData(byte data, byte address);
    	byte sendData(byte data[], byte length, byte address);
    	byte next();
    	void connect(uint16_t devices);


        void setNetworkName(String name);
        void setAddress(int address);

        int interrupt();
        boolean processSoftInt();
        boolean tickInt();

        void ledLightShow();

        int getState();
        int dataAvailable();
        int nextPacket();
        int nextPacketLength();
        int readBytes(byte *array,int packetSize);

        int packetRSSI(byte *array,int packetSize);

    private:
        void setRadioPins();
        void setFlutterPins();
        void setInterrupts();
};


#endif

