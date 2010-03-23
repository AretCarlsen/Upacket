// Posix CRC32 precalculated value table

#include "Checksum.hpp"

namespace MAP {

// Validate a MAP packet's checksum.
//
// If packet does not include a checksum or the checksum does not match the packet's data,
// returns false. If the converse, returns true.
bool validateChecksum(MAP::MAPPacket *packet){
// Make sure packet includes a checksum
  if(!(
       packet->get_checksumPresent()
    && (packet->get_size() > MAP::ChecksumLength + 1)
  ))
    return false;

DEBUGprint("validateChecksum: Checksum present.\n");
DEBUGprint("validateChecksum: packet capacity = %u.\n", packet->get_capacity());
DEBUGprint("validateChecksum: packet size = %u.\n", packet->get_size());

  // Generate checksum
// Initial value (inverted)
  Checksum_t checksum = ChecksumInitialValue;
  MAP::Data_t *packetData = packet->front();
  for(uint8_t i = packet->get_size() - 4; i > 0; i--){
  // Actual checksum calculation, relying on 1kb table
    checksum = checksumEngine(checksum, *packetData);
    packetData++;
  }
// Invert checksum
  checksum = ~checksum;

  // Note that packetData is left pointing at the first checksum byte.

  // Validate checksum
  for(uint8_t i = ChecksumLength; i > 0; i--){
  // Validate 1/4 of checksum.
    // This may not be the most efficient way to extract the LSB from the csum.
    // A cast to uint8_t could be better, for instance.
    if(*packetData != (checksum & 0xFF)){
DEBUGprint("Checksum byte invalid. Expected %X, received %X.\n", checksum & 0xFF, *packetData);
      return false;
    }
    checksum = checksum >> 8;

    packetData++;
  }

  // Checksum was valid.
  return true;
}

// Returns true if the packet already contained a checksum or a checksum has been successfully appended.
// False otherwise (e.g. if unable to append sufficient memory).
bool appendChecksum(MAP::MAPPacket *packet){
DEBUGprint("appendChecksum: Start\n");

  // Check is packet already contains checksum
  if(packet->get_checksumPresent())
    return true;

DEBUGprint("appendChecksum: Checksum not already present.\n");
DEBUGprint("appendChecksum: packet capacity = %u.\n", packet->get_capacity());
DEBUGprint("appendChecksum: packet size = %u.\n", packet->get_size());

  // Make sure packet has sufficient buffer capacity to store the additional 4 CRC bytes.
  if(packet->get_capacity() - packet->get_size() < ChecksumLength){
    // Attempt to increase capacity
    if(! packet->set_capacity(packet->get_capacity() + ChecksumLength)){
DEBUGprint("appendChecksum: Unable to expand.\n");
      return false;
    }
  }

  // Set checksum presence bit
  packet->set_checksumPresent(true);

  // Generate checksum
// Initial value (inverted)
  Checksum_t checksum = ChecksumInitialValue;
  MAP::Data_t *packetData = packet->front();
  for(uint8_t i = packet->get_size(); i > 0; i--){
  // Actual checksum calculation, relying on 1kb table
    checksum = checksumEngine(checksum, *packetData);
    packetData++;
  }
// Invert after calculation
  checksum = ~checksum;

  // Append checksum
  for(uint8_t i = 4; i > 0; i--){
   // Append byte
    packet->append(checksum & 0xFF);
   // Right-shift checksum by one byte
    checksum = checksum >> 8;
  }

DEBUGprint("appendChecksum: Successfully appended. New packet size = %u.\n", packet->get_size());
  return true;
}

// End namespace: MAP
}

