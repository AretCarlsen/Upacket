
#include "AddressGraph.hpp"

bool AddressFilter::isMatch(const uint8_t cmp_addressType, uint8_t* cmp_addressValue){
  // Initialize, just for fun.
  bool matched = false;

  switch(mode & ModeMask__Opcode){
    case Mode__Inactive:
      return false;
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
  DEBUGprint("AG::sP: Pack size %d, hO %d.\n", packet->get_size(), headerOffset);

  MAP::Data_t *header = packet->get_header(headerOffset);
  if(header == NULL) return Status::Status__Bad;

  MAP::Data_t destAddressType = MAP::get_addressType(*header);
  MAP::Data_t *destAddressValue = packet->get_destAddress(header);
  if(destAddressValue == NULL) return Status::Status__Bad;

  DEBUGprint("AG::sP: pack addy X%x/X%x\n", destAddressType, *destAddressValue, localAddressType, localAddressValue);
  if((destAddressType == localAddressType) && (*destAddressValue == localAddressValue)){
    DEBUGprint("AG::sP: cmd packet match\n");
    process_command_packet(packet, headerOffset);
  // Stop processing, to avoid loops.
    return Status::Status__Good;
  }

  for(AddressFilter *edge = addressEdges.front(); edge < addressEdges.back(); edge++){
    if(edge->isMatch(destAddressType, destAddressValue) && (edge->packetSinkIndex < packetSinks->get_size())){
      DEBUGprint("AG::sP: Edge accepts: i %d, hO %d.\n", edge->packetSinkIndex, edge->headerOffset);
      packetSinks->get(edge->packetSinkIndex)->sinkPacket(packet, headerOffset + edge->headerOffset);
    }
  }

  return Status::Status__Good;
}

void AddressGraph::command_add(MAP::MAPPacket* const packet, MAP::Data_t *data_ptr){
  DEBUGprint("cmd_add() st\n");
  AddressFilter newEdge;

  // Read sinkIndex
  if(data_ptr >= packet->back() || (*data_ptr & 0x80)) return;
  newEdge.packetSinkIndex = *data_ptr;
  data_ptr++;

  // Read headerOffset
  if(data_ptr >= packet->back() || (*data_ptr & 0x80)) return;
  newEdge.headerOffset = *data_ptr;
  data_ptr++;

  // Read mode
  if(data_ptr >= packet->back() || (*data_ptr & 0x80)) return;
  newEdge.mode = *data_ptr;
  data_ptr++;
  
  if(newEdge.mode == AddressFilter::Mode__MatchAll){
    DEBUGprint("cmd_add: sinkEdge, m MA\n");
    sinkEdge(newEdge);
    return;
  }

  DEBUGprint("cmd_add: r aT\n");
  // Read addressType
  if(data_ptr >= packet->back() || (*data_ptr & 0x80)) return;
  newEdge.addressType = *data_ptr;
  data_ptr++;
  
  if(newEdge.mode == AddressFilter::Mode__AddressType){
    DEBUGprint("cmd_add: sinkEdge, m AT\n");
    sinkEdge(newEdge);
    return;
  }

// Sanity check
  if(newEdge.mode != AddressFilter::Mode__MaskedAddressValue) return;

  DEBUGprint("cmd_add: r aV\n");
  // Read addressValue
  if(data_ptr >= packet->back() || (*data_ptr & 0x80)) return;
  newEdge.addressValue = *data_ptr;
  data_ptr++;

  DEBUGprint("cmd_add: r aVM\n");
  // Read addressValueMask
  if(data_ptr >= packet->back()) return;
  newEdge.addressValueMask = *data_ptr & 0x7F;
  // Value may be 128 to 255
  if(*data_ptr & 0x80){
    if(*data_ptr != 0x81) return;
    newEdge.addressValueMask <<= 7;
    data_ptr++;
    if(data_ptr >= packet->back() || (*data_ptr & 0x80)) return;
    newEdge.addressValueMask |= *data_ptr;
  }else
    newEdge.addressValueMask = *data_ptr;

  DEBUGprint("cmd_add: sinkEdge, m MV\n");
  sinkEdge(newEdge);

  return;
}

void AddressGraph::command_remove(MAP::MAPPacket* const packet, MAP::Data_t *data_ptr){
// Opcode == Remove
  AddressFilter newEdge;

  // Read sinkIndex
  if(data_ptr >= packet->back() || (*data_ptr & 0x80)) return;
  newEdge.packetSinkIndex = *data_ptr;
  data_ptr++;

  if(data_ptr >= packet->back()){
  // Remove everything for this sinkIndex
    for(AddressFilter *edge = addressEdges.front(); edge < addressEdges.back(); edge++){
      if(edge->packetSinkIndex == newEdge.packetSinkIndex)
        removeEdge(*edge);
    }
    return;
  }

  // Read mode
  if(*data_ptr & 0x80) return;
  newEdge.mode = *data_ptr;
  data_ptr++;

  if(data_ptr >= packet->back()){
  // Remove everything for this sinkIndex/mode
    for(AddressFilter *edge = addressEdges.front(); edge < addressEdges.back(); edge++){
      if(edge->packetSinkIndex == newEdge.packetSinkIndex && edge->mode == newEdge.mode)
        removeEdge(*edge);
    }
    return;
  }

  // Read addressType
  if(*data_ptr & 0x80) return;
  newEdge.addressType = *data_ptr;
  data_ptr++;

  if(data_ptr >= packet->back()){
  // Remove everything for this sinkIndex/mode/addressType
    for(AddressFilter *edge = addressEdges.front(); edge < addressEdges.back(); edge++){
      if(edge->packetSinkIndex == newEdge.packetSinkIndex && edge->mode == newEdge.mode && edge->addressType == newEdge.addressType)
        removeEdge(*edge);
    }
  }

  // Read addressValue
  if(*data_ptr & 0x80) return;
  newEdge.addressValue = *data_ptr;
  data_ptr++;
  if(data_ptr >= packet->back()){
  // Remove everything for this sinkIndex/mode/addressType/addressValue/headerOffset
    for(AddressFilter *edge = addressEdges.front(); edge < addressEdges.back(); edge++){
      if(edge->packetSinkIndex == newEdge.packetSinkIndex && edge->mode == newEdge.mode && edge->addressType == newEdge.addressType
         && edge->addressValue == newEdge.addressValue && edge->headerOffset == newEdge.headerOffset)
        removeEdge(*edge);
    }
  }

  return;
}

void AddressGraph::process_command_packet(MAP::MAPPacket* const packet, MAP::MAPPacket::Offset_t headerOffset){
  MAP::Data_t *data_ptr = packet->get_data(packet->get_header(headerOffset));
  if(data_ptr == NULL) return;

  // Read opcode
  if(*data_ptr & 0x80) return;
  uint8_t opcode = *data_ptr;
  data_ptr++;

  if(opcode == Opcode__Add)
    command_add(packet, data_ptr);
  else if(opcode == Opcode__RemoveAll)
    clearEdges();
  else if(opcode == Opcode__Remove)
    command_remove(packet, data_ptr);

  return;
}

