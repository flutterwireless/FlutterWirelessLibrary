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

#include "Network.h"


Radio radio;
byte pinState=LOW;
volatile bool hopNow = false;
volatile bool updateSysTick = false;
static const uint32_t bands[] = FREQ_BANDS;
static const uint32_t numChannels[] = NUM_CHANNELS;



#define HOPTIME 40 //ms per hop
#define TXTIME 2 //ms into hop
#define TIMING_ADJUST 6900-3180 //us to add for timing adjustment



//Constructor
Network::Network(){
  //do any class initialization here
	networkName="";
	networkStatus = DISABLED;
	networkSeed=0;
	address = 0;
	pendingRxPacket = 0;
	band = 0;
  channel = 0;
  channelIndex = 0;
  scheduledMicros = 0;
  scheduledTask = NO_TASK;
  txBlocked = false;
  queuedTXCommands=0;
  radioState = IDLE;
}

//configuration functions

void Network::setNetworkName(String _networkName)
{
	networkName=_networkName;
}

void Network::setAddress(int _address)
{
  address = _address;
  radio.setAddress(address);
}

//initialization

boolean Network::init(byte _band) //intialize radio. we could also call this "Reset"?
{
	band = _band;

	if(!radio.enabled)
	{
		radio.enabled = radio.init();
	}
  radioState=RXIDLE;

  return radio.enabled;

}

void Network::connect(uint16_t devices)
{
    if(networkName=="")
  {
    setChannel(ORPHAN_CH);
    networkStatus=ORPHAN_SEARCH;
  }else
  {
    generateChannelList(networkName);
    setChannelByIndex(TIMING_CH_INDEX);
    if(address==MASTER_ADDRESS)
    {
      networkStatus=NORMAL_OPERATION;
    }else
    {
      networkStatus=SYNC_WAIT;
      //networkStatus=NORMAL_OPERATION; //TODO: This is a hack. Please remove when Sync Wait mode is working.
    }
    //printChannelList();
    
  }
	//requestHost(devices);
}

void Network::requestHost(uint16_t devices)
{

  byte txData[6] = {5,BROADCAST_ADDRESS,address,CMD_HOST_REQUEST,(byte)(devices>>8),(byte)(devices && 0xFF)};
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

	int i,temp = 0;

	randomSeed(seed); //initialize Arduino pseudorandom number generator with our seed.

	while(i<51)
	{
	
		temp = random(numChannels[band]);

		int j = 0;

		//this loop makes sure the new channel value is not already in use.
		while(j<i)
		{
			if(channelList[j]==temp || temp==ORPHAN_CH)
			{
				temp=random(numChannels[band]);
				j=0;
			}else
			{
				j++;
			}
		}
		channelList[i]=temp;
		i++;
	}
	

}

uint16_t Network::generateSeed(String name)
{
	uint16_t i,seed=0;
	seed=name[0];
	i++;
	while(name[i]!='\0')
	{
		seed = (seed>>8) | ((uint16_t)(seed<<8)); //swap low and high byte, just for kicks.
		seed = seed ^ name[i];
		i++;
		//generate a seed. Maybe XOR some bytes?
	}

	return seed;
}

void Network::printChannelList()
{
	if(!Serial)
	{
		Serial.begin(115200);
	}

	Serial.print("Network Name: ");
	Serial.print(networkName);
	Serial.print(", Seed: ");
	Serial.println(networkSeed);
	for(int i=0; i<50;i++)
	{
		//Serial.print("Ch Index ");
		Serial.print(i);
		Serial.print(", ");
		Serial.println(channelList[i]);
	}


}



// Radio interface functions

boolean Network::setChannel(uint32_t _channel)
{
  if(_channel>=numChannels[band]) return false;
  channel=_channel;
  return radio.setFrequency(bands[band]+(CHANNEL_SPACE/2)+(uint32_t)_channel*CHANNEL_SPACE+FREQ_ADJ);  
}

boolean Network::setChannelByIndex(uint32_t _channel_index)
{
  channel=channelList[_channel_index];
  channelIndex=_channel_index;
  return setChannel(channel);
}

void Network::printTime()
{
  Serial.print("Time: ");
  Serial.print(time.seconds);
  Serial.print("s, ");
  Serial.print(time.millis);
  Serial.println("ms.");
}


