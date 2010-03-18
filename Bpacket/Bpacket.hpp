
#pragma once

#include "SEP.hpp"
#include "../DynamicArray/DynamicArray.hpp"

namespace Packet {

// A packet of buffered (randomly accessible) data and an associated status.
// Limited to a 2^16-1 byte count.
class Bpacket : public DynamicArray<SEP::Data_t, uint16_t> {
// Current status
  SEP::Status_t status;

public:
  virtual SEP::Status_t sinkStatus(SEP::Status_t new_status){
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
  virtual SEP::Status_t sinkPacket(Bpacket *new_packet) = 0;
};

// End namespace: SEP
}

