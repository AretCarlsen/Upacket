// MAP routing devices
//
// StaticLocalMAPServerBinding; BroadcastRouter; MAPPacketStore; SimpleServerProcess

#pragma once

#include "../MAP/MAP.hpp"
#include "../../Process/Process.hpp"
#include "../../DataStore/Buffer.hpp"

// Definition of a single address.
// Allows for comparison.
class AddressDefinition {
public:
  virtual bool compareAddress(MAP::AddressType_t *cmp_addressType, MAP::Data_t *cmp_addressValue) = 0;
};

class ShortAddressDefinition : public AddressDefinition {
private:
  MAP::AddressType_t addressType;
  uint8_t addressValue;

public:

  ShortAddressDefinition(MAP::AddressType_t new_addressType, uint8_t new_addressValue)
  : addressType(new_addressType), addressValue(new_addressValue)
  { }

  bool compareAddress(MAP::AddressType_t *cmp_addressType, uint8_t *cmp_addressValue){
    return ( (*cmp_addressType == addressType) && (*cmp_addressValue == addressValue) );
  }
};

/*
class MultipleShortAddressDefinition : public AddressDefinition {
private:
// Hosts a static ArrayBuffer
};
*/

// MAP packet filter, address-based.
class ShortAddressPacketFilter : public MAP::MAPPacketSink {
private:
  AddressDefinition *addressDefinition;
  MAP::MAPPacketSink *packetSink;

public:
  ShortAddressPacketFilter(AddressDefinition *new_addressDefinition, MAP::MAPPacketSink *new_packetSink)
  : addressDefinition(new_addressDefinition),
    packetSink(new_packetSink)
  { }
  
  Status::Status_t sinkPacket(MAP::MAPPacket *packet){
  // Check address.
    MAP::Data_t *header = packet->get_first_header();
  // Look for header with dest address of correct type.
    for(; header != NULL; header = packet->get_next_header(header)){
    // Move along if no dest address present.
      if(! MAP::get_destAddressPresent(*header)){
        DEBUGprint("Binding filter: No dest address in this header.\n");
        continue;
      }
      MAP::Data_t addressType = MAP::get_addressType(*header);
    // Move along if dest address does not match.
      if(! addressDefinition->compareAddress(&addressType, packet->get_destAddress(header))){
        DEBUGprint("Binding filter: Dest address does not match filter.\n");
        continue;
      }

    // Match found!
      break;
    }
  // Check if ran out of addresses to test before hitting a match.
    if(header == NULL){
      DEBUGprint("Binding filter: Blocking packet.\n");
      return Status::Status__Bad;
    }

    DEBUGprint("Binding filter: Accepting packet.\n");
    return packetSink->sinkPacket(packet);
  }
};

// Router which broadcasts received packets to all configured sinks.
class BroadcastRouter : public MAP::MAPPacketSink {
private:
// Max of 255 sinks
  typedef uint8_t Capacity_t;

// Static
  DataStore::ArrayBuffer<MAPPacketSink*, Capacity_t> sinks;

public:

  BroadcastRouter(MAPPacketSink **sinks_buffer, Capacity_t sinks_buffer_capacity)
  : sinks(sinks_buffer, sinks_buffer_capacity)
  { }

  Status::Status_t addSink(MAPPacketSink *new_sink){
    return sinks.sinkData(new_sink);
  }

// Signal/slot style broadcasting.
  Status::Status_t sinkPacket(MAP::MAPPacket *packet){
  // Note packet in use.
    MAP::referencePacket(packet);

  // Does not stop after an acceptance.
    for(MAPPacketSink **packetSink = sinks.front(); packetSink < sinks.back(); packetSink++){
      (*packetSink)->sinkPacket(packet);
    }

  // Free packet.
    MAP::dereferencePacket(packet);

    return Status::Status__Good;
  }
};

