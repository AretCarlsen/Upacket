
#pragma once

// Dynamic arrays
#include "../../DataStore/Buffer.hpp"
// Status codes
#include "../../Status/Status.hpp"

namespace Packet {

typedef uint8_t Data_t;

// A packet of buffered (randomly accessible) data and an associated status.
// Limited to a 2^16-1 byte count.
#define BPACKET_CAPACITY_T uint16_t
class Bpacket : public DataStore::DynamicArrayBuffer<Data_t, BPACKET_CAPACITY_T> {
  // Current status
  Status::Status_t status;

  // Reference count (for garbage collection)
  uint8_t referenceCount;

public:
  Bpacket()
  : status(Status::Status__Complete),
    referenceCount(0)
  { }

  typedef BPACKET_CAPACITY_T Capacity_t;

  virtual Status::Status_t sinkStatus(Status::Status_t new_status){
    status = new_status;
  }

  uint8_t incrementReferenceCount(){
    return ++referenceCount;
  }
  uint8_t decrementReferenceCount(){
    return --referenceCount;
  }
};

// Bpacket sink.
// 
class BpacketSink {
public:
// sinkPacket return values:
//   Good indicates accepted;
//   Bad indicates rejected;
//   Busy indicates busy (not accepted).
  virtual Status::Status_t sinkPacket(Bpacket *new_packet) = 0;
};

// Reference a packet.
inline void referencePacket(Bpacket *packet){
// Returns the new reference count
  packet->incrementReferenceCount();
}
// Dereference a packet, and free the packet
// if it is no longer referenced by anyone.
inline void dereferencePacket(Bpacket *packet){
// Returns the new reference count
  if(packet->decrementReferenceCount() == 0)
    delete packet;
}

// End namespace: MEP
}

