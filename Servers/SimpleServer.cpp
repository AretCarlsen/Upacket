
#include "SimpleServer.hpp"

// Attempt to prepare a reply packet.
// Returned packet will have at least the specified data_capacity available capacity.
// Will fail if the reply packet cannot be allocated, or if the passed *packet does not
// contain any src addresses to which to reply.
bool SimpleServer::prepareReply(MAP::MAPPacket **replyPacket_ptr_ptr, MAP::MAPPacket *srcPacket, uint16_t data_capacity){
// Make sure at least one src address is found (and copied to the reply packet).
// Otherwise, where will the reply packet go?
  bool successful_preparation = false;

  MAP::MAPPacket *replyPacket;

  DEBUGprint("SS:pR: try alloc pack\n");

  // Attempt to allocate reply packet.
  if(! MAP::allocateNewPacket(&replyPacket, InitialReplyPacketCapacity + data_capacity))
    return false;

  // Step through headers. Stop at actual data (after last MAP headers), when get_next_header returns NULL.
  for(MAP::Data_t* srcHeader = srcPacket->get_first_header(); srcHeader != NULL; srcHeader = srcPacket->get_next_header(srcHeader)){
  // If header indicates a source address is present, sink it to the reply packet as a dest address.
    if(MAP::get_srcAddressPresent(*srcHeader)){
      // Assume copy will fail until proven otherwise.
      successful_preparation = false;

      DEBUGprint("SS:pR: cp src->dest\n");

      // This implementation only supports addy types 0 to 14 (not Expanded).
      if(MAP::get_addressType(*srcHeader) > 14)
        break;

      // Sink a destAddressPresent header to the replyPacket.
      if(! replyPacket->sinkExpand(MAP::DestAddressPresent_Mask | MAP::get_addressType(*srcHeader), IncrementReplyPacketCapacity))
        break;

//      DEBUGprint("prepareReply: Header byte sunk.\n");

      // Copy src address from incoming packet to outgoing replyPacket (as dest address).
      for(MAP::Data_t *src_data_ptr = srcPacket->get_srcAddress(srcHeader); src_data_ptr < srcPacket->back(); src_data_ptr++){
      // Attempt to sink the dest address byte.
        if(! replyPacket->sinkExpand(*src_data_ptr, IncrementReplyPacketCapacity))
          break;

      // Stop copying address if have reached last C78 byte.
        if(Code78::isLastByte(*src_data_ptr)){
      // Note that a src address was found and copied successfully.
          successful_preparation = true;
          break;
        }
      }

  // Check for errors
      if(successful_preparation){
        DEBUGprint("SS:pR: cmplt cp src->dest\n");
      }else{
        DEBUGprint("SS:pR: failed cp src->dest\n");
        break;
      }
    }
  }

// Make sure sufficient data capacity is left available.
  if(successful_preparation && (replyPacket->get_availableCapacity() < data_capacity)){
    DEBUGprint("SS:pR: atmpting expnsn\n");
  // Abort if cannot leave sufficient data capacity available.
    if(! replyPacket->set_availableCapacity(data_capacity))
      successful_preparation = false;
  }

  // Problems, or no src addresses found? Abort.
  if(! successful_preparation){
    DEBUGprint("SS:pR: prep failed\n");
    delete replyPacket;
    return false;
  }

// Success!
  DEBUGprint("SS:pR: prep succeed\n");
  *replyPacket_ptr_ptr = replyPacket;
  return true;
}

