// MEPDecoder class
//
// Decodes an MEP-encoded stream into PushSinkPacket events.

#pragma once

#include "MEP.hpp"
#include "../../DataTransfer/DataSink.hpp"
#include "../../StateMachine/StateMachine.hpp"
#include "../MAP/MAP.hpp"

// Begin MEP namespace
namespace MEP {

// Decode packets from an encoded byte stream.
class MEPDecoder : public DataTransfer::DataSink<uint8_t, Status::Status_t> {
private:
// State machine
  StateMachine state;

  MAP::MAPPacketSink *packetSink;
  MAP::MAPPacket *packet;

// Initial packet capacity, in bytes
  static const uint8_t PacketCapacity__Initial = 20;
// Packet resizing increment, in bytes
  static const uint8_t PacketCapacity__Increment = 10;

public:

// Constructor
  MEPDecoder(MAP::MAPPacketSink *new_packetSink)
  : packetSink(new_packetSink),
    packet(NULL)
  {
    reset();
  }

// Accept MEP-encoded data to be decoded.
  Status::Status_t sinkData(MEP::Data_t data);

// Reset decoder.
  void reset(){
    STATE_MACHINE__RESET(state);
  }

// Discard the current packet.
  void discardPacket(){
    MAP::dereferencePacket(packet);
    packet = NULL;
  }

// Allocate a new packet.
  bool allocateNewPacket(){
// Attempt to allocate initial packet
    MAP::MAPPacket *newPacket = new MAP::MAPPacket;

// Allocation failed? Return false.
    if(newPacket == NULL)
      return false;

// Attempt to allocate buffer storage
    if(! newPacket->set_capacity(PacketCapacity__Initial)){
      delete newPacket;
      return false;
    }

DEBUGprint("allocateNewPacket: Allocated new packet.\n");

// Save new packet.
    packet = newPacket;
    MAP::referencePacket(packet);

    return true;
  }

// Expand the current packet.
  bool expandPacketCapacity(){
    assert(packet != NULL);

    return packet->set_capacity(packet->get_capacity() + PacketCapacity__Increment);
  }
};

// End namespace: MEP
}

