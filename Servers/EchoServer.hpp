
#include "SimpleServer.hpp"

class EchoServer : public SimpleServer, public Process {
  MAPPacketSink *packetSink;

public:

  EchoServer(MAPPacketSink *new_packetSink)
  : packetSink(new_packetSink)
  { }

// Assumes the packet has been validated!!
  Status::Status_t process(){
  // Packet in progress?
    if(packet == NULL)
      return Status::Status__Good;

DEBUGprint("Echo server: Processing packet.\n");

    MAP::MAPPacket *replyPacket;
  // Attempt to prepare a reply packet large enough to contain the received packet's contents.
  // Only works if the sender encapsulated a MAP-encoded destination address.
  // Returns false if the source packet was invalid (not encapsulated, missing
  // a dest address, dest address too long), or if the packet data could not
  // be allocated.
    MAP::Data_t* data_ptr = packet->get_data();
    if(prepareReply(&replyPacket, packet, packet->back() - data_ptr)){

  // Append the received packet contents.
      for(; data_ptr < packet->back(); data_ptr++)
        replyPacket->sinkData(*data_ptr);

  // Send the packet on its way.
      MAP::referencePacket(replyPacket);
      packetSink->sinkPacket(replyPacket);
      MAP::dereferencePacket(replyPacket);
    }else{
      DEBUGprint("Echo server: Reply preparation failed.\n");
    }
  // Finished with the source packet.
    finishedWithPacket();
    return Status::Status__Good;
  }
};

