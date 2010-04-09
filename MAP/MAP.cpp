// MAP definitions

#include "MAP.hpp"
#include "../PosixCRC32ChecksumEngine/PosixCRC32ChecksumEngine.hpp"

#include "MAPPacket.cpp"

// Allocate a new packet.
bool MAP::allocateNewPacket(MAPPacket** const packet, const uint16_t &capacity){
// Sanity check
  if(capacity == 0) return false;

// Attempt to allocate initial packet
  MAPPacket *newPacket = new MAPPacket;

// Allocation failed? Return false.
  if(newPacket == NULL)
    return false;

// Attempt to allocate buffer storage
  if(! newPacket->set_capacity(capacity)){
    DEBUGprint("MP:allNewP: couldn't alloc %d\n", capacity);
    delete newPacket;
    return false;
  }

  DEBUGprint("MP:allNewP: alloc'd %d\n", capacity);

// Set the new packet's header byte to all zeroes
  *(newPacket->front()) = 0;

// Save new packet.
  *packet = newPacket;

  return true;
}

