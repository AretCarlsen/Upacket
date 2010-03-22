// MEPEncoder class definition

#include "MEPEncoder.hpp"

// Debug
#include <stdio.h>

#include "../globals.hpp"

// Begin processing a new packet
Status::Status_t MEP::MEPEncoder::sinkPacket(Packet::Bpacket *new_packet){
  // Busy? Then refuse to accept.
  if(isBusy())
    return Status::Status__Busy;

  // Save packet for later calls to process().
  packet = new_packet;

  // Set packet status.
  packet->sinkStatus(Status::Status__Busy);
  Packet::referencePacket(packet);

  // Packet has been accepted.
  return Status::Status__Good;
}

// Process the current packet.
// Returns Good normally.
Status::Status_t MEP::MEPEncoder::process(){
  // If not busy (no packet in progress), return immediately.
  if(! isBusy())
    return Status::Status__Complete;

// State machine initialization
STATE_MACHINE__BEGIN(state);

  // Initialize
  controlCollisionInProgress = false;
  // Current packet position
  packetData = packet->front();

// Checkpoint: Transmitting data.
STATE_MACHINE__CHECKPOINT(state);

    // Comparison is a little shifty...
  while(packetData <= packet->back()){
    // Consecutive or terminating bytes that collide with the MEP control byte need to be encoded.
    // Check for a byte matching the MEP control prefix (masked) directly following a byte that
    // exactly matches the MEP control character (prefix).
    if(
         controlCollisionInProgress
      && ( (*packetData & MEP::PrefixMask) == MEP::DefaultControlPrefix )
    ){
      // Send the control sequence that encodes the control prefix as a data byte.
        // Check for outgoing buffer overflow, as indicated by a return value other than 0.
      if(outputSink->sinkData(MEP::DefaultControlPrefix | MEP::Opcode__SendControlPrefixAsData) != Status::Status__Good)
        return Status::Status__Good;

      controlCollisionInProgress = false;
    }

    // Send data byte.
      // Check for outgoing buffer overflow.
    if(outputSink->sinkData(*packetData) != Status::Status__Good)
      return Status::Status__Good;
      
    // Check for control prefix collision
    controlCollisionInProgress = (*packetData == MEP::DefaultControlPrefix);

    packetData++;
  }

// Unnecessary Checkpoint: Data transmission complete, about to end packet
STATE_MACHINE__CHECKPOINT(state);

  // Resolve any pending control collision.
  if(controlCollisionInProgress){
    // Attempt sink
    if(outputSink->sinkData(MEP::DefaultControlPrefix) != Status::Status__Good)
      return Status::Status__Good;

    controlCollisionInProgress = false;
  }

// Unnecessary Checkpoint: Any control collisions resolved.
STATE_MACHINE__CHECKPOINT(state);

  // Sink control prefix
  if(outputSink->sinkData(MEP::DefaultControlPrefix) != Status::Status__Good)
    return Status::Status__Good;

// Checkpoint: Control byte sent, preparing to send end packet
STATE_MACHINE__CHECKPOINT(state);

  // Attempt to send end packet
  if(outputSink->sinkData(MEP::DefaultControlPrefix | MEP::Opcode__CompletePacket) != Status::Status__Good)
    return Status::Status__Good;

  // Indicate packet processing is complete.
  packet->sinkStatus(Status::Status__Complete);

  // Reset encoder state.
  reset();

  // All done for now.
  return Status::Status__Good;

STATE_MACHINE__END(state);
}

