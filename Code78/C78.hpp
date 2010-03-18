// C78.hpp
// C++
//
// C78 encoder and decoders

#pragma once

#include "C78Encoder.hpp"
#include "C78Decoder.hpp"

inline bool C78_isLastByte(uint8_t enc_data){
  return ((enc_data & 0x80) == 0);
}

