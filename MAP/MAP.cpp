// MAP definitions

#include "MAP.hpp"
#include "../PosixCRC32ChecksumEngine/PosixCRC32ChecksumEngine.hpp"

#include "MAPPacket.cpp"

// Allocate a new packet.
bool MAP::allocateNewPacket(MAPPacket** const packet, const uint16_t &capacity){  // , MemoryPool *pool){
// Sanity check
  if(capacity == 0) return false;

// Attempt to allocate initial packet
//  if(! pool->allocate(sizeof(MAPPacket))) return false;

  MAPPacket *newPacket = new MAPPacket;

// Allocation failed? Return false.
  if(newPacket == NULL){
//    pool->deallocate(sizeof(MAPPacket));
    return false;
  }

// Attempt to allocate buffer storage
  if(! newPacket->set_capacity(capacity)){
    DEBUGprint_MAP("MP:allNewP: couldn't alloc %d\n", capacity);
    delete newPacket;
    return false;
  }

  DEBUGprint_MAP("MP:allNewP: alloc'd %d\n", capacity);

// Set the new packet's header byte to all zeroes
  *(newPacket->front()) = 0;

// Save new packet.
  *packet = newPacket;

  return true;
}

