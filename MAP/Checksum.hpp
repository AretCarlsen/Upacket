// MAP::ChecksumGenerator
//
// Class t
// Class to act as an inline CRC32 generator.
// Rejects empty packets. Transparently passes along packets
// that already include a checksum.
// Packets that are non-empty but do not indicate a checksum
// in the header byte have their header bytes modified (to indicate
// a checksum), and a checksum is generated and appended to their
// data after they are drained.

#pragma once

#include "../globals.hpp"

#include "../../DataTransfer/DataSource.hpp"
#include "../MEP/MEP.hpp"
#include "../../StateMachine/StateMachine.hpp"
#include "MAP.hpp"

namespace MAP {

// Checksum type
typedef uint32_t Checksum_t;
// Initial checksum value (inverted 0)
static const Checksum_t ChecksumInitialValue = 0xFFFFFFFF;

// Get the ith precalculated checksum table entry.
uint32_t getChecksumTableEntry(uint8_t i);

// Checksum generation engine.
// Currently a simple one-liner, as is based on 1kB source table.
inline Checksum_t checksumEngine(Checksum_t old_value, uint8_t new_data){
  return getChecksumTableEntry((uint8_t) old_value ^ new_data) ^ (old_value >> 8);
}

// Returns true if the packet already contained a checksum or a checksum has been successfully appended.
// False otherwise (e.g. if unable to append sufficient memory).
bool appendChecksum(MAP::MAPPacket *packet);

// Validate a MAP packet's checksum.
//
// If packet does not include a checksum or the checksum does not match the packet's data,
// returns false. If the converse, returns true.
bool validateChecksum(MAP::MAPPacket *packet);


// Checksums are always generated where the byte is treated as containing the CSumPresent bit.
// uint32_t generateChecksum(MAP::Packet *packet);

class ChecksumGenerator : public MAP::MAPPacketSink {
  MAP::MAPPacketSink *nextSink;

public:

  ChecksumGenerator(MAP::MAPPacketSink *new_nextSink)
  : nextSink(new_nextSink)
  { }

  Status::Status_t sinkPacket(MAP::MAPPacket *packet){
    assert(nextSink != NULL);
    assert(packet != NULL);

// Generate checksum and append, if necessary
    if(appendChecksum(packet))
      return nextSink->sinkPacket(packet);
  // Checksum generation can fail is add'l memory cannot be allocated, for instance.
    else
      return Status::Status__Bad;
  }
};

class ChecksumValidator : public MAP::MAPPacketSink {
  MAP::MAPPacketSink *nextSink;

public:

  ChecksumValidator(MAP::MAPPacketSink *new_nextSink)
  : nextSink(new_nextSink)
  { }

  Status::Status_t sinkPacket(MAP::MAPPacket *packet){
    assert(nextSink != NULL);
    assert(packet != NULL);

  // Validate checksum
    if(validateChecksum(packet)){
      DEBUGprint("ChecksumValidator: Checksum valid.\n");

  // Remove the checksum
//      packet->set_size(packet->get_size() - 4);
//      packet->set_checksumPresent(false);

      return nextSink->sinkPacket(packet);
    }else{
      DEBUGprint("ChecksumValidator: Checksum invalid.\n");
      return Status::Status__Bad;
    }
  }
};


// End namespace: MAP
}

