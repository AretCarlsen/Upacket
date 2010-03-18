// C78Decoder implementation

#pragma once

// Only usable by one caller at a time! (between reset()s)
class C78Decoder : public DataSink<uint8_t> {
private:
  DataSink<uint8_t> *dataSink;
  uint8_t decData;
  uint8_t dataMask; 
 
  const static uint8_t DATAMASK_INITIAL_VALUE = 0x7F;

public:
  C78Decoder()
  : dataSink(NULL)
  {
    reset();
  }

  C78Decoder(DataSink<uint8_t> *new_dataSink)
  : dataSink(new_dataSink)
  {
    reset();
  }
  
  inline void setDataSink(DataSink<uint8_t> *new_dataSink){
    dataSink = new_dataSink;
  }
  
  inline void reset(){
    decData = 0;
    dataMask = DATAMASK_INITIAL_VALUE;
  }

// Returns true when decoding is complete; returns false otherwise.
  bool sinkData(uint8_t data);
};

