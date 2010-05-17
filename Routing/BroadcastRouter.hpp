// Copyright (C) 2010, Aret N Carlsen (aretcarlsen@autonomoustools.com).
// Dynamic routing handlers (C++).
// Licensed under GPLv3 and later versions. See license.txt or <http://www.gnu.org/licenses/>.


// MAP routing devices
//
// StaticLocalMAPServerBinding; BroadcastRouter; MAPPacketStore; SimpleServerProcess

#pragma once

#include "../MAP/MAP.hpp"
#include "../../Process/Process.hpp"
#include "../../DataStore/Buffer.hpp"

// Router which broadcasts received packets to all configured sinks.
class BroadcastRouter : public MAP::MAPPacketSink {
private:
// Max of 255 sinks
  typedef uint8_t Capacity_t;

// Static
  DataStore::ArrayBuffer<MAPPacketSink*, Capacity_t> sinks;

public:

  BroadcastRouter(MAPPacketSink** const sinks_buffer, const Capacity_t sinks_buffer_capacity)
  : sinks(sinks_buffer, sinks_buffer_capacity)
  { }

  Status::Status_t addSink(MAPPacketSink* const &new_sink){
    return sinks.sinkData(new_sink);
  }

// Signal/slot style broadcasting.
  Status::Status_t sinkPacket(MAP::MAPPacket *packet, Offset_t headerOffset){
  // Note packet in use.
    MAP::referencePacket(packet);

  // Does not stop after an acceptance.
    for(MAPPacketSink **packetSink = sinks.front(); packetSink < sinks.back(); packetSink++){
      (*packetSink)->sinkPacket(packet, headerOffset);
    }

  // Free packet.
    MAP::dereferencePacket(packet);

    return Status::Status__Good;
  }
};

