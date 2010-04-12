
namespace MAP {

class MAPPacketSink {
public:
  // HeaderOffset limited to 256, obviously.
  virtual Status::Status_t sinkPacket(MAPPacket *packet, MAPPacket::Offset_t headerOffset = 0) = 0;
};

class OffsetMAPPacket {
public:
  MAPPacket *packet;
  MAPPacket::Offset_t headerOffset;

  OffsetMAPPacket(MAPPacket *new_packet = NULL, MAPPacket::Offset_t new_headerOffset = 0)
  : packet(new_packet), headerOffset(new_headerOffset)
  { }
};

class MAPPacketBuffer : public MAPPacketSink, public Process {
  DataStore::RingBuffer<OffsetMAPPacket, uint8_t> packetBuffer;

  MAPPacketSink *packetSink;

public:

  MAPPacketBuffer(MAPPacketSink *new_packetSink, OffsetMAPPacket* raw_packet_buffer, uint8_t buffer_capacity)
  : packetBuffer(raw_packet_buffer, buffer_capacity),
    packetSink(new_packetSink)
  { }

  Status::Status_t sinkPacket(MAPPacket* const packet, MAPPacket::Offset_t headerOffset){
  // If the packet buffer is empty, try to sink the packet immediately.
    if(packetBuffer.is_empty()){
    // Try to sink the packet
      Status::Status_t tempStatus = packetSink->sinkPacket(packet, headerOffset);
    // If the packet sink was not busy, return transparently.
      if(tempStatus != Status::Status__Busy)
        return tempStatus;
    }

/*
  // If the packet buffer is full, return Busy.
    if(packetBuffer.is_full())
      return Status::Status__Busy;
    // Could skip safety checks, since we have already verified the buffer is not full.
*/

  // Reference the packet.
    referencePacket(packet);
  // Attempt to append the packet to the buffer
    if(packetBuffer.sinkData(OffsetMAPPacket(packet, headerOffset))){
    // Packet has been buffered.
      return Status::Status__Good;
    }

  // Buffering failed
    dereferencePacket(packet);
    return Status::Status__Busy;
  }

  Status::Status_t process(){
  // Attempt to sink a buffered packet, if any are pending..
    if(! packetBuffer.is_empty()){
    // Temporarily pop a packet. If the sink does not return Busy, permanently remove
    // the packet from the buffer.
      OffsetMAPPacket offsetPacket = packetBuffer.get_in_place();
      if(packetSink->sinkPacket(offsetPacket.packet, offsetPacket.headerOffset) != Status::Status__Busy){
        // Finish pop.
        packetBuffer.increment_read_position();
        // Dereference the packet (which we referenced upon sinking).
        dereferencePacket(offsetPacket.packet);
      }
    }

  // Good.
    return Status::Status__Good;
  }
};

// End namespace: MAP
}

