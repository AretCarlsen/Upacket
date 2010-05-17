// Copyright (C) 2010, Aret N Carlsen (aretcarlsen@autonomoustools.com).
// Dynamic routing handlers (C++).
// Licensed under GPLv3 and later versions. See license.txt or <http://www.gnu.org/licenses/>.


#pragma once

#include "../globals.hpp"
#include "../MAP/MAP.hpp"

struct AddressFilter {
public:
  typedef uint8_t Mode_t;
  static const Mode_t ModeMask__Negate = 0x80;
  static const Mode_t ModeMask__Opcode = 0x7F;

  static const Mode_t Mode__Inactive = 0;
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
  MAP::MAPPacket::HeaderOffset_t headerOffset;

 // This implementation supports only types and values less than 128.
  // Type
  uint8_t addressType;
  uint8_t addressValue;
  // Mask applied to the value
  uint8_t addressValueMask;

public:

  AddressFilter(Mode_t new_mode = Mode__Inactive, uint8_t new_packetSinkIndex = 0,
                const uint8_t &new_addressType = 0x00, const uint8_t &new_addressValue = 0x00, const uint8_t &new_addressValueMask = 0xFF,
                uint8_t new_headerOffset = 0)
  : mode(new_mode), packetSinkIndex(new_packetSinkIndex), headerOffset(new_headerOffset),
    addressType(new_addressType), addressValue(new_addressValue), addressValueMask(new_addressValueMask)
  { }
  
  bool isMatch(const uint8_t cmp_addressType, uint8_t* cmp_addressValue);

  bool operator==(const AddressFilter &cmpFilter) const{
    return (memcmp(this, &cmpFilter, sizeof(cmpFilter)) == 0);
  }

  uint8_t debugPrintValues(){
    DEBUGprint("m%d i%d h%d aT%d aV%d aM%d\n", mode, packetSinkIndex, headerOffset, addressType, addressValue, addressValueMask);
    return 0;
  }
};

class AddressGraph : public MAP::MAPPacketSink {
private:
  uint8_t localAddressType;
  uint8_t localAddressValue;
  
  DataStore::ArrayBuffer<MAPPacketSink*, uint8_t> *packetSinks;
  DataStore::DynamicArrayBuffer<AddressFilter,uint8_t> addressEdges;

  static const uint8_t DefaultInitialCapacity = 0;
  static const uint8_t DefaultCapacityIncrement = 4;
  static const uint8_t DefaultMaxCapacity = 16;

  typedef uint8_t Opcode_t;
  static const Opcode_t Opcode__Add = 1;
  static const Opcode_t Opcode__Remove = 2;
  static const Opcode_t Opcode__RemoveAll = 3;

public:

// HEAP
  AddressGraph(uint8_t new_localAddressType, uint8_t new_localAddressValue, 
               DataStore::ArrayBuffer<MAPPacketSink*, uint8_t> *new_packetSinks,
               MemoryPool *new_memoryPool, uint8_t initial_capacity = DefaultInitialCapacity)
  : localAddressType(new_localAddressType), localAddressValue(new_localAddressValue),
    packetSinks(new_packetSinks),
    addressEdges(new_memoryPool, initial_capacity)
  {
    assert(new_packetSinks != NULL);
    assert(new_memoryPool != NULL);
  }

  Status::Status_t sinkPacket(MAP::MAPPacket* const packet, MAP::MAPPacket::HeaderOffset_t headerOffset);

  // Note that edge is copied by value.
  bool sinkEdge(const AddressFilter &newEdge){
    DEBUGprint_RARE("AG: edge sink: ");
    for(AddressFilter *filter = addressEdges.front(); filter < addressEdges.back(); filter++){
      DEBUG_AG(filter->debugPrintValues());
  // Check for an existing edge that matches exactly
      if(*filter == newEdge){
        DEBUGprint_RARE("fld (exists)\n");
        return true;
      }
  // Check for an existing edge that is inactive
      if(filter->mode == AddressFilter::Mode__Inactive){
        DEBUGprint_RARE("scs (rplc)\n");
        *filter = newEdge;
        return true;
      }
    }

    bool skExp = addressEdges.sinkExpand(newEdge, DefaultCapacityIncrement, DefaultMaxCapacity);
    DEBUGprint_RARE("skExp: %d\n", (skExp? 1 : 0));
    return skExp;
  }

  void removeEdge(AddressFilter &edge){
// Mark edge as inactive
    edge.mode = AddressFilter::Mode__Inactive;
  }

  void clearEdges(){
    addressEdges.set_size(0);
  }
  
  void process_command_packet(MAP::MAPPacket* const packet, MAP::MAPPacket::HeaderOffset_t headerOffset);
  void command_add(MAP::MAPPacket* const packet, MAP::Data_t *data_ptr);
  void command_remove(MAP::MAPPacket* const packet, MAP::Data_t *data_ptr);

  friend class EepromAddressGraph;
};

