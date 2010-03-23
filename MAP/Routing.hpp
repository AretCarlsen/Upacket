// MAP routing devices
//
// StaticLocalMAPServerBinding; BroadcastRouter; MAPPacketStore; SimpleServerProcess

#pragma once

#include "MAP.hpp"
#include "../../Process/Process.hpp"
#include "../../DataStore/Buffer.hpp"

// Stores a MAP server binding, static-local addresses only.
class StaticLocalMAPServerBinding : public MAP::MAPPacketSink {
public:
// Only addresses up to 255
  typedef uint8_t Address_t;

private:
  Address_t address;
  MAP::MAPPacketSink *packetSink;

public:
  StaticLocalMAPServerBinding(Address_t new_address, MAPPacketSink *new_packetSink)
  : address(new_address),
    packetSink(new_packetSink)
  {
/*
  // Address must be of type Local-Static and fit in one byte.
    if(   addressType != MAP::AddressType__DeviceLocalStatic
       || address > 127)
      return MEP::Status__Bad;
*/
  }
  
  Status::Status_t sinkPacket(MAP::MAPPacket *packet){
// Make sure packet includes a destination address of type Device-Local-Static,
// and that the address is correct.
    if(   packet->get_addressType() != MAP::AddressType__DeviceLocalStatic
       || !(packet->get_destAddressPresent())
       || ( *(packet->get_destAddress()) != address)
    ){
DEBUGprint("Binding filter: Blocking packet.\n");
      return Status::Status__Bad;
    }

DEBUGprint("Binding filter: Accepting packet. Dest address: %X\n", *(packet->get_destAddress()));
    return packetSink->sinkPacket(packet);
  }
};

// Router which broadcasts received packets to all configured sinks.
class BroadcastRouter : public MAP::MAPPacketSink {
private:
// Max of 255 sinks
  typedef uint8_t Capacity_t;

  DataStore::StaticArrayBuffer<MAPPacketSink*, Capacity_t> sinks;

public:

  BroadcastRouter(MAPPacketSink **sinks_buffer, Capacity_t sinks_buffer_capacity)
  : sinks(sinks_buffer, sinks_buffer_capacity)
  { }

  Status::Status_t addSink(MAPPacketSink *new_sink){
    return sinks.sinkData(new_sink);
  }

// Signal/slot style broadcasting.
  Status::Status_t sinkPacket(MAP::MAPPacket *packet){
  // Note packet in use.
    MAP::referencePacket(packet);

  // Does not stop after an acceptance.
    for(MAPPacketSink **packetSink = sinks.front(); packetSink < sinks.back(); packetSink++){
packet->get_checksumPresent();
      (*packetSink)->sinkPacket(packet);
    }

  // Free packet.
    MAP::dereferencePacket(packet);

    return Status::Status__Good;
  }
};

class SimpleServerProcess : public MAP::MAPPacketSink {  // public Process
protected:
  MAP::MAPPacket *packet;

public:

  SimpleServerProcess()
  : packet(NULL)
  { }

  Status::Status_t sinkPacket(MAP::MAPPacket *new_packet){
    if(packet != NULL)
      return Status::Status__Bad;

    packet = new_packet;
// Note packet in use.
    MAP::referencePacket(packet);

    return Status::Status__Good;
  }

  void finishedWithPacket(){
    packet->sinkStatus(Status::Status__Complete);
    MAP::dereferencePacket(packet);
    packet = NULL;
  }
};


