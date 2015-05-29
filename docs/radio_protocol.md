# Flutter Radio Protocol 

NOTE: Documentation is still in Beta 

Flutter Radio Protocol is a Spread Spectrum open source protocol. The goal is a reliable and 
cooperatve (and jam-free) protocol for individual or mesh of devices to work together. 

This document covers the basics of the over the air protocol design.

# General Design: 
The Flutter protocol is a Frequency-hopping spread spectrum (FHSS)[1] protocol, where communication 
between a Coach node and the rest of the team nodes hop channels over time.  Setting 'Coach' or team
membership is bsed on build-time configuration.  Along with 48 general purpous channels, there are two 'master' channels, which nodes use to sync their hops and who is the master of their group. 

## Setting a netwwork 

 - Assign a coach (Address 0x01)
  Select a deivice that will behave as master. That device will act as a master, based on logic/behavior found in Network.cpp.  The coach device will blindly Freq Hops, when hits the first channel sends a timing info packet. 


-  Everything else is a Team device:
Other devices process every packet, and look for a command byte for timing information. When they do find a timing packet, they do some calibration of timing to start joining the frequency-hopping party, so they know where to jump to for the next message in the chain. 

## Timing Calculations 
When a packet is recived, the flutter node saves local system time. The node then Decodes the delta from local system time to absolute time.  By subtracing out the delta, the node know accuratly enough to what system time should have been. Example: 3600ms delay offset for packet travel time,  ~100us accuracy. This is enough to find the right channel at the right time. 


## Packet Data Format

Each Packet of data (a flut) contains the same header information FIZBUZZ

  packet[0] = length-1; //packet length byte not included in length calculation
  packet[1] = destaddress;
  packet[2] = address;
  packet[3] = command;
  packet[4] = network code;


### Command codes:

These are the command codes that have been developed so far. Command codes are for identifying packet type.  Mostly, these are not exposed to end users, and are used internally for data management and system vs. user data flow. 


0x00 - 0x1F - RESERVED 

0x20 - user data, byte array
0x21 - user data, string
0x22 - user data, 32 bit float
0x23 - user data, 64 bit double
0x24 - user data, 32 bit unsigned int
0x25 - user data, 32 bit signed int
0x26 - user data, 64 bit unsigned int
0x27 - user data, 64 bit signed int
0x28 - user data, two 32 bit signed int
0x29 - 0x2F - RESERVED

0x30 - 0x3F - RESERVED

0x40 - system control, orphan host request
0x41 - system control, orphan request response. Contains timing and channel information or orphan-orphan response.
0x42 - system control, network descriptor packet - timing and channel information
0x43 - system control, request for host. If no host is known, one will be elected. Default choice is first to send request for host.
0x44 -  0x4F - RESERVED

0x4F - 0xFF - RESERVED



# Footnotes
[1] https://en.wikipedia.org/wiki/Frequency-hopping_spread_spectrum

