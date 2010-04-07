// MEPEncoder class declaration

#pragma once

#include "MEP.hpp"
#include "../MAP/MAP.hpp"
#include "../../StateMachine/StateMachine.hpp"
#include "../../DataTransfer/DataTransfer.hpp"
#include "../../Process/Process.hpp"

#ifndef NULL
#define NULL 0
#endif

namespace MEP {

// Encode a packet to an outgoing bytestream.
class MEPEncoder : public MAP::MAPPacketSink, public Process {
private:
// Current control prefix
  MAP::Data_t controlPrefix;

// Current packet
  MAP::OffsetMAPPacket offsetPacket;
// Current packet data
  Packet::Data_t *packetData;

// State machine
  StateMachine state;
  bool controlCollisionInProgress;

// MEP-encoded outgoing data
//  DataTransfer::DataSink<MEP::Data_t, Status::Status_t> *outputSink;
  DataStore::RingBuffer<MEP::Data_t, Status::Status_t> *outputSink;

public:

// Constructor
  //MEPEncoder(DataTransfer::DataSink<MEP::Data_t, Status::Status_t> *new_outputSink)
  MEPEncoder(DataStore::RingBuffer<MEP::Data_t, Status::Status_t> *new_outputSink, MAP::Data_t new_controlPrefix = MEP::DefaultControlPrefix)
  : controlPrefix(new_controlPrefix),
    offsetPacket(NULL, 0),
    controlCollisionInProgress(false),
    outputSink(new_outputSink)
  {
    STATE_MACHINE__RESET(state);
  }


// Accept a packet to be encoded.
// Non-blocking. May return Good, Busy, or Bad (rejected).
  Status::Status_t sinkPacket(MAP::MAPPacket* const new_packet, MAP::MAPPacket::Offset_t headerOffset);

// Continue encoding the packet.
  Status::Status_t process();

// Reset the encoder.
  void reset(){
DEBUGprint("MEPEncoder: Resetting.\n");
    STATE_MACHINE__RESET(state);
    if(offsetPacket.packet != NULL){
      MAP::dereferencePacket(offsetPacket.packet);
      offsetPacket.packet = NULL;
    }
    controlCollisionInProgress = false;
  }

  bool isBusy() const{
    return (offsetPacket.packet != NULL);
  }
};

// End namespace: MEP
}

