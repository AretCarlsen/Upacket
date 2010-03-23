//
// Micro Address Protocol common definitions
//

#pragma once

#include "../Bpacket/Bpacket.hpp"
#include "../Code78/Code78.hpp"

namespace MAP {
// Data type
  typedef uint8_t Data_t;

// Bit positions and masks for the MAP header
  typedef uint8_t MaskType_t;
  static const MaskType_t ChecksumPresent_Mask = 0x80;
  static const MaskType_t NextProtoPresent_Mask = 0x40;
  static const MaskType_t DestAddressPresent_Mask = 0x20;
  static const MaskType_t SrcAddressPresent_Mask = 0x10;
// Lower nibble is Address Type
  static const MaskType_t AddressType_Mask = 0x0F;

// Checksum length (32 bits)
  static const uint8_t ChecksumLength = 4;

  typedef uint8_t Protocol_t;
// No next proto. (Raw data enclosed, or perhaps the port is determined by the receiver
// on the basis of the src and/or dest addresses.)
  static const Protocol_t Protocol__none   = 0;
// Micro Addressed Protocol
  static const Protocol_t Protocol__MAP    = 1;
// Internet Control Message Protocol
  static const Protocol_t Protocol__ICMP   = 4;
// Media Access Control
  static const Protocol_t Protocol__MAC    = 5;
// Internet Protocol, v4
  static const Protocol_t Protocol__IPv4   = 6;
// User Datagram Protocol
  static const Protocol_t Protocol__UDP    = 7;
// Transmission Control Protocol
  static const Protocol_t Protocol__TCP    = 8;
// Address Resolution Protocol
  static const Protocol_t Protocol__ARP    = 9;
// Domain Name Service
  static const Protocol_t Protocol__DNS    = 10;
// Protocols above 63 are given in C78-encoded format
// beginning with the byte immediately following the header byte.
  static const Protocol_t Protocol__extended = 63;

  typedef uint8_t AddressType_t;
// (none)
  static const AddressType_t AddressType__none = 0;
// Hardware bus-local addressing. Not intended for relay beyond the target hardware bus.
  static const AddressType_t AddressType__HardwareLocal = 1;
// Device-local addressing. Not intended for relay beyond the target device.
// Also known as "ports".
  // Static addresses are typically used for incoming connections.
  // A specific static address can be requested by a process.
  static const AddressType_t AddressType__DeviceLocalStatic = 2;
  // Dynamic addresses are allocated to processes as needed.
  // Processes are not expected to request a specific dynamic address.
  static const AddressType_t AddressType__DeviceLocalDynamic = 3;
// LAN-local addressing. Not intended for relay beyond the local LAN.
  static const AddressType_t AddressType__LANLocal = 4;
// MAC addresses, a la ethernet. Globally unique to a machine, but lengthy.
  static const AddressType_t AddressType__MAC = 5;
// IP addresses, whether v4 or v6. Sometimes globally unique to a machine.
  static const AddressType_t AddressType__IP = 6;
// DNS addresses. Lengthy, globally unique textual address; ascii text in a
// dot-delimited little-endian hierarchy.
  static const AddressType_t AddressType__DNS = 7;
// Address types above 14 are given in C78-encoded format
// beginning with the byte immediately following the header byte.
  static const AddressType_t AddressType__extended = 15;

// Set a set of header bits to true/false.
  inline void setHeaderMask(Data_t &header, uint8_t bit_mask, bool new_value){
    if(new_value)
      header |= bit_mask;
    else
      header &= ~bit_mask;
  }

  inline bool get_nextProtoPresent(Data_t header){
    return (header & NextProtoPresent_Mask);
  }
  inline Data_t set_nextProtoPresent(Data_t header, bool new_value){
    setHeaderMask(header, NextProtoPresent_Mask, new_value);
    return header;
  }

  inline bool get_checksumPresent(Data_t header){
    return (header & ChecksumPresent_Mask);
  }
  inline Data_t set_checksumPresent(Data_t header, bool new_value){
    setHeaderMask(header, ChecksumPresent_Mask, new_value);
    return header;
  }

  inline bool get_srcAddressPresent(Data_t header){
    return (header & SrcAddressPresent_Mask);
  }
  inline Data_t set_srcAddressPresent(Data_t header, bool new_value){
    setHeaderMask(header, SrcAddressPresent_Mask, new_value);
    return header;
  }

  inline bool get_destAddressPresent(Data_t header){
    return (header & DestAddressPresent_Mask);
  }
  inline Data_t set_destAddressPresent(Data_t header, bool new_value){
    setHeaderMask(header, DestAddressPresent_Mask, new_value);
    return header;
  }

