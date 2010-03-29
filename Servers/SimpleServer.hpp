#pragma once

#include "Routing.hpp"

// SimpleServer class
class SimpleServer : public MAP::MAPPacketSink {
protected:
  MAP::MAPPacket *packet;

  static const uint8_t InitialReplyPacketCapacity = 8;
  static const uint8_t IncrementReplyPacketCapacity = 8;

public:

  SimpleServer()
  : packet(NULL)
  { }

  Status::Status_t sinkPacket(MAP::MAPPacket *new_packet){
    if(packet != NULL)
      return Status::Status__Bad;

    packet = new_packet;
// Note packet in use.
    MAP::referencePacket(packet);

    return Status::Status__Good;
  }

  void finishedWithPacket(){
    packet->sinkStatus(Status::Status__Complete);
    MAP::dereferencePacket(packet);
    packet = NULL;
  }

  bool prepareReply(MAP::MAPPacket **replyPacket, MAP::MAPPacket *packet, uint16_t data_capacity);
};

