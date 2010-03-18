// SEPDecoder class
//
// Decodes an SEP-encoded stream into PushSinkPacket events.

#pragma once

#include "SEP.hpp"
#include "../Bpacket/Bpacket.hpp"
#include "../../DataTransfer/DataSink.hpp"
#include "../../StateMachine/StateMachine.hpp"

// Begin SEP namespace
namespace SEP {

// Decode packets from an encoded byte stream.
class SEPDecoder : public DataTransfer::DataSink<uint8_t, DataTransfer::Status_t> {
private:
// State machine
  StateMachine state;

  BpacketSink *packetSink;
  Bpacket *packet;

// Initial packet capacity, in bytes
  static const uint8_t PacketCapacity__Initial = 20;
// Packet resizing increment, in bytes
  static const uint8_t PacketCapacity__Increment = 10;

public:

// Constructor
  SEPDecoder(BpacketSink *new_packetSink)
  : packetSink(new_packetSink),
    packet(NULL)
  {
    reset();
  }

// Accept SEP-encoded data to be decoded.
  SEP::Status_t sinkData(SEP::Data_t data);

// Reset decoder.
  void reset(){
    STATE_MACHINE__RESET(state);
  }

// Discard the current packet.
  void discardPacket(){
    // Discard the packet
    packet->deconstructor();
    free(packet);
    packet = NULL;
  }

// Allocate a new packet.
  bool generateNewPacket(){
  // Attempt to allocate initial packet
    packet = (Bpacket*) malloc(sizeof(Bpacket));
    if(packet == NULL)
      return false;
    packet->constructor(SEP::PacketCapacity__Initial);
  )

// Expand the current packet.
  bool expandPacket(){
    assert(packet != NULL);

    return packet->setCapacity(packet->getCapacity() + PacketCapacity__Increment))
  }
};

// End namespace: SEP
}