boolean Network::softInt() //software interrupt
{
  interrupts();


  if(time.millis==0) 
    {
     // printTime();
    }
  //if process was only fired to run a scheduled task, the following condition will be true
  if(scheduledMicros==0 && scheduledTask!=NO_TASK)
  {
    //run our task.
    if(scheduledTask==TX_TASK)
    {
      if(txBlocked==false)
      {
        scheduledTask=NO_TASK;
        txNext();
      }
    }
  }

  if(rxPending==true)
  {
    rxPending=false;
    #ifdef DEBUG1
    Serial.println("Read RX"); 
    #endif
    byte packetLength=readPacket();
    if(packetLength>0)
    {
      #ifdef DEBUG1
      Serial.print("Read "); 
      Serial.print(packetLength); 
      Serial.print(" bytes from radio."); 
      #endif
      processRXPacket(packetLength);
    }else if(packetLength<0)
    {
      queuedRXPackets=0;
      rxBuffer.clear();
    }
  }


   switch(networkStatus)
  {
  	case DISABLED:
	  	return true;
	  	break;
	case ORPHAN_SEARCH:
		if(time.millis==0 || time.millis==500) //every 500mS, we send out an orphan search packet
		{
			transmitOrphanSearchPacket();
		}
		break;
	case ORPHAN_WAIT:
		break;
	case SYNC_WAIT:
    processNormalOperation();
		break;
	case NORMAL_OPERATION:
		processNormalOperation();
		break;
  }

  
  return true;
}

void Network::txNext()
{

  if(queuedTXCommands>0)
  {
     if(commandQueueTX[0][0]==QUEUE_TX)
    {
      transmitPacket(0);
    }
  }

}

void Network::transmitPacket(int index)
{
  radio.transmit(txBuffer.array, commandQueueTX[index][1], commandQueueTX[index][2]);
  radioState = TXPENDING;
  dequeueTXBytes(index);
}

void Network::dequeueTXBytes(int index)
{
  if(index==0)
  {
    int length = commandQueueTX[index][2];
    int i=0;
    for(i=0;i<commandQueueTX[index][2];i++)
    {
      txBuffer.read();
    }

    for(i=0;i<queuedTXCommands-1;i++)
    {
      commandQueueTX[i][1]=commandQueueTX[i+1][1]-length;
      commandQueueTX[i][2]=commandQueueTX[i+1][2];
    }
    queuedTXCommands--;
  }


}


void Network::dequeueRXPacket(int index)
{
  if(index==0)
  {
    int length = packetQueueRX[index][2];
    int i=0;
    for(i=0;i<packetQueueRX[index][2];i++)
    {
      rxBuffer.read();
    }

    for(i=0;i<queuedRXPackets-1;i++)
    {
      packetQueueRX[i][1]=packetQueueRX[i+1][1]-length;
      packetQueueRX[i][2]=packetQueueRX[i+1][2];
    }
    queuedRXPackets--;
  }


}

void Network::processNormalOperation()
{

 // Serial.println("blip"); 
  if(hopNow==true)
  {
    hopNow = false;
  //  hop();
  }



  if(time.millis%HOPTIME==TXTIME)
  {

    #ifdef TESTMODE
    switch(txTestMode)
    {
      case 1:
      radio.txBytes(0);
      break;
      case 2:
      radio.txBytes(TESTBYTES);
      break;
    }
    #endif

    if(channelIndex == TIMING_CH_INDEX && address == MASTER_ADDRESS)
    {
     // transmitSyncPacket(0x00);
    }

    if(channelIndex!= TIMING_CH_INDEX)// && time.micros%HOPTIME <=45000)
    {
     // Serial.println("Processing Queue");
     // processQueue();
     // txNext();
    }
  }

  #ifdef DEBUG1
  if(time.millis%1000==0)
  {
  printChipStatus();
  radio.printMARC();
  }
  #endif

}

int Network::readPacket()
{
  byte count = radio.bytesAvailable();

 //  Serial.print("Count: "); 
 //  Serial.print(count); 

  if(count>0)
  {
    if(radio.readRX(rxBuffer,count))
    {
      return count;
    }else
    {
      return -1;
    }
  }
	return 0;
}

