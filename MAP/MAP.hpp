// Micro Address Protocol declarations and definitions
//
// Includes protocol-related definitions, header byte manipulation functions,
// MAPPacket class, MAPPacketSink class
//

#pragma once

#include "../Bpacket/Bpacket.hpp"
#include "../Code78/Code78.hpp"
#include "../../Process/Process.hpp"
#include "../../DataStore/RingBuffer.hpp"

namespace MAP {
// Data type
  typedef uint8_t Data_t;

/* PROTOCOL DEFINITIONS */

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
// Topics. (Aka multicast.)
  static const AddressType_t AddressType__Topic = 8;
// Address types above 14 are given in C78-encoded format
// beginning with the byte immediately following the header byte.
  static const AddressType_t AddressType__extended = 15;


/* HEADER BYTE MANIPULATION */

// Set a set of header bits to true/false.
  inline void setHeaderMask(Data_t &header, uint8_t bit_mask, bool new_value){
    if(new_value)
      header |= bit_mask;
    else
      header &= ~bit_mask;
  }

// Check/configure checksum-present bit flag in header
  inline bool get_checksumPresent(Data_t header){
    return (header & ChecksumPresent_Mask);
  }
  inline Data_t set_checksumPresent(Data_t header, bool new_value){
    setHeaderMask(header, ChecksumPresent_Mask, new_value);
    return header;
  }

// Check/configure next-proto-present bit flag in header
  inline bool get_nextProtoPresent(Data_t header){
    return (header & NextProtoPresent_Mask);
  }
  inline Data_t set_nextProtoPresent(Data_t header, bool new_value){
    setHeaderMask(header, NextProtoPresent_Mask, new_value);
    return header;
  }

// Check/configure dest-address-present bit flag in header
  inline bool get_destAddressPresent(Data_t header){
    return (header & DestAddressPresent_Mask);
  }
  inline Data_t set_destAddressPresent(Data_t header, bool new_value){
    setHeaderMask(header, DestAddressPresent_Mask, new_value);
    return header;
  }

// Check/configure src-address-present bit flag in header
  inline bool get_srcAddressPresent(Data_t header){
    return (header & SrcAddressPresent_Mask);
  }
  inline Data_t set_srcAddressPresent(Data_t header, bool new_value){
    setHeaderMask(header, SrcAddressPresent_Mask, new_value);
    return header;
  }

// Check/configure addressType (in header)
  inline AddressType_t get_addressType(Data_t header){
    return (header & AddressType_Mask);
  }
  inline Data_t set_addressType(Data_t header, AddressType_t new_value){
// Can't handle expanded Types
    assert(new_value < 15);
  // Erase current Type
    header &= ~AddressType_Mask;
  // Set new Type
    header |= new_value;
  // Return newly Typed header
    return header;
  }


/* CHECKSUMS */

// Checksum type: 32-bit (Posix)
  typedef uint32_t Checksum_t;
};

// MAPPacket class
#include "MAPPacket.hpp"

namespace MAP {

// Reference a packet.
inline void referencePacket(MAPPacket *packet){
  packet->incrementReferenceCount();
}
// Dereference a packet, and free the packet
// if it is no longer referenced by anyone.
inline void dereferencePacket(MAPPacket *packet){
  if(packet->decrementReferenceCount() == 0)
  // Packet frees its own Buffers.
    delete packet;
}

}

// MAPPacketSink class
#include "MAPPacketSink.hpp"

namespace MAP {

// Allocate a new packet.
bool allocateNewPacket(MAPPacket **packet, uint16_t capacity);

// PacketChecksumGenerator: inline MAPPacketSink.
// Appends checksums to packets as necessary.
// Returns Good if checksum already present or was appended, in which
// case packet has already been passed to the configured Sink.
class PacketChecksumGenerator : public MAP::MAPPacketSink {
// Next sink, to which the packet is relayed if a checksum is already prsent
// or successfully appended.
  MAP::MAPPacketSink *nextSink;

public:

  PacketChecksumGenerator(MAP::MAPPacketSink *new_nextSink)
  : nextSink(new_nextSink)
  { }

// Generate a checksum for a packet.
  Status::Status_t sinkPacket(MAPPacket *packet){
    assert(nextSink != NULL);
    assert(packet != NULL);

// Generate checksum and append, if necessary
    if(packet->appendChecksum())
      return nextSink->sinkPacket(packet);
  // Checksum generation can fail if add'l memory cannot be allocated, for instance.
    else
      return Status::Status__Bad;
  }
};

// PacketValidator: inline MAPPacketSink.
// Validates packets, including checksums and MAP headers.
class PacketValidator : public MAP::MAPPacketSink {
// Next sink, to which the packet is relayed if valid.
  MAP::MAPPacketSink *nextSink;

public:

  PacketValidator(MAP::MAPPacketSink *new_nextSink)
  : nextSink(new_nextSink)
  { }

// Validate a packet.
  Status::Status_t sinkPacket(MAP::MAPPacket *packet){
    assert(nextSink != NULL);
    assert(packet != NULL);

  // Validate checksum
    if(packet->validate()){
      DEBUGprint("PacketValidator: Packet valid.\n");
      return nextSink->sinkPacket(packet);
    }else{
      DEBUGprint("PacketValidator: Packet invalid.\n");
      return Status::Status__Bad;
    }
  }
};


// End namespace: MAP
};

