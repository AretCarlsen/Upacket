// Copyright (C) 2010, Aret N Carlsen (aretcarlsen@autonomoustools.com).
// MEP packet handling (C++).
// Licensed under GPLv3 and later versions. See license.txt or <http://www.gnu.org/licenses/>.

// MEPEncoder class definition

#include "MEPEncoder.hpp"

// Begin processing a new packet
Status::Status_t MEP::MEPEncoder::sinkPacket(MAP::MAPPacket *new_packet, MAP::MAPPacket::HeaderOffset_t headerOffset){
  DEBUGprint_MEP("MEPe: sP st\n");

  // Busy? Then refuse to accept.
  if(isBusy()){
    DEBUGprint_MEP("MEPe: busy, refusing\n");
    return Status::Status__Busy;
  }

  // Save packet for later calls to process().
  offsetPacket.packet = new_packet;
  offsetPacket.headerOffset = headerOffset;

  // Set packet status.
  offsetPacket.packet->sinkStatus(Status::Status__Busy);
  MAP::referencePacket(offsetPacket.packet);

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
  packetData = offsetPacket.packet->get_header(offsetPacket.headerOffset);

// Checkpoint: Transmitting data.
STATE_MACHINE__AUTOCHECKPOINT(state);

    // Comparison is a little shifty...
  while(packetData < offsetPacket.packet->back()){
    // Consecutive or terminating bytes that collide with the MEP control byte need to be encoded.
    // Check for a byte matching the MEP control prefix (masked) directly following a byte that
    // exactly matches the MEP control character (prefix).
    if(
         controlCollisionInProgress
      && ( (*packetData & MEP::PrefixMask) == controlPrefix )
    ){
      // Send the control sequence that encodes the control prefix as a data byte.
        // Check for outgoing buffer overflow, as indicated by a return value other than 0.
      if(outputSink->sinkData(controlPrefix | MEP::Opcode__SendControlPrefixAsData) != Status::Status__Good)
        return Status::Status__Good;

      controlCollisionInProgress = false;
    }

    // Send data byte.
      // Check for outgoing buffer overflow.
    if(outputSink->sinkData(*packetData) != Status::Status__Good)
      return Status::Status__Good;
      
    // Check for control prefix collision
    controlCollisionInProgress = (*packetData == controlPrefix);

    packetData++;
  }

// Unnecessary Checkpoint: Data transmission complete, about to end packet
STATE_MACHINE__AUTOCHECKPOINT(state);

  // Resolve any pending control collision.
  if(controlCollisionInProgress){
    // Attempt sink
    if(outputSink->sinkData(controlPrefix) != Status::Status__Good)
      return Status::Status__Good;

    controlCollisionInProgress = false;
  }

// Unnecessary Checkpoint: Any control collisions resolved.
STATE_MACHINE__AUTOCHECKPOINT(state);

  // Sink control prefix
  if(outputSink->sinkData(controlPrefix) != Status::Status__Good)
    return Status::Status__Good;

// Checkpoint: Control byte sent, preparing to send end packet
STATE_MACHINE__AUTOCHECKPOINT(state);

  // Attempt to send end packet
  if(outputSink->sinkData(controlPrefix | MEP::Opcode__CompletePacket) != Status::Status__Good)
    return Status::Status__Good;

  // Indicate packet processing is complete.
  offsetPacket.packet->sinkStatus(Status::Status__Complete);

  // Reset encoder state.
  reset();

  // All done for now.
  return Status::Status__Good;

STATE_MACHINE__END(state); return Status::Status__Bad;
}

