// C78Decoder implementation

#include "C78.hpp"
#include "C78Decoder.hpp"

#undef DEBUGprint
#undef DEBUG
//#define DEBUG

// Debugging
#ifdef DEBUG
#include <stdio.h>
#define DEBUGprint(args) (fprintf(stderr, "DEBUG SSPPDecoder.cpp: "), fprintf args)
#else
#define DEBUGprint(args)
#endif


bool C78Decoder::sinkData(uint8_t data){
// Check for MSB data.
  if(dataMask < 0x7F){
// decData gets MSB.
    decData |= (data & (~dataMask)) << 1;
    dataSink->sinkData(decData);
  }

// Optionally, watch for dataSink to return overflow value.

// If this is the last encoded byte
  if(C78_isLastByte(data)){
DEBUGprint((stderr, "Decoder: last byte: %X\n", data));
  data &= dataMask;

  // Check for a compressed MSB
    if(data != 0){
    // Sink the compressed MSB's LSbs
      dataSink->sinkData(data);
    }

  // Prepare for next data
    reset();

  // Return enumerated "finished" response
    return true;
  }

// decData gets LSB.
  decData = data & dataMask;

// Carriage return
  if(dataMask == 0x00){
    dataMask = DATAMASK_INITIAL_VALUE;
  }else
// Datamask shifts over to the right;
    dataMask = dataMask >> 1;
  
// Return "not finished" response
  return false;
}