byte Network::processRXPacket(byte packetLength)
{


  int commandByteIndex= rxBuffer.bytesEnd()-packetLength + 3;
  

  int command = rxBuffer.array[commandByteIndex];

  Serial.print("Command Byte. 0x");
  Serial.println(command, HEX);

  switch(command)
  {
    case CMD_SYNCTIME:

    break;
    case CMD_USER_ARRAY:
      queueRXPacket(packetLength);
      break;

  }


  return 0;
}

byte Network::queueRXPacket(byte packetLength)
{

  packetQueueRX[queuedRXPackets][0]=RX_PACKET;
  packetQueueRX[queuedRXPackets][1]=rxBuffer.bytesEnd()-packetLength;
  packetQueueRX[queuedRXPackets][2]=packetLength;
  

  #ifdef DEBUG
  Serial.print("Received Packet, Index #");
  Serial.print(queuedRXPackets);
  Serial.print(", RX Buffer bytes: ");
  Serial.println(rxBuffer.bytesEnd());
  #endif

  queuedRXPackets++;

  if(rxBuffer.bytesEnd()>100 || queuedRXPackets == RX_PACKET_QUEUE_SIZE)
  {
    queuedRXPackets=0;
    rxBuffer.clear();
  }

	return 0;
}

//user data functions
int Network::dataAvailable()
{
  return rxBuffer.bytesEnd();
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
  for(i=0;i<packetSize;i++)
  {
    array[i]=rxBuffer.array[i];
  }
Serial.println("");
  Serial.println("----------------");
  Serial.println("");



  return packetSize;
}


//system work functions

boolean Network::hop()
{
  //channel = (time.micros*200)/1000000;

  //pinDebug(7, HIGH);

  if(channelIndex==TIMING_CH_INDEX && networkStatus!=NORMAL_OPERATION && address != MASTER_ADDRESS)
  {
      return false; //don't hop if sync not recieved
  }

    channelIndex++;

  if(channelIndex>=50)
  {
    channelIndex=0;
  }else{
  
  }

/*
  if(channel == BASE_CHANNEL)
  {
    synchronized=false;
  }  
*/




    #ifdef DEBUG1
         Serial.print("Setting radio to channel ");
         Serial.println(channelIndex);
    #endif
    int errors = 0;
    while(setChannelByIndex(channelIndex)==false)
    {
      errors++;
      if(errors>5)
      {
        //do something if there is an error.
        #ifdef DEBUG
         Serial.println("Set Frequency Failed!");
       #endif
        return false;
       // break;
      }
    }

  //  pinDebug(7, LOW);
    return true;
}


boolean Network::tickInterrupt() 
{
  interrupts();
  //pinDebug(6, HIGH);

  //deal with task scheduling
  if(scheduledTask!=NO_TASK)
  {
    if(scheduledMicros<=1000) //less than 1000 means we are in the middle of a milisecond and it is time to fire. exactly 1000 means we hit a new millisecond and we must tick down to zero and fire.
    {
      if(scheduledMicros!=1000) setMicros(1000-scheduledMicros); //we are in the middle of a millisecond tick. Tell the system to finish that tick.
      scheduledMicros=0;
      softInterrupt();
      return false; //tell system not to update its own millisecond timer for this tick
    }

    scheduledMicros-=1000;
    
    if(scheduledMicros<1000)
    {
      setMicros(scheduledMicros); //set the timer to fire at the scheduled time
      updateSysTick = false;
    }

  }


  time.millis+=1;

  if(time.millis >= 1000) // increment millis and reset if it rolls over
  {
    time.millis=0;
    time.seconds++;
  }


  if(updateSysTick==true)
  {
    setMicros(1000); //reset system tick to 1ms if it was changed to adjust timing
    updateSysTick = false;
  }

  int tickTime = time.millis%HOPTIME;

  switch(networkStatus)
  {
    case DISABLED:
	  	break;
	 case ORPHAN_SEARCH:
		if(time.millis==0 || time.millis==500)
		{
      softInterrupt();
		}
		break;
	case ORPHAN_WAIT:
		break;
	case SYNC_WAIT:
		break;
	case NORMAL_OPERATION:
      switch(tickTime)
      {
        case 0:
        hopNow = true;
        if(queuedTXCommands>0)
        {
          scheduledTask=TX_TASK;
          scheduledMicros=3000;
        }
        case TXTIME:
        softInterrupt();
        break;
      }
		break;
  }



  //pinDebug(6,LOW);
  
  return true; //system will update its own millisecond timer if this is true.
}

