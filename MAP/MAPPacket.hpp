
namespace MAP {

// A MAP packet.
// The first byte of the packet contents are taken as a MAP header.
// Depending on the header byte, the following bytes may be dest and/or src addresses,
// and the packet contents may be suffixed with a CRC32 checksum.
//class MAPPacket : public Packet::Bpacket {
// A packet of buffered (randomly accessible) data and an associated status.
// Limited to a 2^16-1 byte count.
#define PACKET_CAPACITY_T uint16_t
class MAPPacket : public DataStore::DynamicArrayBuffer<Data_t, PACKET_CAPACITY_T> {
  // Current status
  Status::Status_t status;

  // Reference count (for garbage collection)
  uint8_t referenceCount;

public:
  typedef PACKET_CAPACITY_T Capacity_t;

  typedef uint8_t Offset_t;

private:
  Capacity_t dataOffset;

public:

  MAPPacket()
  : status(Status::Status__Complete),
    referenceCount(0),
    dataOffset(0)
  { }

// Set the current packet status
  void sinkStatus(const Status::Status_t &new_status){
    status = new_status;
  }

  static const Capacity_t DefaultCapacityLimit = 50;

// Append a data byte to a packet, expanding the packet's capacity if necessary.
  inline bool sinkExpand(Data_t data, const uint8_t capacity_increment = 1, const uint8_t capacity_limit = DefaultCapacityLimit){
    return DataStore::DynamicArrayBuffer<Data_t,Capacity_t>::sinkExpand(data, capacity_increment, capacity_limit);
  }

  inline uint8_t incrementReferenceCount(){
    return ++referenceCount;
  }
  inline uint8_t decrementReferenceCount(){
    if(referenceCount == 0)
      return 0;
    else
      return --referenceCount;
  }

  inline Data_t* get_first_header() const{
    return front();
  }

