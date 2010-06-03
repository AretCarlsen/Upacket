// Copyright (C) 2010, Aret N Carlsen (aretcarlsen@autonomoustools.com).
// MAP packet handling (C++).
// Licensed under GPLv3 and later versions. See license.txt or <http://www.gnu.org/licenses/>.


#include "MAP.hpp"
#include <Upacket/PosixCRC32ChecksumEngine/PosixCRC32ChecksumEngine.hpp>

// Sink a numeric value in C78-encoded big-endian format.
bool MAP::MAPPacket::sinkC78(const uint32_t value, const Capacity_t capacity_increment, const Capacity_t capacity_limit){
  // Calculate extra C78 bytes required (before concluding byte, with its 0 MSb).
  // Will need an extra byte for each log (base 2^7) increment.
  uint8_t extraBytes = 0;
  for(uint32_t tmpValue = value >> 7; tmpValue > 0; tmpValue >>= 7)
    extraBytes++;

  // For each extra byte, sink the corresponding highest 7 bits.
  for(; extraBytes > 0; extraBytes--){
    if(! sinkExpand((value >> (7 * extraBytes)) | 0x80, capacity_increment, capacity_limit)) return false;
  }
  // Sink last posByte
  return sinkExpand(value & 0x7F, capacity_increment, capacity_limit);
}

// Source a C78-encoded big-endian numeric value.
  // Note that the data_ptr is left pointing at the last C78 byte, if valid.
bool MAP::MAPPacket::sourceC78(uint32_t &value, Data_t*& data_ptr){
// Sanity check on data.
  if(data_ptr == NULL || data_ptr >= back())
    return false;

// Initial value for numerics.
  value = 0;
  while(data_ptr < back()){
    value |= *data_ptr & 0x7F;
  // Check if C78 value has concluded.
    if(!(*data_ptr & 0x80))
      break;

  // Left-shift in preparation for next byte.
    value <<= 7;
  // Next byte.
    data_ptr++;
  }

// Check whether C78 value concluded properly.
  return (data_ptr < back());
}

// Source a C78String (C78 pascal encoding).
// If the C78 prefix is invalid or the value is greater than max_len, read_len will be set to 0 and false returned.
// If the C78 prefix is 0, read_len will be set to 0 and true returned.
// If the C78 prefix is valid, and the complete string is sourced, read_len will be equal to the string length and true returned.
// If the C78 prefix is valid, but the complete string is not present (because the packet is not long enough), read_len will be
// set to the string portion that was available and false returned.
bool MAP::MAPPacket::sourceC78String(Data_t *strBuf, Capacity_t &read_len, Capacity_t max_len, Data_t*& data_ptr){
  read_len = 0;
  uint32_t strLen;
// Source C78 (and sanity checks)
  if(! (sourceC78(strLen, data_ptr) && (strLen <= max_len))) return false;
  data_ptr++;

  Data_t *packetBack = back();
// Source string itself
  for(; strLen > 0; strLen--){
    if(data_ptr > packetBack) return false;
    *strBuf = *data_ptr;
    strBuf++; data_ptr++;
    read_len++;
  }
  data_ptr--;

  return true;
}

// Validate a MAP packet.
//
// If the require_checksum argument is true, then the packet must contain a (valid) checksum
// in the outermost encapsulation to be considered valid.
// If the remove_checksums argument is true, then any checksums present will be removed
// once validated. (Note that the process aborts after encountering the first packet error.)
bool MAP::MAPPacket::validate(HeaderOffset_t headerOffset, bool require_checksum, bool remove_checksums){
// Make sure packet includes at least an initial header
// Make sure packet includes an outermost checksum, if requested.
  Data_t* header = get_header(headerOffset);
  if(   header == NULL
     || ( require_checksum && !get_checksumPresent(*header) )
    ) return false;

  DEBUGprint_MAP("MPPval: cap=%d, size=%d\n", get_capacity(), get_size());

// Validate (possibly nested) MAP header structure.
  // This requires a pass through the entire header structure.
  // Some processing time could perhaps be saved by using the same
  // pass to feed all the headers through the checksum engine,
  // especially if the packet data is stored in something slower
  // than RAM.
  Data_t* data_ptr = get_data(header);
  if(data_ptr == NULL)
    return false;

  // Stop point
  Data_t *stop_ptr = back();

  // Stop at actual data (no more MAP headers).
  while(header != NULL){
  // If header indicates a checksum, validate it.
    if(get_checksumPresent(*header)){
       DEBUGprint_MAP("MPPval: val crc\n");
    // Validate the checksum
      if(! validateChecksum(header, stop_ptr))
        return false;

    // Encapsulated data is now one checksum back.
      stop_ptr -= MAP::ChecksumLength;

      if(remove_checksums){
        *header = MAP::set_checksumPresent(*header, false);
        DEBUGprint_MAP("MPPval: rem crc\n");
      }
    }

  // Next header
    header = get_next_header(header);
  }

// If requested, cut off all checksums.
  if(remove_checksums){
    // Set the packet size such that the checksums (at the end) are all removed.
    set_size(stop_ptr - front());
    // Try to eliminate excess capacity
    //set_capacity(stop_ptr - front());
  }

  // Checksums (if any) were all valid.
  return true;
}

// Validate a checksum from the header at data_ptr to the end of the checksum
// just before stop_ptr. (Obviously the checksum will not be valid if there are
// less than four bytes between data_ptr (the header) and stop_ptr.)
bool MAP::MAPPacket::validateChecksum(const Data_t* data_ptr, const Data_t* stop_ptr){
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
      DEBUGprint_MAP("MPPvalCrc: CRC byte invalid. Exp %x, rcvd %x.\n", checksum & 0xFF, *data_ptr);
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
bool MAP::MAPPacket::appendChecksum(HeaderOffset_t headerOffset){
  DEBUGprint_MAP("apCrc: St\n");

// Packet must have at least one byte.
  if(is_empty()) return false;


  // Check if packet already contains checksum (or at least indicates so in header).
  Data_t *header = get_header(headerOffset);
  if(MAP::get_checksumPresent(*header)) return true;

  DEBUGprint_MAP("apCrc: Crc not pres. cap %d, size %d\n", get_capacity(), get_size());

  // Make sure packet has sufficient buffer capacity to store the additional 4 CRC bytes.
  if(get_availableCapacity() < ChecksumLength){
    // Attempt to increase capacity
    if(! set_availableCapacity(ChecksumLength)){
      DEBUGprint_MAP("apCrc: Expand failed\n");
      return false;
    }
// Pointers are now invalid
    header = get_header(headerOffset);
// Overly cautious.
    if(header == NULL) return false;
  }

  *header = MAP::set_checksumPresent(*header, true);

  // Checksum calculation engine.
  PosixCRC32ChecksumEngine checksumEngine;

  // Calculate checksum
  for(MAP::Data_t *data_ptr = header; data_ptr < back(); data_ptr++)
    checksumEngine.sinkData(*data_ptr);

  Checksum_t checksum = checksumEngine.getChecksum();
  // Append checksum
  for(uint8_t i = 4; i > 0; i--){
   // Append byte. Could use append() here, since already verified capacity, but being cautious.
    sinkData(checksum & 0xFF);
  // Next checksum byte.
    checksum = checksum >> 8;
  }

  DEBUGprint_MAP("apCrc: App cmplt. New size: %d\n", get_size());
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


