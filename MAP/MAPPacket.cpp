
#include "MAP.hpp"
#include "../PosixCRC32ChecksumEngine/PosixCRC32ChecksumEngine.hpp"


// Append a data byte to a packet, expanding the packet's capacity if necessary.
// Will not expand beyond capacity_limit.
bool MAP::MAPPacket::sinkExpand(Data_t data, uint8_t capacity_increment, uint8_t capacity_limit){
// Attempt to sink
  if(sinkData(data) != Status::Status__Good){
  // Attempt to expand and sink again, or fail. (Don't expand beyond capacity_limit.)
    if((get_capacity() + capacity_increment > capacity_limit)
       || !(
               set_capacity(get_capacity() + capacity_increment)
            && (sinkData(data) == Status::Status__Good)
       )
    )
      return false;
  }

  return true;
}

// Validate a MAP packet.
//
// If the require_checksum argument is true, then the packet must contain a (valid) checksum
// in the outermost encapsulation to be considered valid.
// If the remove_checksums argument is true, then any checksums present will be removed
// once validated. (Note that the process aborts after encountering the first packet error.)
bool MAP::MAPPacket::validate(bool require_checksum, bool remove_checksums){
// Make sure packet includes at least an initial header
  if(get_size() == 0)
    return false;

// Make sure packet includes an outermost checksum, if requested.
  if(require_checksum && !( get_checksumPresent(*(get_first_header())) ))
    return false;

  DEBUGprint("validate: Checksum present. packet capacity = %u. packet size = %u\n", get_capacity(), get_size());

// Validate (possibly nested) MAP header structure.
  // This requires a pass through the entire header structure.
  // Some processing time could perhaps be saved by using the same
  // pass to feed all the headers through the checksum engine,
  // especially if the packet data is stored in something slower
  // than RAM.
  Data_t* data_ptr = get_data();
  if(data_ptr == NULL)
    return false;

  // Initial header and stop_ptr.
  Data_t *header = get_first_header();
  Data_t *stop_ptr = back();

  // Stop at actual data (no more MAP headers).
  while(header != NULL){
  // If header indicates a checksum, validate it.
    if(get_checksumPresent(*header)){
    // Validate the checksum
      if(! validateChecksum(header, stop_ptr))
        return false;

    // Encapsulated data is now one checksum back.
      stop_ptr -= MAP::ChecksumLength;
    }

  // Next header
    header = get_next_header(header);
  }

// If requested, cut off all checksums.
  if(remove_checksums){
    // Set the packet size such that the checksums (at the end) are all removed.
    // (Because get_first_header() is equal to front()).
    set_size(stop_ptr - get_first_header());
  }

  // Checkums (if any) were all valid.
  return true;
}

// Validate a checksum from the header at data_ptr to the end of the checksum
// just before stop_ptr. (Obviously the checksum will not be valid if there are
// less than four bytes between data_ptr (the header) and stop_ptr.)
bool MAP::MAPPacket::validateChecksum(Data_t *data_ptr, Data_t *stop_ptr){
  // Checksum calculation engine.
  PosixCRC32ChecksumEngine checksumEngine;

    // Make sure that there is at least room for the checksum to exist.
    // That is, four bytes beginning at the next byte after the header.
    // stop_ptr points at the byte AFTER that byte.
    // For instance, if data_ptr (initially the header) has a value of 9,
    // then the first data byte is at 10, and stop_ptr (which points
    // to the byte just AFTER the last byte in the packet) needs to be at
    // 14 to leave room for four checksum bytes (10, 11, 12, 13).
  // Note that this is another instance of the "what if back() happens to be 0 due to overflow" problem.
  if(stop_ptr - (data_ptr + 1) < MAP::ChecksumLength)
    return false;

  // Stop at end of data (before checksum value).
  stop_ptr -= MAP::ChecksumLength;

  // Run the actual headers and data through the checksum engine.
  for(; data_ptr < stop_ptr; data_ptr++)
    checksumEngine.sinkData(*data_ptr);

  // Return stop_ptr to just after the end of the checksum value.
  stop_ptr += MAP::ChecksumLength;
  // Retrieve the calculated checksum.
  MAP::Checksum_t checksum = checksumEngine.getChecksum();

  // Continue at data_ptr's current position -- just after the end of the data, therefore
  // at the beginning of the checksum value -- and validate checksum.
  for(; data_ptr < stop_ptr; data_ptr++){
  // Validate one byte of checksum.
    // This may not be the most efficient way to extract the LSB from the csum.
    // A cast to uint8_t may be better, for instance. Not sure.
    if(*data_ptr != (checksum & 0xFF)){
DEBUGprint("Checksum byte invalid. Expected %X, received %X.\n", checksum & 0xFF, *data_ptr);
      return false;
    }

  // Next checksum byte.
    checksum >>= 8;
  }

  // Checksum was valid.
  return true;
}

