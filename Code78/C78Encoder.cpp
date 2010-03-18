// C78Encoder implementation

#include "C78Encoder.hpp"
#include <assert.h>

// Incoming data is always buffered by 1-2 bytes, because the encoded byte cannot be sent
// until it known whether another data byte is forthcoming
//
// Every 7th data byte warrants an extra encoded byte; again, however, this extra byte will not be transmitted
// until the 8th data byte to determine the encoded MSb.
bool C78Encoder::sinkData(uint8_t data){
  assert(dataSink != NULL);

// bitPos is now between 0 and 6 inclusive,
// or the special value 0x80 which indicates "first byte"

// First byte: Just store
  if(bitPos == 0x80){
    olderData = 0;
    newerData = data;
  // Begin regular processing
    bitPos = 0;
    return true;
  }

// Bitpos is incremented at the beginning of the loop, rather than the end.
  bitPos++;
// bitPos is now between 1 and 7 inclusive

// Data mask
  uint8_t dataMask = 0xFF >> bitPos;

// Send MSbs of older with LSbs of newer
  dataSink->sinkData(((uint8_t) 0x80) | olderData | (newerData & dataMask));

// bitPos == 7: Carriage return
  if(bitPos == 7){
  // Send remaining data
    dataSink->sinkData(0x80 | (newerData >> 1));
    olderData = 0;
    bitPos = 0;
  }else{
    olderData = (newerData & ~dataMask) >> 1;
  }
  newerData = data;

  return true;
}

void C78Encoder::endOfData(){
  assert(dataSink != NULL);

// bitPos == 0 or 1through6
  if(bitPos < 7){
  // bitPos == 0; 1 byte pending
    if(bitPos == 0){
  // Compressible MSB?  (MSB == 0 and LSB > 0)
      if((newerData < 0x80) && (newerData > 0))
    // Send LSbs
        dataSink->sinkData(newerData);
      else{
    // Send LSbs, then MSbs
        dataSink->sinkData(0x80 | (0x7F & newerData));
        dataSink->sinkData((newerData & 0x80) >> 1);
      }
  
  // bitPos between 1 and 6
    }else{
      uint8_t dataMask = 0x7F >> bitPos;

  // Compressible MSB?  (LSB>0 and MSB==0)
      if( ((newerData & dataMask) > 0)  &&  ((newerData & dataMask) == newerData) ){
      // Send LSbs
        dataSink->sinkData(olderData | (newerData & dataMask));

  // Uncompressible MSB
      }else{
      // Send LSbs, then MSbs
        dataSink->sinkData(0x80 | olderData | (newerData & dataMask));
        dataSink->sinkData((newerData & ~dataMask) >> 1);
      }
    }

// bitPos == 7 or 0x80
  }else{
    // Empty string
    if(bitPos == 0x80){
      dataSink->sinkData((uint8_t) 0x00);

    // bitPos == 7; 1 byte pending
    }else{
    // MSbs of the MSB; LSb has already been transmitted
    // (MSb is implicitly clear)
      dataSink->sinkData(newerData);
    }
  }

// Prepare for next data
  reset();
}

void C78Encoder::sinkArray(uint8_t *buf, uint8_t len){
// Use buffer sink from DataSink class
//   sinkData(buf, len);
// A while loop also works
  for(; len > 0; len--){
    sinkData(*buf);
    buf++;
  }

//  endOfData();
}

uint8_t logBase256(uint32_t num){
  uint8_t log256 = 0;
  while(num > 0){
    num = num >> 8;
    log256++;
  }
  return log256;

/*
// 1 or 2 bytes
  if(num <= 0xFFFF){  // < 0x100000
    if(num <= 0xFF)  // < 0x100
      return 1;
    else
      return 2;

// 3 or 4 bytes
  }else{
    if(num <= 0xFFFFFF)  // < 0x1000000
      return 3;
    else
      return 4;
  }
*/
}

void C78Encoder::sinkUint32(uint32_t num){
// Null
//  if(num == 0){
//    endOfData();
//    return
//  }

// logBase256 returns 0 for num==0
//  sinkArray(&num, logBase256(num));
  while(num > 0){
    sinkData(num & 0xFF);
    num = num >> 8;
  }
//  endOfData();
}

