#pragma once

#include "../globals.hpp"
#include "../../DataStore/Buffer.hpp"
#include "../MAP/MAP.hpp"

// SimpleServer class
class SimpleServer : public MAP::MAPPacketSink {
protected:
  MAP::OffsetMAPPacket offsetPacket;

  static const uint8_t InitialReplyPacketCapacity = 8;
  static const uint8_t IncrementReplyPacketCapacity = 8;

public:

  SimpleServer()
  : offsetPacket(NULL, 0)
  { }

  Status::Status_t sinkPacket(MAP::MAPPacket* const packet, MAP::MAPPacket::Offset_t headerOffset){
    if(offsetPacket.packet != NULL)
      return Status::Status__Bad;

    offsetPacket.packet = packet;
    offsetPacket.headerOffset = headerOffset;
// Note packet in use.
    MAP::referencePacket(offsetPacket.packet);

    return Status::Status__Good;
  }

  Status::Status_t finishedWithPacket(MAP::MAPPacket* const secondPacket){
    MAP::dereferencePacket(secondPacket);
    return finishedWithPacket();
  }

  Status::Status_t finishedWithPacket(){
    offsetPacket.packet->sinkStatus(Status::Status__Complete);
    MAP::dereferencePacket(offsetPacket.packet);
    offsetPacket.packet = NULL;

    return Status::Status__Good;
  }

  bool prepareReply(MAP::MAPPacket **replyPacket, MAP::MAPPacket *packet, uint16_t data_capacity);
};

