// Copyright (C) 2010, Aret N Carlsen (aretcarlsen@autonomoustools.com).
// MEP packet handling (C++).
// Licensed under GPLv3 and later versions. See license.txt or <http://www.gnu.org/licenses/>.


// MEPDecoder class
//
// Decodes an MEP-encoded stream into PushSinkPacket events.

#pragma once

#include "MEP.hpp"
#include "../../DataTransfer/DataTransfer.hpp"
#include "../../StateMachine/StateMachine.hpp"
#include "../MAP/MAP.hpp"

// Begin MEP namespace
namespace MEP {

// Decode packets from an encoded byte stream.
class MEPDecoder { //: public DataTransfer::DataSink<uint8_t, Status::Status_t> {
private:
// Control prefix
  MAP::Data_t controlPrefix;

// State machine
  StateMachine state;
// Currently discarding a packet?
  bool discardingPacket;

  MAP::MAPPacketSink *packetSink;
  MAP::MAPPacket *packet;

// Allocation pool
  MemoryPool *memoryPool;

// Initial packet capacity, in bytes
  static const uint8_t PacketCapacity__Initial = 20;
// Packet resizing increment, in bytes
  static const uint8_t PacketCapacity__Increment = 10;
// Max allowed packet size, in bytes
  static const uint8_t PacketCapacity__Max = 150;

public:

// Constructor
  MEPDecoder(MAP::MAPPacketSink *new_packetSink, MemoryPool *new_memoryPool, MAP::Data_t new_controlPrefix = MEP::DefaultControlPrefix)
  : controlPrefix(new_controlPrefix),
    packetSink(new_packetSink),
    packet(NULL),
    memoryPool(new_memoryPool)
  {
    assert(packetSink != NULL);
    assert(memoryPool != NULL);
    reset();
  }

// Accept MEP-encoded data to be decoded.
  Status::Status_t sinkData(const MEP::Data_t &data);

// Reset decoder.
  void reset(){
    STATE_MACHINE__RESET(state);
    discardingPacket = false;
  }

// Discard the current packet.
  void discardPacket(){
    if(packet != NULL){
      MAP::dereferencePacket(packet);
      packet = NULL;
    }
    discardingPacket = true;
  }

// Allocate a new packet.
  bool allocateNewPacket(){
  // Attempt to allocate
    if(! MAP::allocateNewPacket(&packet, PacketCapacity__Initial, memoryPool))
      return false;

  // Reference packet
    MAP::referencePacket(packet);

    return true;
  }

/*
// Expand the current packet.
  bool expandPacketCapacity(){
    assert(packet != NULL);

    return packet->set_capacity(packet->get_capacity() + PacketCapacity__Increment);
  }
*/
};

// End namespace: MEP
}

