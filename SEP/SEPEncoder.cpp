// SEPEncoder class definition

#include "SEPEncoder.hpp"

#ifndef NULL
#define NULL 0
#endif

// Debug
#include <stdio.h>

// Begin processing a new packet
SEP::Status_t SEP::SEPEncoder::sinkPacket(SEP::Packet *new_packet){
  // Busy? Then refuse to accept.
  if(isBusy())
    return SEP::Status__Busy;

  // Save packet for later calls to process().
  packet = new_packet;

  // Set packet status.
  packet->sinkStatus(SEP::Status__Busy);

  // Packet has been accepted.
  return SEP::Status__Good;
}

// Process the current packet.
// Returns Good normally.
Process::Status_t SEP::SEPEncoder::process(){
  // If not busy (no packet in progress), return immediately.
  if(! isBusy())
    return SEP::Status__Complete;

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
    // Consecutive or terminating bytes that collide with the SEP control byte need to be encoded.
    // Check for a byte matching the SEP control prefix (masked) directly following a byte that
    // exactly matches the SEP control character (prefix).
    if(
         controlCollisionInProgress
      && ( (*packetData & SEP::PrefixMask) == SEP::DefaultControlPrefix )
    ){
      // Send the control sequence that encodes the control prefix as a data byte.
        // Check for outgoing buffer overflow, as indicated by a return value other than 0.
      if(outputSink->sinkData(SEP::DefaultControlPrefix | SEP::Opcode__SendControlPrefixAsData) != DataTransfer::Status__Good)
        return Process::Status__Good;

      controlCollisionInProgress = false;
    }

    // Send data byte.
      // Check for outgoing buffer overflow.
    if(outputSink->sinkData(*packetData) != DataTransfer::Status__Good)
      return SEP::Status__Good;
      
    // Check for control prefix collision
    controlCollisionInProgress = (packetData == SEP::DefaultControlPrefix);

    packetData++;
  }

// Unnecessary Checkpoint: Data transmission complete, about to end packet
STATE_MACHINE__CHECKPOINT(state);

  // Resolve any pending control collision.
  if(controlCollisionInProgress){
    // Attempt sink
    if(outputSink->sinkData(SEP::DefaultControlPrefix) != DataTransfer::Status__Good)
      return SEP::Status__Good;

    controlCollisionInProgress = false;
  }

// Unnecessary Checkpoint: Any control collisions resolved.
STATE_MACHINE__CHECKPOINT(state);

  // Sink control prefix
  if(outputSink->sinkData(SEP::DefaultControlPrefix) != DataTransfer::Status__Good)
    return SEP::Status__Good;

// Checkpoint: Control byte sent, preparing to send end packet
STATE_MACHINE__CHECKPOINT(state);

  // Attempt to send end packet
  if(outputSink->sinkData(SEP::DefaultControlPrefix | SEP::Opcode__CompletePacket) != DataTransfer::Status__Good)
    return SEP::Status__Good;

  // Indicate packet processing is complete.
  packet->sinkStatus(SEP::Status__Complete);

  // Reset encoder state.
  reset();

  // All done for now.
  return SEP::Status__Good;

STATE_MACHINE__END(state);
}

