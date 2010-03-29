
namespace MAP {

class MAPPacketSink {
public:
  virtual Status::Status_t sinkPacket(MAPPacket *packet) = 0;
};

class MAPPacketBuffer : public MAPPacketSink, public Process {
  DataStore::RingBuffer<MAPPacket*, uint8_t> packetBuffer;

  MAPPacketSink *packetSink;

public:

  MAPPacketBuffer(MAPPacketSink *new_packetSink, MAPPacket** raw_packet_buffer, uint8_t buffer_capacity)
  : packetBuffer(raw_packet_buffer, buffer_capacity),
    packetSink(new_packetSink)
  { }

  Status::Status_t sinkPacket(MAPPacket* packet){
  // If the packet buffer is empty, try to sink the packet immediately.
    if(packetBuffer.is_empty()){
    // Try to sink the packet
      Status::Status_t tempStatus = packetSink->sinkPacket(packet);
    // If the packet sink was not busy, return transparently.
      if(tempStatus != Status::Status__Busy)
        return tempStatus;

  // If the packet buffer is full, return Busy.
    }else if(packetBuffer.is_full())
      return Status::Status__Busy;

  // Reference the packet.
    referencePacket(packet);
  // Append the packet to the buffer
    // Could skip safety checks, since we have already verified the buffer is not full.
    packetBuffer.sinkData(packet);

  // Packet has been buffered.
    return Status::Status__Good;
  }

  Status::Status_t process(){
  // Attempt to sink a buffered packet, if any are pending..
    if(! packetBuffer.is_empty()){
    // Temporarily pop a packet. If the sink does not return Busy, permanently remove
    // the packet from the buffer.
      MAPPacket* packet = packetBuffer.get_in_place();
      if(packetSink->sinkPacket(packet) != Status::Status__Busy){
        // Finish pop.
        packetBuffer.increment_read_position();
        // Dereference the packet.
        dereferencePacket(packet);
      }
    }

  // Good.
    return Status::Status__Good;
  }
};

// End namespace: MAP
}

