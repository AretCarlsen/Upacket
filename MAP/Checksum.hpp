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

#include "../DataTransfer/DataSource.hpp"
#include "../SEP/SEP.hpp"
#include "../SEP/Packet.hpp"
#include "../StateMachine/StateMachine.hpp"
#include "MAP.hpp"
#include "../PosixCRC32Checksum/PosixCRC32Checksum.hpp"

#ifndef NULL
#define NULL 0
#endif

namespace MAP {

// Checksum type
typedef uint32_t Checksum_t;
// Checksum length (32 bits)
static const uint8_t ChecksumLength = 4;
// Initial checksum value (inverted 0)
Checksum_t ChecksumInitialValue = 0xFFFFFFFF;
// Checksum table is indexable by any single byte. Its size, therefore, is 2^8.
static const uint32_t ChecksumTable[256];

// Checksum generation engine.
// Currently a simple one-liner, as is based on 1kB source table.
Checksum_t checksumEngine(Checksum_t old_value, uint8_t new_data){
  return PosixCRC32ChecksumTable[(uint8_t) old_value ^ new_data] ^ (old_value >> 8);
}

// Returns true if the packet already contained a checksum or a checksum has been successfully appended.
// False otherwise (e.g. if unable to append sufficient memory).
bool appendChecksum(MAP::Packet *packet){
  // Check is packet already contains checksum
  if(packet->getChecksumPresent())
    return true;

  // Make sure packet has sufficient buffer capacity to store the additional 4 CRC bytes.
  if(packet->capacity() - packet->length() < ChecksumLength){
    // Attempt to increase capacity
    if(! packet->setCapacity(packet->capacity() + ChecksumLength))
      return false;
  }

  // Set checksum presence bit
  packet->setChecksumPresent(true);

  // Generate checksum
  Checksum_t checksum = ChecksumInitialValue;
  for(uint8_t i = packet->length(); i > 0; i--){
  // Actual checksum calculation, relying on 1kb table
    checksum = checksumEngine(checksum, *packetData);
    packetData++;
  }

  // Append checksum
  for(uint8_t i = 4; i > 0; i--){
   // Append byte
    packet.append(checksum & 0xFF);
   // Right-shift checksum by one byte
    checksum >> 8;
  }

  return true;
}

// Validate a MAP packet's checksum.
//
// If packet does not include a checksum or the checksum does not match the packet's data,
// returns false. If the converse, returns true.
bool validateChecksum(MAP::Packet *packet){
// Make sure packet includes a checksum
  if(!(
       packet->getChecksumPresent()
    && (packet->length() > ChecksumLength + 1)
  ))
    return false;

  // Generate checksum
  Checksum_t checksum = ChecksumInitialValue;
  MAP::Data_t *packetData = packet->dataStart();
  for(uint8_t i = packet->length() - 4; i > 0; i--){
  // Actual checksum calculation, relying on 1kb table
    checksum = checksumEngine(checksum, *packetData);
    packetData++;
  }
  // Note that packetData is left pointing at the first checksum byte.

  // Validate checksum
  for(uint8_t i = ChecksumLength; i > 0; i--){
  // Validate 1/4 of checksum.
    // This may not be the most efficient way to extract the LSB from the csum.
    // A cast to uint8_t could be better, for instance.
    if(*packetData != (checksum & 0xFF))
      return false;

    packetData++;
  }

  // Checksum was valid.
  return true;
}


/*
// Checksums are always generated where the byte is treated as containing the CSumPresent bit.
uint32_t generateChecksum(MAP::Packet *packet){
  // Initialize checksum. (Don't bother storing 4-byte initial value? Which is 0xFFFFFFFF.)
  Checksum_t checksum = ChecksumInitialValue;

  // Feed the packet contents into the checksum engine
  uint8_t *packetData = packet->getStartPointer();
  uint8_t packetLength = packet->length();

// Checksum is over the packet data only
  if(packet->headerByte() & MAP::ChecksumPresentMask)
    packetLength -= ChecksumLength;
  else
    headerByte =

  for(uint8_t i = packet->length(); i > 0; i--){
  // Actual checksum calculation, relying on 1kb table
    checksum = PosixCRC32ChecksumTable[(uint8_t) checksum ^ *packetData] ^ (checksum >> 8);
    packetData++;
  }

  // Invert
  return ~checksum;
}
*/


// End namespace: MAP
}

