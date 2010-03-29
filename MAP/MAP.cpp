// MAP definitions

#include "MAP.hpp"
#include "../PosixCRC32ChecksumEngine/PosixCRC32ChecksumEngine.hpp"

#include "MAPPacket.cpp"

// Allocate a new packet.
bool MAP::allocateNewPacket(MAPPacket **packet, uint16_t capacity){
  assert(capacity > 0);

// Attempt to allocate initial packet
  MAPPacket *newPacket = new MAPPacket;

// Allocation failed? Return false.
  if(newPacket == NULL)
    return false;

// Attempt to allocate buffer storage
  if(! newPacket->set_capacity(capacity)){
    delete newPacket;
    return false;
  }

DEBUGprint("allocateNewPacket: Allocated new packet.\n");

// Set the new packet's header byte to all zeroes
  *(newPacket->get_first_header()) = 0;

// Save new packet.
  *packet = newPacket;

  return true;
}