  inline AddressType_t get_addressType(Data_t header){
    return (header & AddressType_Mask);
  }
  inline Data_t set_addressType(Data_t header, AddressType_t new_value){
    assert(new_value < 15);
  // Erase current Type
    header &= ~AddressType_Mask;
  // Set new Type
    header |= new_value;
  // Return newly Typed header
    return header;
  }

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

  void sinkStatus(Status::Status_t new_status){
    status = new_status;
  }

  uint8_t incrementReferenceCount(){
    return ++referenceCount;
  }
  uint8_t decrementReferenceCount(){
    return --referenceCount;
  }

private:
  Capacity_t dataOffset;

public:

  MAPPacket()
  : status(Status::Status__Complete),
    referenceCount(0),
    dataOffset(0)
  { }

  Data_t get_headerByte(){
    return get(0);
  }
  void set_headerByte(Data_t new_value){
    set(0, new_value);
  }

  Data_t* get_data(){
    if(dataOffset == 0)
      calcDataOffset();

    return front() + dataOffset;
  }
  Data_t get_data(Capacity_t index){
    if(dataOffset == 0)
      calcDataOffset();

    return get(dataOffset + index);
  }
  void set_data(Capacity_t index, Data_t new_value){
    if(dataOffset == 0)
      calcDataOffset();

    set(dataOffset + index, new_value);
  }
// Get the data area size
  Capacity_t get_data_size(){
    if(dataOffset == 0)
      calcDataOffset();

    Capacity_t data_size = size;

    if(data_size <= dataOffset)
      return 0;

    data_size -= dataOffset;

    if(get_checksumPresent()){
      if(data_size <= 4)
        return 0;

      data_size -= 4;
    }

DEBUGprint("Data offset: %u. Data size: %u. Checksum present: %u\n", dataOffset, data_size, get_checksumPresent());

    return data_size;
  }

  void calcDataOffset(){
    // Begin just after header byte.
    uint8_t *byte = front() + 1;

    // Step past dest address, if present.
    if(get_destAddressPresent()){
      while(! Code78::isLastByte(*byte))
        byte++;
      byte++;
    }

    // Step past src address, if present.
    if(get_srcAddressPresent()){
      while(! Code78::isLastByte(*byte))
        byte++;
      byte++;
    }

    if(get_nextProtoPresent()){
      while(! Code78::isLastByte(*byte))
        byte++;
      byte++;
    }

  // Data offset is difference between current position (at first byte of data)
  // and header byte position.
    dataOffset = byte - front();
  }

  Data_t* get_destAddress(){
    assert(get_destAddressPresent());

    if(get_size() < 2)
      return NULL;

    return front() + 1;
  }
  Data_t* get_srcAddress(){
    assert(get_destAddressPresent());

    Data_t *data_ptr = get_destAddress();
    while(! Code78::isLastByte(*data_ptr))
      data_ptr++;
    data_ptr++;
    if(data_ptr >= back())
      return NULL;

    return data_ptr;
  }

// Checksum presence
  bool get_checksumPresent(){
    return MAP::get_checksumPresent(get_headerByte());
  }
  void set_checksumPresent(bool new_value){
    set_headerByte(MAP::set_checksumPresent(get_headerByte(), new_value));
  }

// Next-protocol presence
  bool get_nextProtoPresent(){
    return MAP::get_nextProtoPresent(get_headerByte());
  }
  void set_nextProtoPresent(bool new_value){
    set_headerByte(MAP::set_nextProtoPresent(get_headerByte(), new_value));
  }

// Destination address presence
  bool get_destAddressPresent(){
    return MAP::get_destAddressPresent(get_headerByte());
  }
  void set_destAddressPresent(bool new_value){
    set_headerByte(MAP::set_destAddressPresent(get_headerByte(), new_value));
  }

// Source address presence
  bool get_srcAddressPresent(){
    return MAP::get_srcAddressPresent(get_headerByte());
  }
  void set_srcAddressPresent(bool new_value){
    set_headerByte(MAP::set_srcAddressPresent(get_headerByte(), new_value));
  }

// Address type
  AddressType_t get_addressType(){
    return MAP::get_addressType(get_headerByte());
  }
  void set_addressType(AddressType_t new_value){
    set_headerByte(MAP::set_addressType(get_headerByte(), new_value));
  }
};

// Reference a packet.
inline void referencePacket(MAPPacket *packet){
// Returns the new reference count
  packet->incrementReferenceCount();
}
// Dereference a packet, and free the packet
// if it is no longer referenced by anyone.
inline void dereferencePacket(MAPPacket *packet){
// Returns the new reference count
  if(packet->decrementReferenceCount() == 0)
    delete packet;
}

class MAPPacketSink {
public:
  virtual Status::Status_t sinkPacket(MAPPacket *packet) = 0;
};

// End namespace: MAP
};

