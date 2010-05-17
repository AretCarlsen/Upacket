// Copyright (C) 2010, Aret N Carlsen (aretcarlsen@autonomoustools.com).
// MAP packet handling (C++).
// Licensed under GPLv3 and later versions. See license.txt or <http://www.gnu.org/licenses/>.

#include "MAP.hpp"
#include "../PosixCRC32ChecksumEngine/PosixCRC32ChecksumEngine.hpp"
#include "MAPPacket.cpp"

// Allocate a new packet.
bool MAP::allocateNewPacket(MAPPacket** const packet, const uint16_t &capacity, MemoryPool* const memoryPool){
// Sanity checks
  if((capacity == 0) || (memoryPool == NULL)){
    DEBUGprint_MAP("MP:allNewP: Sanity chk fld, cap=%d\n", capacity);
    return false;
  }

// Attempt to allocate initial packet (via memory pool)
// HEAP
  void* new_mem = memoryPool->malloc(sizeof(MAPPacket));
  MAPPacket *newPacket = new(new_mem) MAPPacket(memoryPool);

// Allocation failed? Return false.
  if(newPacket == NULL){
    DEBUGprint_MAP("MP:allNewP: couldn't alloc pack\n");
    return false;
  }

// Attempt to allocate buffer storage
  if(! newPacket->set_capacity(capacity)){
    DEBUGprint_MAP("MP:allNewP: couldn't alloc %d\n", capacity);

  // Delete and deallocate
    delete newPacket;
    memoryPool->deallocate(sizeof(MAPPacket));

    return false;
  }

  DEBUGprint_MAP("MP:allNewP: alloc'd %d\n", capacity);

// Set the new packet's header byte to all zeroes
  *(newPacket->front()) = 0;

// Save new packet.
  *packet = newPacket;

  return true;
}