  inline Data_t* get_header(Offset_t headerOffset){
    Data_t *header_ptr = get_first_header();
    for(; headerOffset > 0; headerOffset--)
      header_ptr = get_next_header(header_ptr);
    return header_ptr;
  }

/*
// Get the data area size
  Capacity_t get_data_size(){
    return back() - get_data();
  }
*/

// Bypass a C78 field
  Data_t* c78Pass(Data_t* data_ptr) const{
  // Sanity checks
    if(data_ptr == NULL || data_ptr >= back())
      return NULL;

//    DEBUGprint("c78pass: input data_ptr = %X, front() = %X, back() = %X\n", (unsigned int) data_ptr, (unsigned int) front(), (unsigned int) back());
  // Step through bytes until hit end of C78-encoded field
  // or end of packet.
    for(; data_ptr < back() && (! Code78::isLastByte(*data_ptr)); data_ptr++);

  // Pointer is at last C78 byte, need to advance one more.
    data_ptr++;
//    DEBUGprint("c78pass: output data_ptr = %X\n", (uint32_t) data_ptr);

  // Sanity check
    if(data_ptr >= back())
      return NULL;

    return data_ptr;
  }

// Bypass the next-proto field, if present.
// Returns NULL if it cannot return a valid address.
  Data_t* bypass_nextProto(const Data_t* const header, Data_t* data_ptr) const{
    // Step past src address, if present.
    if(MAP::get_nextProtoPresent(*header))
      return c78Pass(data_ptr);
    else
      return (data_ptr > back())? NULL : data_ptr;
  }
// Bypass the dest address field, if present.
// Returns NULL if it cannot return a valid address.
  Data_t* bypass_destAddress(const Data_t* const header, Data_t* data_ptr) const{
    // Step past src address, if present.
    if(MAP::get_destAddressPresent(*header))
      return c78Pass(data_ptr);
    else
      return (data_ptr > back())? NULL : data_ptr;
  }
// Bypass the src address field, if present.
// Returns NULL if it cannot return a valid address.
  Data_t* bypass_srcAddress(const Data_t* const header, Data_t* data_ptr) const{
    // Step past src address, if present.
    if(MAP::get_srcAddressPresent(*header))
      return c78Pass(data_ptr);
    else
      return (data_ptr > back())? NULL : data_ptr;
  }

// Get a pointer to the next-proto field.
  Data_t* get_nextProto(Data_t* const header) const{
  // Sanity check
    if(! MAP::get_nextProtoPresent(*header))
      return NULL;

  // Check if past end-of-packet.
    return (header + 1 > back())? NULL : header + 1;
  }
// Get a pointer to the dest-address field.
  Data_t* get_destAddress(Data_t* const header) const{
  // Sanity check
    if(! MAP::get_destAddressPresent(*header))
      return NULL;

  // Beginning at next-proto, bypass next-proto.
    return bypass_nextProto(header, header + 1);
  }
// Get a pointer to the src-address field.
  Data_t* get_srcAddress(Data_t* const header) const{
  // Sanity check
    if(! MAP::get_srcAddressPresent(*header))
      return NULL;

  // Beginning at next-proto, bypass next-proto. Then bypass dest-address.
    return bypass_destAddress(header, bypass_nextProto(header, header + 1));
  }
// Get a pointer to the packet contents (everything after headers).
  Data_t* get_contents(Data_t* const header) const{
  // Begin at first byte after header (header + 1). Bypass next-proto, dest-address, and src-address.
    return bypass_srcAddress( header, bypass_destAddress(header, bypass_nextProto(header, header + 1)) );
  }
// Get the next header, if any.
// Returns NULL if the next-proto is not MAP.
  Data_t* get_next_header(Data_t* const header){
  // Make sure next-proto is MAP.
    Data_t* data_ptr = get_nextProto(header);
    if(data_ptr == NULL || *data_ptr != MAP::Protocol__MAP)
      return NULL;

  // Return pointer to first byte of packet contents,
  // which is the encapsulated MAP header byte.
    return get_contents(header);
  }

// Step through headers until reach a non-MAP encapsulated packet.
  Data_t* get_data(Data_t* header){
    // Search for next header (if any).
    // Stop when no further non-MAP headers accessible.
    for(Data_t* data_ptr = get_next_header(header); data_ptr != NULL; data_ptr = get_next_header(header)){
    // Next header found. Continue searching for data from the new header.
      header = data_ptr;
    }
  // The packet contents at this (non-MAP) header.
    return get_contents(header);
  }
// Get the first non-MAP packet contents.
  Data_t* get_data(Offset_t headerOffset){
    return get_data(get_header(headerOffset));
  }

// Validate the MAP packet.
//
// If the require_checksum argument is true, then the packet must contain a (valid) checksum
// in the outermost encapsulation to be considered valid.
// If the remove_checksums argument is true, then any checksums present will be removed
// once validated.
  bool validate(Offset_t headerOffset = 0, const bool require_checksum = false, const bool remove_checksums = true);

// Validate a checksum from the header at data_ptr to the end of the checksum
// just before stop_ptr.
  bool validateChecksum(const Data_t* data_ptr, const Data_t* stop_ptr);

// Append a checksum to the packet, if there is not already one present.
//
// Returns true if the packet already contained a checksum or a checksum has been successfully appended.
// False otherwise (e.g. if unable to append sufficient memory).
  bool appendChecksum(Offset_t headerOffset = 0);

// C78-sink a numeric value
  bool sinkC78(const uint32_t value, const uint8_t capacity_increment = 1, const uint8_t capacity_limit = DefaultCapacityLimit);
  bool sinkC78Signed(const int32_t value, const uint8_t capacity_increment = 1, const uint8_t capacity_limit = DefaultCapacityLimit){
    uint32_t unsigned_value;
    if(value < 0)  // Handle negative values specially.
      unsigned_value = (-(value+1) << 1) | 0x01;  // Add one to account for -0.
    else  // Positive values are just left-shifted a bit.
      unsigned_value = value << 1;  // Arithmetic bit shift s.t. the LSb is filled with a 0.

    return sinkC78(unsigned_value, capacity_increment, capacity_limit);
  }
// Source a C78-encoded big-endian numeric value.
  //  template <typename IntType_t>
  bool sourceC78(uint32_t &value, Data_t*& data_ptr);
  bool sourceC78Signed(int32_t &value, Data_t*& data_ptr){
    uint32_t unsigned_value;
    if(! sourceC78(unsigned_value, data_ptr)) return false;
    if(unsigned_value & 0x01)  // Handle negative values
      value = -(unsigned_value >> 1) - 1;   // Subtract 1 to account for -0.
    else
      value = unsigned_value >> 1;  // The MSb 
    return true;
  }
};


// End namespace: MAP
}