void Network::pinDebug(int pin, int value)
{
  #ifdef PIN_DEBUG
  digitalWrite(pin,value);
  #endif
}

int Network::radioInterrupt()
{
  interrupts();
  
  pinState = digitalRead(GDO0_PIN);
  if(pinState==HIGH)
  {
   // lastPacketTime.micros=time.micros;
   // lastPacketTime.seconds=time.seconds;
    //Serial.print(" HIGH ");
    switch(radioState)
    { 
        case RXIDLE:
        radioState=RXACTIVE;
        //pendingRxPacket=queueRX();
        //pinDebug(9,HIGH);

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

  }else
  {
    //Serial.print(" LOW ");
       switch(radioState)
    { 
        case RXACTIVE:
        radioState = RXIDLE;
        rxPending = true;
       // pinDebug(9, HIGH);
        softInterrupt();
        break;
        case TXACTIVE:
        radioState = RXIDLE;
        break;
        default:
        //this is some kind of error
       // errors++;
        radioState = RXIDLE;
        break;

    }  
    

  }
 // Serial.println(radioState);

  return 0;

}


void Network::transmitOrphanSearchPacket() //sends packet with just data (no commands added)
{

  byte txData[4] = {3,BROADCAST_ADDRESS,address,CMD_ORPHAN_REQUEST};

  radio.transmit(txData, 4);
  radioState = TXPENDING;
}

void Network::transmitSyncPacket(byte destaddress) //sends packet with just data (no commands added)
{

  uint32_t _micros = micros()%1000;
  uint32_t _millis = time.millis;
  uint32_t _seconds = time.seconds;

  byte txData[11] = {10,BROADCAST_ADDRESS,address,CMD_SYNCTIME,0, (byte)((_micros)/4),(byte)(((_millis)>>2) & 0xFF) ,(byte)((_millis<<6) & 0xC0) | (_seconds>>24 & 0x3F) ,(byte)(_seconds>>16 & 0xFF) ,(byte)(_seconds>>8 & 0xFF) ,(byte)(_seconds & 0xFF)};

  radio.transmit(txData, 11);
  radioState = TXPENDING;

}

int Network::queueDataPacket(byte command, byte *tx, byte length, byte destaddress) //sends packet with just data (no commands added)
{
  length +=5; //We add 5 bytes to the packet

  queueCommand(TX_PACKET, txBuffer.bytesEnd(), length);


  txBuffer.write(length-1); //packet length byte not included in length calculation
  txBuffer.write(destaddress);
  txBuffer.write(address);
  txBuffer.write(command);
  txBuffer.write(0); //an extra byte, currentl not in use.

  byte i;
  for(i=5; i<length; i++)
  {
    txBuffer.write(tx[i-5]); 
  }

  
  return length;
}

void Network::queueCommand(byte cmd, byte start, byte length)
{

  if(cmd==QUEUE_TX)
  {
    if(queuedTXCommands+1<COMMAND_QUEUE_SIZE)
    {
    commandQueueTX[queuedTXCommands][0]=cmd;
    commandQueueTX[queuedTXCommands][1]=start;
    commandQueueTX[queuedTXCommands][2]=length;
    queuedTXCommands++;
    Serial.print("Queuing Command "); 
      Serial.print(queuedTXCommands, DEC); 
      Serial.print(" = ");
      Serial.print(cmd, DEC); 
      Serial.print(" ");
      Serial.print(start, DEC); 
      Serial.print(" ");
      Serial.println(length, DEC); 
      
    }else
    {
     // Serial.print("ERR: Cannot Queue Command, queue full."); 
    }
  }
 

}


void Network::setMicros(uint32_t value) 
{
  if (SysTick_Config((SystemCoreClock/1000) * (value / 1000.0f)))
  {
    // Capture error
    while (true);
  }
  if(value!=1000)
  {
    updateSysTick = true;
  }
  return;
}

