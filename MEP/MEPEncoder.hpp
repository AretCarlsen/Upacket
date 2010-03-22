// MEPEncoder class declaration

#pragma once

#include "MEP.hpp"
#include "../Bpacket/Bpacket.hpp"
#include "../MAP/MAP.hpp"
#include "../../StateMachine/StateMachine.hpp"
#include "../../DataTransfer/DataTransfer.hpp"
#include "../../Process/Process.hpp"

#ifndef NULL
#define NULL 0
#endif

namespace MEP {

// Encode a packet to an outgoing bytestream.
class MEPEncoder : public Packet::BpacketSink, public MAP::MAPPacketSink, public Process {
private:
// Current packet
  Packet::Bpacket *packet;
// Current packet data
  Packet::Data_t *packetData;

// State machine
  StateMachine state;
  bool controlCollisionInProgress;

// MEP-encoded outgoing data
  DataTransfer::DataSink<MEP::Data_t, Status::Status_t> *outputSink;

// Track if free at end is required.
  bool isMAP;

public:

// Constructor
  MEPEncoder(DataTransfer::DataSink<MEP::Data_t, Status::Status_t> *new_outputSink)
  : outputSink(new_outputSink),
    packet(NULL),
    controlCollisionInProgress(false),
    isMAP(false)
  {
    STATE_MACHINE__RESET(state);
  }


// Accept a packet to be encoded.
// Non-blocking. May return Good, Busy, or Bad (rejected).
  Status::Status_t sinkPacket(Packet::Bpacket *new_packet);
  Status::Status_t sinkPacket(MAP::MAPPacket *new_packet){
    sinkPacket((Packet::Bpacket*) new_packet);
  }

// Continue encoding the packet.
  Status::Status_t process();

// Reset the encoder.
  void reset(){
    STATE_MACHINE__RESET(state);
    Packet::dereferencePacket(packet);
    packet = NULL;
    controlCollisionInProgress = false;
  }

  bool isBusy(){
    return (packet != NULL);
  }
};

// End namespace: MEP
}

