// Copyright (C) 2010, Aret N Carlsen (aretcarlsen@autonomoustools.com).
// Fundamental MAP servers (C++).
// Licensed under GPLv3 and later versions. See license.txt or <http://www.gnu.org/licenses/>.


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

  DEBUGprint_SS("SS:pR: try alloc pack\n");

  // Attempt to allocate reply packet.
  if(! allocateNewPacket(&replyPacket, InitialReplyPacketCapacity + data_capacity))
    return false;

  // Step through headers. Stop at actual data (after last MAP headers), when get_next_header returns NULL.
  for(MAP::Data_t* srcHeader = srcPacket->get_first_header(); srcHeader != NULL; srcHeader = srcPacket->get_next_header(srcHeader)){
  // If header indicates a source address is present, sink it to the reply packet as a dest address.
    if(MAP::get_srcAddressPresent(*srcHeader)){
      // Assume copy will fail until proven otherwise.
      successful_preparation = false;

      DEBUGprint_SS("SS:pR: cp src->dest\n");

      // This implementation only supports addy types 0 to 14 (not Expanded).
      if(MAP::get_addressType(*srcHeader) > 14)
        break;

      // Sink a destAddressPresent header to the replyPacket.
      if(! replyPacket->sinkExpand(MAP::DestAddressPresent_Mask | MAP::get_addressType(*srcHeader), IncrementReplyPacketCapacity))
        break;

//      DEBUGprint_SS("prepareReply: Header byte sunk.\n");

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
        DEBUGprint_SS("SS:pR: cmplt cp src->dest\n");
      }else{
        DEBUGprint_SS("SS:pR: failed cp src->dest\n");
        break;
      }
    }
  }

// Make sure sufficient data capacity is left available.
  if(successful_preparation && (replyPacket->get_availableCapacity() < data_capacity)){
    DEBUGprint_SS("SS:pR: atmpting expnsn\n");
  // Abort if cannot leave sufficient data capacity available.
    if(! replyPacket->set_availableCapacity(data_capacity))
      successful_preparation = false;
  }

  // Problems, or no src addresses found? Abort.
  if(! successful_preparation){
    DEBUGprint_SS("SS:pR: prep failed\n");
    delete replyPacket;
    return false;
  }

// Success!
  DEBUGprint_SS("SS:pR: prep succeed\n");
  *replyPacket_ptr_ptr = replyPacket;
  return true;
}

// Prepare a boolean reply packet.
// Packet contains a single byte, 1 for true, 0 for false.
bool SimpleServer::replyBoolean(bool value){
  // Attempt to prepare reply packet and sink the single content byte.
  MAP::MAPPacket *replyPacket;
  if(! SimpleServer::prepareReply(&replyPacket, offsetPacket.packet, sizeof(bool))) return false;
  if(! replyPacket->sinkBool(value)){ MAP::dereferencePacket(replyPacket); return false; }
  // Reference and transmit the packet.
  return sendPacket(replyPacket);
}

// Prepare a Code78 reply packet.
// Packet contains a variable number of bytes, based on the numeric value given.
bool SimpleServer::replyC78(uint32_t value){
  // Attempt to prepare reply packet (if memory available etc).
  MAP::MAPPacket *replyPacket;
  // Educated guess as to C78 size.
  if(! SimpleServer::prepareReply(&replyPacket, offsetPacket.packet, sizeof(uint32_t))) return false;
  if(! replyPacket->sinkC78(value)){ MAP::dereferencePacket(replyPacket); return false; }
  // Reference and transmit packet.
  return sendPacket(replyPacket);
}

bool SimpleServer::replyC78String(uint8_t* const buf, uint16_t buf_len){
  // Attempt to prepare reply packet (if memory available etc).
  MAP::MAPPacket *replyPacket;

  // Sink string length in C78. Educated guess as to C78 size.
  if(! SimpleServer::prepareReply(&replyPacket, offsetPacket.packet, 1 + buf_len)) return false;
  if(! replyPacket->sinkC78(buf_len)){ MAP::dereferencePacket(replyPacket); return false; }

  // Sink string itself
  for(; buf_len > 0; buf_len--){
    if(! replyPacket->sinkData(*buf)){ MAP::dereferencePacket(replyPacket); return false; }
  }

  // Reference and transmit packet.
  return sendPacket(replyPacket);
}

