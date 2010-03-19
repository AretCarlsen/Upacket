#pragma once

#include "../../DataTransfer/DataTransfer.hpp"
#include "../../DataStore/DynamicArray.hpp"
#include "../Bpacket/Bpacket.hpp"
#include "../../Process/Process.hpp"

namespace NP {

// Encode a packet to an outgoing bytestream.
//
// Complete is indicated as newline.
class NPEncoder : public Packet::BpacketSink, public Process {
private:
// NP-encoded outgoing data
  DataTransfer::DataSink<uint8_t, Status::Status_t> *dataSink;

// Current packet
  Packet::Bpacket* packet;

// Current packet data pointer
  Packet::Data_t *packetData;

public:

// Constructor
  NPEncoder(DataTransfer::DataSink<SEP::Data_t, Status::Status_t> *new_dataSink)
  : dataSink(new_dataSink),
    packet(NULL)
  { }

// Accept a packet to be encoded
  Status::Status_t sinkPacket(Packet::Bpacket *new_packet){
    if(packet != NULL)
      return Packet::Status__Busy;

    packet = new_packet;
    packetData = packet->front();
  }

// Continue encoding the packet.
  Status::Status_t process(){
  // Anything to process?
    if(packet == NULL)
      return Process::Status__Good;

  // Process any available data.
    while(packetData < packet->back()){
    // Attempt to sink the data.
      if(dataSink->sinkData(*packetData) != DataTransfer::Status__Good)
        return Process::Status__Good;

    // Iterate.
      packetData++;
    }

  // Attempt to sink end-of-packet
    if(dataSink->sinkData('\n') != DataTransfer::Status__Good)
      return Process::Status__Good;

  // Indicate packet complete
    packet->sinkStatus(Packet::Status__Complete);
  // Reset state
    packet = NULL;

    return Process::Status__Good;
  }
};

// Decode packets from an encoded byte stream.
//
// Newlines are interpreted as Complete status events. Exclamation points are Bad (abort packet).
class NPDecoder : public DataTransfer::DataSink<uint8_t, Status::Status_t> {
private:

  Packet::BpacketSink *packetSink;
  Packet::Bpacket *packet;

// Initial packet capacity, in bytes
  static const uint8_t PacketCapacity__Initial = 20;
// Packet resizing increment, in bytes
  static const uint8_t PacketCapacity__Increment = 10;

public:

// Constructor
  NPDecoder(Packet::BpacketSink *new_packetSink)
  : packetSink(new_packetSink),
    packet(NULL)
  { }

// Expand the current packet.
  bool enlargePacketCapacity(){
    assert(packet != NULL);

    return packet->setCapacity(packet->getCapacity() + PacketCapacity__Increment);
  }

// Accept NP-encoded data to be decoded.
  Status::Status_t sinkData(SEP::Data_t data){
    // Complete packet?
    if(data == '\n'){
      packetSink->sinkPacket(packet);
      packet = NULL;

    // Discard packet?
    }else if(data == '!'){
      discardPacket();

    // Regular data
    }else{
  // Start a new packet, if necessary.
      if(packet == NULL){
    // Attempt to allocate a new packet.
        if(! generateNewPacket())
          return DataTransfer::Status__Busy;

  // Enlarge the packet, if necessary
      }else if( packet->isFull() && !(enlargePacketCapacity()) )
        return DataTransfer::Status__Busy;

      packet->append(data);

    }

    return DataTransfer::Status__Good;
  }

// Discard the current packet.
  void discardPacket(){
    // Discard the packet
    packet->deconstructor();
    free(packet);
    packet = NULL;
  }

// Allocate a new packet.
  bool generateNewPacket(){
  // Attempt to allocate initial packet
    packet = (Packet::Bpacket*) malloc(sizeof(Packet::Bpacket));
    if(packet == NULL)
      return false;
    packet->constructor(PacketCapacity__Initial);
  }
};

// End namespace: NP
}

