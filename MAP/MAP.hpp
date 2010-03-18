//
// Micro Address Protocol common definitions
//

#pragma once

namespace MAP {
// Address types
  typedef uint8_t AddressType_t;

// Address not provided
  static const AddressType_t AddressType__NotPresent = 0;
// Hardware bus local. Not intended for distribution
// beyond the receiving hardware.
  static const AddressType_t AddressType__HardwareLocal = 1;
// Device local. Not intended for relay beyond the target device. 
  // Static addresses are typically used for incoming connections.
  // A specific static address can be requested by a process.
  static const AddressType_t AddressType__DeviceLocalStatic = 2;
  // Dynamic addresses are allocated to processes as needed.
  // Processes are not expected to request a specific dynamic address.
  static const AddressType_t AddressType__DeviceLocalDynamic = 3;
// Raw low-value addresses, particular to a LAN
  static const AddressType_t AddressType__LANLocal = 4;
// MAC addresses, a la ethernet
  static const AddressType_t AddressType__MAC = 5;
// IP addresses, whether v4 or v6. Note that further differentiation
// may be necessary for the MAC next-proto level.
  static const AddressType_t AddressType__IP = 6;
// DNS addresses. Typically lengthy textual addresses in a logical
// dot-delimited hierarchy.
  static const AddressType_t AddressType__DNS = 7;

// Maximum allowed address type. (For sanity checks.)
  static const AddressType_t AddressType__MAX = AddressType__DNS;

// Bit positions and masks for the MAP header
  typedef uint8_t MaskType_t;
  static const MaskType_t DestAddressMask = 0b111;
  static const uint8_t SrcAddressShift = 3;
  static const MaskType_t SrcAddressMask = 0b111 << SrcAddressShift;
  static const MaskType_t ChecksumPresentMask = 0b1 << (3 + 3);
  static const MaskType_t NextProtoPresentMask = 0b1 << (3 + 3 + 1);

  typedef uint8_t Protocol_t;
  static const Protocol_t Protocol__Unknown = 1;
  static const Protocol_t Protocol__MAP     = 2;
  static const Protocol_t Protocol__ICMP    = 3;
  static const Protocol_t Protocol__MAC     = 4;
  static const Protocol_t Protocol__IP      = 5;
  static const Protocol_t Protocol__UDP     = 6;
  static const Protocol_t Protocol__TCP     = 7;
  static const Protocol_t Protocol__ARP     = 8;
  static const Protocol_t Protocol__DNS     = 9;

  inline bool checksumPresent(uint8_t header){
    return (header & ChecksumPresentMask);
  }
  inline bool nextProtoPresent(uint8_t header){
    return (header & NextProtoPresentMask);
  }
  inline uint8_t srcAddressType(uint8_t header){
    return (header & SrcAddressMask) >> SrcAddressShift;
  }
  inline uint8_t destAddressType(uint8_t header){
    return (header & DestAddressMask);
  }

};

