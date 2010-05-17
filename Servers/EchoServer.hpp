// Copyright (C) 2010, Aret N Carlsen (aretcarlsen@autonomoustools.com).
// Fundamental MAP servers (C++).
// Licensed under GPLv3 and later versions. See license.txt or <http://www.gnu.org/licenses/>.


#include "SimpleServer.hpp"

class EchoServer : public SimpleServer, public Process {
public:

  EchoServer(MAPPacketSink *new_packetSink, MemoryPool *new_memoryPool)
  : SimpleServer(new_memoryPool, new_packetSink)
  {
    assert(new_memoryPool != NULL);
  }

// Assumes the packet has been validated!!
  Status::Status_t process(){
  // Packet in progress?
    if(offsetPacket.packet == NULL)
      return Status::Status__Good;

    DEBUGprint_MISC("EchSrv: proc pack\n");

    MAP::MAPPacket *replyPacket;
  // Attempt to prepare a reply packet large enough to contain the received packet's contents.
  // Only works if the sender encapsulated a MAP-encoded destination address.
  // Returns false if the source packet was invalid (not encapsulated, missing
  // a dest address, dest address too long), or if the packet data could not
  // be allocated.
    MAP::Data_t* data_ptr = offsetPacket.packet->get_data(offsetPacket.headerOffset);
    if(prepareReply(&replyPacket, offsetPacket.packet, offsetPacket.packet->back() - data_ptr)){

  // Append the received packet contents.
      for(; data_ptr < offsetPacket.packet->back(); data_ptr++)
        replyPacket->sinkData(*data_ptr);

  // Send the packet on its way.
      sendPacket(replyPacket);
    }else{
      DEBUGprint_MISC("EchSrv: reply prep fld\n");
    }
  // Finished with the source packet.
    finishedWithPacket();
    return Status::Status__Good;
  }
};

