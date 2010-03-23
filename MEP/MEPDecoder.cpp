// MEPDecoder class implementation

#include "MEPDecoder.hpp"
#include "../../StateMachine/StateMachine.hpp"

// If Busy is returned, then the Decoder was not able to allocate sufficient memory.
// The caller may try again.
Status::Status_t MEP::MEPDecoder::sinkData(MEP::Data_t data){

// Start a new packet, if necessary.
  if(packet == NULL){
DEBUGprint("sinkData: Processing new packet.\n");
  // Attempt to allocate a new packet.
    if(! allocateNewPacket())
      return Status::Status__Busy;
  }
/*
DEBUGprint("Init packet size: %X\n", packet->get_size());
for(uint8_t i = packet->get_size(); i > 0; i --)
  DEBUGprint("Packet byte: %X\n", packet->get(i));
*/

STATE_MACHINE__BEGIN(state);
// Regular data mode.

  // Check for control byte match. (The control byte is equal to the unmasked control prefix.)
  // If no match, byte is just regular data to be relayed.
  if(data != DefaultControlPrefix){
    // Attempt to enlarge packet, if necessary
    if( packet->is_full() && (!expandPacketCapacity()) ) 
      return Status::Status__Busy;
    packet->append(data);

    return Status::Status__Good;
  }

  STATE_MACHINE__CHECKPOINT_RETURN(state, true);
// Data matches control byte. Control mode.

// Possible opcode received.

  // If the byte received immediately following a control byte does not
  // match the control prefix, then the sender actually intended to send
  // the control byte as data (followed by the new byte).
  if((data & MEP::PrefixMask) != MEP::DefaultControlPrefix){
    // Attempt to enlarge packet, if necessary
    if( (packet->get_availableCapacity() < 2) && (!expandPacketCapacity()) ) 
      return Status::Status__Busy;

    // First, append the control byte itself as (delayed) data.
    packet->append(MEP::DefaultControlPrefix);
    // Then, append the data itsef.
    packet->append(data);

    // Return to data incoming state
    STATE_MACHINE__RESET(state);
    return Status::Status__Good;
  }

// Definite opcode received.

  // Isolate opcode
  data &= MEP::OpcodeMask;

  // Does the opcode indicate to send the control prefix as data?
  if(data == MEP::Opcode__SendControlPrefixAsData){
    // Attempt to enlarge packet, if necessary
    if( packet->is_full() && (!expandPacketCapacity()) ) 
      return Status::Status__Busy;
    packet->append(DefaultControlPrefix);

  // Does the opcode indicate a complete packet?
  }else if(data == MEP::Opcode__CompletePacket){
DEBUGprint("Packet completed. Size: %X\n", packet->get_size());
//for(uint8_t i = packet->get_size(); i > 0; i --)
//  DEBUGprint("Packet byte: %X\n", packet->get(i));
    // Sink completed packet
    packetSink->sinkPacket(packet);
    // Disassociate the packet (in preparation for the next packet)
    discardPacket();

  // Does the opcode indicate a bad packet?
  }else if(data == MEP::Opcode__BadPacket){
    // Discard the packet
    discardPacket();

  // Double control char received. Remain in control mode.
  }else
    return Status::Status__Good; 

  // Return to data incoming state.
  reset();
  return Status::Status__Good;

STATE_MACHINE__END(state); return Status::Status__Bad;
}

