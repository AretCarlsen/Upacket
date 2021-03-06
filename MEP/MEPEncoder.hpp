// Copyright (C) 2010, Aret N Carlsen (aretcarlsen@autonomoustools.com).
// MEP packet handling (C++).
// Licensed under GPLv3 and later versions. See license.txt or <http://www.gnu.org/licenses/>.

// MEPEncoder class declaration

#pragma once

#include "MEP.hpp"
#include <Upacket/MAP/MAP.hpp>
#include <ATcommon/StateMachine/StateMachine.hpp>
#include <ATcommon/DataTransfer/DataTransfer.hpp>
#include <MapOS/TimedScheduler/TimedScheduler.hpp>

/*
#ifndef NULL
#define NULL 0
#endif
*/

#ifndef DEBUGprint_MEP
#define DEBUGprint_MEP(...)
#endif

namespace MEP {

typedef uint8_t OutputBufferCapacity_t;

// Encode a packet to an outgoing bytestream.
class MEPEncoder : public MAP::MAPPacketSink, public Process {
private:
// Current control prefix
  MAP::Data_t controlPrefix;

// Current packet
  MAP::OffsetMAPPacket offsetPacket;
// Current packet data
  MAP::Data_t *packetData;

// State machine
  StateMachine state;
  bool controlCollisionInProgress;

// MEP-encoded outgoing data
//  DataTransfer::DataSink<MEP::Data_t, Status::Status_t> *outputSink;
  DataStore::RingBuffer<MEP::Data_t, OutputBufferCapacity_t> *outputSink;

public:

// Constructor
  //MEPEncoder(DataTransfer::DataSink<MEP::Data_t, Status::Status_t> *new_outputSink)
  MEPEncoder(DataStore::RingBuffer<MEP::Data_t, OutputBufferCapacity_t> *new_outputSink, MAP::Data_t new_controlPrefix = MEP::DefaultControlPrefix)
  : controlPrefix(new_controlPrefix),
    offsetPacket(NULL, 0),
    controlCollisionInProgress(false),
    outputSink(new_outputSink)
  {
    STATE_MACHINE__RESET(state);
  }

// Accept a packet to be encoded.
// Non-blocking. May return Good, Busy, or Bad (rejected).
  Status::Status_t sinkPacket(MAP::MAPPacket* const new_packet, MAP::MAPPacket::HeaderOffset_t headerOffset);

// Continue encoding the packet.
  Status::Status_t process();

// Reset the encoder.
  void reset(){
    DEBUGprint_MEP("MEPe: rset\n");
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

