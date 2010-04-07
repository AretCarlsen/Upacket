#pragma once

#include "../globals.hpp"
#include "../MAP/MAP.hpp"

struct AddressFilter {
public:
  typedef uint8_t Mode_t;
  static const Mode_t ModeMask__Negate = 0x80;
  static const Mode_t ModeMask__Opcode = 0x7F;
  // Negated MatchAll is essentially equivalent to Inactive
  static const Mode_t Mode__MatchAll = 1;
  static const Mode_t Mode__AddressType = 2;
  static const Mode_t Mode__MaskedAddressValue = 3;

//private:

  // Matching mode
  Mode_t mode;

  // Index into an array of packetSinks
  uint8_t packetSinkIndex;

  // Header offset (for encapsulation/nesting)
  MAP::MAPPacket::Offset_t headerOffset;

 // This implementation supports only types and values less than 128.
  // Type
  uint8_t addressType;
  uint8_t addressValue;
  // Mask applied to the value
  uint8_t addressValueMask;

public:

  AddressFilter(Mode_t new_mode, uint8_t new_packetSinkIndex,
                const uint8_t &new_addressType = 0x00, const uint8_t &new_addressValue = 0x00, const uint8_t &new_addressValueMask = 0xFF,
                uint8_t new_headerOffset = 0)
  : mode(new_mode), packetSinkIndex(new_packetSinkIndex), headerOffset(new_headerOffset),
    addressType(new_addressType), addressValue(new_addressValue), addressValueMask(new_addressValueMask)
  { }
  
  bool isMatch(const uint8_t cmp_addressType, uint8_t* cmp_addressValue);
};

class AddressGraph : public MAP::MAPPacketSink {
private:
  uint8_t localAddressType;
  uint8_t localAddressValue;
  
  DataStore::ArrayBuffer<MAPPacketSink*, uint8_t> *packetSinks;
  DataStore::DynamicArrayBuffer<AddressFilter,uint8_t> addressEdges;

  static const uint8_t DefaultInitialCapacity = 3;
  static const uint8_t DefaultCapacityIncrement = 3;
  static const uint8_t DefaultMaxCapacity = 12;

public:

  AddressGraph(uint8_t new_localAddressType, uint8_t new_localAddressValue, 
               DataStore::ArrayBuffer<MAPPacketSink*, uint8_t> *new_packetSinks,
               uint8_t initial_capacity = DefaultInitialCapacity)
  : localAddressType(new_localAddressType), localAddressValue(new_localAddressValue),
    packetSinks(new_packetSinks),
    addressEdges(initial_capacity)
  { }

  Status::Status_t sinkPacket(MAP::MAPPacket* const packet, MAP::MAPPacket::Offset_t headerOffset);

  // Note that edge is copied by value.
  bool sinkEdge(const AddressFilter &filter){
    return addressEdges.sinkExpand(filter,  DefaultCapacityIncrement, DefaultMaxCapacity);
  }
  
  // bool clearEdges(){
  // }
  
  void process_command_packet(MAP::MAPPacket* const packet){
    DEBUGprint("Processing command packet.\n");
  }
};

