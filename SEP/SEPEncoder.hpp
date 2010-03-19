// SEPEncoder class declaration

#pragma once

#include "SEP.hpp"
#include "../Bpacket/Bpacket.hpp"
#include "../../StateMachine/StateMachine.hpp"
#include "../../DataTransfer/DataTransfer.hpp"
#include "../../Process/Process.hpp"

#ifndef NULL
#define NULL 0
#endif

namespace SEP {

// Encode a packet to an outgoing bytestream.
class SEPEncoder : public Packet::BpacketSink, public Process {
private:
// Current packet
  Packet::Bpacket* packet;

// State machine
  StateMachine state;
  bool controlCollisionInProgress;

// SEP-encoded outgoing data
  DataTransfer::DataSink<SEP::Data_t, Status::Status_t> *outputSink;

public:

// Constructor
  SEPEncoder(DataTransfer::DataSink<SEP::Data_t, Status::Status_t> *new_outputSink)
  : outputSink(new_outputSink),
    packet(NULL),
    controlCollisionInProgress(false)
  {
    STATE_MACHINE__RESET(state);
  }


// Accept a packet to be encoded.
// Non-blocking. May return Good, Busy, or Bad (rejected).
  Status::Status_t sinkPacket(SEP::Packet::Bpacket *new_packet);


// Continue encoding the packet.
  Status::Status_t process();

// Reset the encoder.
  void reset(){
    STATE_MACHINE__RESET(state);
    packet = NULL;
    controlCollisionInProgress = false;
  }

  void isBusy(){
    return (packet != NULL);
  }
};

// End namespace: SEP
}

