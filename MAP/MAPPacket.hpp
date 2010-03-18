#include "MAP.hpp"

// A MAP packet.
// The first byte of the packet contents are taken as a MAP header.
// Depending on the header byte, the following bytes may be dest and/or src addresses,
// and the packet contents may be suffixed with a CRC32 checksum.
class MAPPacket : public Bpacket {

  MAP::Data_t headerByte(){
    return read(0);
  }
};

typedef BpacketSink MAPPacketSink;

