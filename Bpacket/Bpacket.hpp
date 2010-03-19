
#pragma once

#include "../../DataStore/DynamicArray.hpp"
// Status codes
#include "../../Status/Status.hpp"

namespace Packet {

typedef uint8_t Data_t;

// A packet of buffered (randomly accessible) data and an associated status.
// Limited to a 2^16-1 byte count.
#define BPACKET_CAPACITY_T uint16_t
class Bpacket : public DataStore::DynamicArray<Data_t, BPACKET_CAPACITY_T> {
  // Current status
  Status::Status_t status;

public:
  typedef BPACKET_CAPACITY_T Capacity_t;

  virtual Status::Status_t sinkStatus(Status::Status_t new_status){
    status = new_status;
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

// End namespace: SEP
}

