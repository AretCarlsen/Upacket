// C78Encoder declarations

#pragma once

#include "../DataContainer/DataSink.hpp"

// Only usable by one caller at a time!
class C78Encoder : public DataSink<uint8_t> {
private:
  DataSink<uint8_t> *dataSink;
  uint8_t bitPos;
  uint8_t olderData;
  uint8_t newerData;

public:
  C78Encoder()
  : dataSink(NULL)
  {
    reset();
  }

  C78Encoder(DataSink<uint8_t> *new_dataSink)
  : dataSink(new_dataSink)
  {
    reset();
  }

// Set the outgoing sink
  inline void setDataSink(DataSink<uint8_t> *new_dataSink){
    dataSink = new_dataSink;
  }
  
// Reset the decoder state
  inline void reset(){
    bitPos = 0x80;
  // Reset a target buffer's length, for instance
//    dataSink->reset();
  }

// Signal end-of-data
  void endOfData();

/*
// Signals end of decoded data
  inline void interrupt(){
    endOfData();
  }
*/

// Sink a single byte
  bool sinkData(uint8_t data);
  
// Sink an entire array/buffer
  void sinkArray(uint8_t *buf, uint8_t len);

// Sink a 32-bit unsigned int
  void sinkUint32(uint32_t num);
};

