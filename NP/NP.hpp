#pragma once

#include "../../DataTransfer/DataTransfer.hpp"
#include "../../DataStore/Buffer.hpp"
#include "../Bpacket/Bpacket.hpp"
#include "../../Process/Process.hpp"
#include "../MEP/MEP.hpp"
#include "../MAP/MAP.hpp"

namespace NP {

// Encode a packet to an outgoing bytestream.
//
// Complete is indicated as newline.
class NPEncoder : public MAP::MAPPacketSink, public Process {
private:
// NP-encoded outgoing data
  DataTransfer::DataSink<uint8_t, Status::Status_t> *dataSink;

// Current packet
  Packet::Bpacket* packet;

// Current packet data pointer
  Packet::Data_t *packetData;

public:

// Constructor
  NPEncoder(DataTransfer::DataSink<MEP::Data_t, Status::Status_t> *new_dataSink)
  : dataSink(new_dataSink),
    packet(NULL)
  { }

// Accept a packet to be encoded
  Status::Status_t sinkPacket(Packet::Bpacket *new_packet){
    if(packet != NULL)
      return Status::Status__Busy;

    packet = new_packet;
    packetData = packet->front();
  }

// Continue encoding the packet.
  Status::Status_t process(){
  // Anything to process?
    if(packet == NULL)
      return Status::Status__Good;

  // Process any available data.
    while(packetData < packet->back()){
    // Attempt to sink the data.
      if(dataSink->sinkData(*packetData) != Status::Status__Good)
        return Status::Status__Good;

    // Iterate.
      packetData++;
    }

  // Attempt to sink end-of-packet
    if(dataSink->sinkData('\n') != Status::Status__Good)
      return Status::Status__Good;

  // Indicate packet complete
    packet->sinkStatus(Status::Status__Complete);
  // Reset state
    packet = NULL;

    return Status::Status__Good;
  }
};

// Decode packets from an encoded byte stream.
//
// Newlines are interpreted as Complete status events. Exclamation points are Bad (abort packet).
class NPDecoder : public DataTransfer::DataSink<uint8_t, Status::Status_t> {
private:

  MAP::MAPPacketSink *packetSink;
  MAP::MAPPacket *packet;

// Initial packet capacity, in bytes
  static const uint8_t PacketCapacity__Initial = 20;
// Packet resizing increment, in bytes
  static const uint8_t PacketCapacity__Increment = 10;

public:

// Constructor
  NPDecoder(MAP::MAPPacketSink *new_packetSink)
  : packetSink(new_packetSink),
    packet(NULL)
  { }

// Discard the current packet.
  void discardPacket(){
    Packet::dereferencePacket(packet);
    packet = NULL;
  }

// Accept NP-encoded data to be decoded.
  Status::Status_t sinkData(MEP::Data_t data){
  // Start a new packet, if necessary.
    if(packet == NULL){
    // Attempt to allocate a new packet.
      if(! allocateNewPacket())
        return Status::Status__Busy;

    }

    // Complete packet?
    if(data == '\n'){
    // Don't bother with empty packets.
      if(! packet->is_empty()){
        packetSink->sinkPacket(packet);
        discardPacket();
      }

    // Discard packet?
    }else if(data == '!'){
      discardPacket();

    // Regular data
    }else{
  // Enlarge the packet, if necessary
      if( packet->is_full() && !(expandPacketCapacity()) )
        return Status::Status__Busy;

      packet->append(data);
    }

    return Status::Status__Good;
  }

// Allocate a new packet.
  bool allocateNewPacket(){
// Attempt to allocate initial packet
    MAP::MAPPacket *newPacket = new MAP::MAPPacket;

// Allocation failed? Return false.
    if(newPacket == NULL)
      return false;

// Attempt to allocate buffer storage
    if(! newPacket->set_capacity(PacketCapacity__Initial)){
      delete newPacket;
      return false;
    }

DEBUGprint("allocateNewPacket: Allocated new packet.\n");

// Save new packet.
    packet = newPacket;
    Packet::referencePacket(packet);
    return true;
  }

// Expand the current packet.
  bool expandPacketCapacity(){
    assert(packet != NULL);

    return packet->set_capacity(packet->get_capacity() + PacketCapacity__Increment);
  }

};

// End namespace: NP
}

