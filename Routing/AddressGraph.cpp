
#include "AddressGraph.hpp"

bool AddressFilter::isMatch(const uint8_t cmp_addressType, uint8_t* cmp_addressValue){
  // Initialize, just for fun.
  bool matched = false;

  switch(mode & ModeMask__Opcode){
    case Mode__MatchAll:
      matched = true;
      break;
    case Mode__AddressType:
      matched = (cmp_addressType == addressType);
      break;
    case Mode__MaskedAddressValue:
      matched = ( (cmp_addressType == addressType) && ((*cmp_addressValue & addressValueMask) == addressValue) );
      break;
  // Invalid mode
    default:
      return false;
  };
  
  // Negate, if configured
  if(mode & ModeMask__Negate)
    matched = !matched;

  return matched;
}

Status::Status_t AddressGraph::sinkPacket(MAP::MAPPacket* const packet, uint8_t headerOffset){
  DEBUGprint("AddressGraph::sinkpacket: Processing packet, size %X, hO %X.\n", packet->get_size(),  headerOffset);

  MAP::Data_t *header = packet->get_header(headerOffset);
  if(header == NULL) return Status::Status__Bad;

  MAP::Data_t destAddressType = MAP::get_addressType(*header);
  MAP::Data_t *destAddressValue = packet->get_destAddress(header);
  if(destAddressValue == NULL) return Status::Status__Bad;

  if((destAddressType == localAddressType) && (*destAddressValue == localAddressValue))
    process_command_packet(packet);

  DEBUGprint("AddressGraph::sinkpacket: Running packet through graph.\n");
  for(AddressFilter *filter = addressEdges.front(); filter < addressEdges.back(); filter++){
    if(filter->isMatch(destAddressType, destAddressValue) && (filter->packetSinkIndex < packetSinks->get_size())){
      DEBUGprint("AddressGraph::sinkpacket: Edge filter accepts: index %X, offset %X.\n", filter->packetSinkIndex, filter->headerOffset);
      packetSinks->get(filter->packetSinkIndex)->sinkPacket(packet, filter->headerOffset);
    }else{
      DEBUGprint("AddressGraph::sinkpacket: Edge filter rejects.\n");
    }
  }

  return Status::Status__Good;
}

