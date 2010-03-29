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
// Current packet
  MAP::MAPPacket *packet;
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
  MEPEncoder(DataStore::RingBuffer<MEP::Data_t, Status::Status_t> *new_outputSink)
  : packet(NULL),
    controlCollisionInProgress(false),
    outputSink(new_outputSink)
  {
    STATE_MACHINE__RESET(state);
  }


// Accept a packet to be encoded.
// Non-blocking. May return Good, Busy, or Bad (rejected).
  Status::Status_t sinkPacket(MAP::MAPPacket *new_packet);

// Continue encoding the packet.
  Status::Status_t process();

// Reset the encoder.
  void reset(){
DEBUGprint("MEPEncoder: Resetting.\n");
    STATE_MACHINE__RESET(state);
    if(packet != NULL){
      MAP::dereferencePacket(packet);
      packet = NULL;
    }
    controlCollisionInProgress = false;
  }

  bool isBusy(){
    return (packet != NULL);
  }
};

// End namespace: MEP
}