// Append an outer checksum to the packet, if not already present.
// Note: Does NOT validate the existing checksum, if any.
//
// Eventually, this should be broken out into a function that takes a stop_ptr
// to allow callers to append checksums to sub-packets. Would require a little
// footwork to move data after the stop_ptr out to provide for the CRC bytes.
bool MAP::MAPPacket::appendChecksum(){
DEBUGprint("appendChecksum: Start\n");

// Packet must have at least one byte.
  assert(get_first_header() != NULL);

  // Check if packet already contains checksum
  if(MAP::get_checksumPresent(*(get_first_header())))
    return true;

DEBUGprint("appendChecksum: Checksum not already present.\n");
DEBUGprint("appendChecksum: packet capacity = %u.\n", get_capacity());
DEBUGprint("appendChecksum: packet size = %u.\n", get_size());

  // Make sure packet has sufficient buffer capacity to store the additional 4 CRC bytes.
  if(get_capacity() - get_size() < ChecksumLength){
    // Attempt to increase capacity
    if(! set_capacity(get_capacity() + ChecksumLength)){
DEBUGprint("appendChecksum: Unable to expand.\n");
      return false;
    }
  }

  // Set checksum presence bit
  Data_t *header = get_first_header();
  *header = MAP::set_checksumPresent(*header, true);

  // Checksum calculation engine.
  PosixCRC32ChecksumEngine checksumEngine;

  // Calculate checksum
  for(MAP::Data_t *data_ptr = get_first_header(); data_ptr < back(); data_ptr++)
    checksumEngine.sinkData(*data_ptr);

  Checksum_t checksum = checksumEngine.getChecksum();
  // Append checksum
  for(uint8_t i = 4; i > 0; i--){
   // Append byte
    append(checksum & 0xFF);
  // Next checksum byte.
    checksum = checksum >> 8;
  }

DEBUGprint("appendChecksum: Successfully appended. New packet size = %u.\n", get_size());
  return true;
}


/* UNTESTED
// Remove all checksums in a packet.
void MAPPacket::removeChecksums(){
// Sum up the CRC count to reduce the size at the end, in one fell swoop.
// (In case set_size calls realloc or is otherwise expensive.)
  uint8_t checksum_count = 0;

// Start at first header.
  Data_t *header = get_first_header();
  while(header != NULL){
  // Check for checksum.
    if(get_checksumPresent(*header)){
      checksum_count++;
  // Unset checksum present bit in header.
      set_checksumPresent(header, NULL);
    }
  // Next header.
    header = get_next_header(*header);
  }

  set_size(get_size() - checksum_count * MAP::ChecksumLength);
}
*/

/* UNTESTED
// Calculates how many bytes of checksums are appended to the end of a packet.
// Returns uint8_t, so is only valid for up to 64 32-bit checksums.
uint8_t MAPPacket::calculateChecksumSize(){
// Running checksum count.
  uint8_t checksum_count = 0;

// Step through headers.
  Data_t *header = get_first_header();
  while(header != NULL){
    if(get_checksumPresent(*header))
      checksum_count++;
    header = get_next_header(*header);
  }

  return checksum_count * MAP::ChecksumLength;
}
*/


