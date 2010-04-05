
#pragma once

#include "PosixCRC32Checksum.hpp"

// Posix CRC32 checksum engine
class PosixCRC32ChecksumEngine { // : public DataTransfer::SinkData<>
public:
// Checksum type
  typedef uint32_t Checksum_t;

private:
  uint32_t checksum;

// Initial checksum value (inverted 0)
  static const Checksum_t ChecksumInitialValue = 0xFFFFFFFF;

public:

  PosixCRC32ChecksumEngine()
  : checksum(ChecksumInitialValue)
  { }

  void reset(){
    checksum = ChecksumInitialValue;
  }

// Checksum generation engine.
// Currently a simple one-liner, as is based on 1kB source table.
  void sinkData(const uint8_t &data){
  // The PosixCRC32Checksum::getChecksumTableEntry function is defined in a system-specific way.
  // The definition is therefore kept out of the Engine class itself.
    checksum = PosixCRC32Checksum::getChecksumTableEntry((uint8_t) checksum ^ data) ^ (checksum >> 8);
  }

  Checksum_t getChecksum() const{
// Invert before returning.
    return ~checksum;
  }
};

