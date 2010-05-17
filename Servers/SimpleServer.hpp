// Copyright (C) 2010, Aret N Carlsen (aretcarlsen@autonomoustools.com).
// Fundamental MAP servers (C++).
// Licensed under GPLv3 and later versions. See license.txt or <http://www.gnu.org/licenses/>.


#pragma once

#include "../globals.hpp"
#include "../../DataStore/Buffer.hpp"
#include "../MAP/MAP.hpp"

// SimpleServer class
class SimpleServer : public MAP::MAPPacketSink {
protected:
// Memory pool (from which packets are sourced)
  MemoryPool *memoryPool;
// Outgoing packet sink
  MAP::MAPPacketSink *outputPacketSink;
// Incoming packet container
  MAP::OffsetMAPPacket offsetPacket;

  static const uint8_t InitialReplyPacketCapacity = 8;
  static const uint8_t IncrementReplyPacketCapacity = 8;

public:

  SimpleServer(MemoryPool *new_memoryPool = NULL, MAP::MAPPacketSink *new_outputPacketSink = NULL)
  : memoryPool(new_memoryPool), outputPacketSink(new_outputPacketSink),
    offsetPacket(NULL, 0)
  { }

  inline Status::Status_t sinkPacket(MAP::MAPPacket* const packet, MAP::MAPPacket::HeaderOffset_t headerOffset){
    if(offsetPacket.packet != NULL)
      return Status::Status__Bad;

    offsetPacket.packet = packet;
    offsetPacket.headerOffset = headerOffset;
// Note packet in use.
    MAP::referencePacket(offsetPacket.packet);

    return Status::Status__Good;
  }

  inline bool packetPending(){
    return (offsetPacket.packet != NULL);
  }

protected:
  inline Status::Status_t finishedWithPacket(MAP::MAPPacket* const secondPacket){
    if(secondPacket != NULL) MAP::dereferencePacket(secondPacket);
    return finishedWithPacket();
  }

  inline Status::Status_t finishedWithPacket(){
    if(offsetPacket.packet != NULL){
      offsetPacket.packet->sinkStatus(Status::Status__Complete);
      MAP::dereferencePacket(offsetPacket.packet);
      offsetPacket.packet = NULL;
    }

    return Status::Status__Good;
  }

  inline bool allocateNewPacket(MAP::MAPPacket** const packet, const uint16_t &capacity){
    return MAP::allocateNewPacket(packet, capacity, memoryPool);
  }

  inline bool sendPacket(MAP::MAPPacket* outPacket){
    MAP::referencePacket(outPacket);
    bool sinkReturn = outputPacketSink->sinkPacket(outPacket);
    MAP::dereferencePacket(outPacket);
    return sinkReturn;
  }

  bool prepareReply(MAP::MAPPacket **replyPacket, MAP::MAPPacket *packet, uint16_t data_capacity);
  bool replyBoolean(bool value);
  bool replyC78(uint32_t value);
  bool replyC78String(uint8_t* const buf, uint16_t buf_len);
};

