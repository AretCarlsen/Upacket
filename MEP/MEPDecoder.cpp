// Copyright (C) 2010, Aret N Carlsen (aretcarlsen@autonomoustools.com).
// MEP packet handling (C++).
// Licensed under GPLv3 and later versions. See license.txt or <http://www.gnu.org/licenses/>.

// MEPDecoder class implementation

#ifndef DEBUGprint_MEP
#define DEBUGprint_MEP(...)
#endif

#include "MEPDecoder.hpp"
#include <ATcommon/StateMachine/StateMachine.hpp>

// If Busy is returned, then the Decoder was not able to allocate sufficient memory.
// The caller may try again.
Status::Status_t MEP::MEPDecoder::sinkData(const MEP::Data_t &data){
//  DEBUGprint_MEP("MEPd: sD, state %d, data X%x\n", state, data);

STATE_MACHINE__BEGIN(state);
// Regular data mode.

// Start a new packet, if necessary.
  if((!discardingPacket) && (packet == NULL)){
    DEBUGprint_MEP("MEPd: new pack\n");
  // Attempt to allocate a new packet.
    if(! allocateNewPacket())
      return Status::Status__Busy;
  }

  // Check for control byte match. (The control byte is equal to the unmasked control prefix.)
  // If no match, byte is just regular data to be relayed.
  if(data != controlPrefix){
    // Attempt to enlarge packet, if necessary
    if((!discardingPacket)
       && (! packet->sinkExpand(data, PacketCapacity__Increment, PacketCapacity__Max))
    ) return Status::Status__Busy;

    return Status::Status__Good;
  }

  STATE_MACHINE__AUTOCHECKPOINT_RETURN(state, Status::Status__Good);
// Data matches control byte. Control mode.

// Possible opcode received.

  // If the byte received immediately following a control byte does not
  // match the control prefix, then the sender actually intended to send
  // the control byte as data (followed by the new byte).
  if((data & MEP::PrefixMask) != controlPrefix){
    // Attempt to enlarge packet, if necessary
//    DEBUGprint_MEP("MEPd: Non-opcode.\n");

    // First, append the control byte itself as (delayed) data.
    // Then, append the data itsef.
    if((!discardingPacket)
       && !(
            packet->sinkExpand(controlPrefix, PacketCapacity__Increment, PacketCapacity__Max)
         && packet->sinkExpand(data, PacketCapacity__Increment, PacketCapacity__Max)
       )
    ) return Status::Status__Busy;

    // Return to data incoming state
    STATE_MACHINE__RESET(state);
    return Status::Status__Good;
  }

// Definite opcode received.

  // Isolate opcode
  uint8_t opcode = data & MEP::OpcodeMask;
//  DEBUGprint_MEP("MEPd: opcode %d\n", opcode);

  // Does the opcode indicate to send the control prefix as data?
  if(opcode == MEP::Opcode__SendControlPrefixAsData){
    // Attempt to enlarge packet, if necessary
//    if( packet->is_full() && (!expandPacketCapacity()) ) 
//      return Status::Status__Busy;
    if((!discardingPacket)
       && (! packet->sinkExpand(controlPrefix, PacketCapacity__Increment, PacketCapacity__Max))
    ) return Status::Status__Busy;

  // Does the opcode indicate a complete packet?
  }else if(opcode == MEP::Opcode__CompletePacket){
    // Sink completed packet
    if(! discardingPacket){
      DEBUGprint_MEP("MEPd: pack cmplt, size %d\n", packet->get_size());
      packetSink->sinkPacket(packet);
    // Disassociate the packet (in preparation for the next packet)
      discardPacket();
    }

  // Does the opcode indicate a bad packet?
  }else if(opcode == MEP::Opcode__BadPacket){
    // Discard the packet
    if(! discardingPacket)
      discardPacket();

  // Double control char received. Remain in control mode.
  }else
    return Status::Status__Good; 

  // Return to data incoming state, and stop discarding packet (if doing so).
  reset();
  return Status::Status__Good;

STATE_MACHINE__END(state); return Status::Status__Bad;
}

