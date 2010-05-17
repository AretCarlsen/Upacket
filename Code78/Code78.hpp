// Copyright (C) 2010, Aret N Carlsen (aretcarlsen@autonomoustools.com).
// Code78 handling (C++).
// Licensed under GPLv3 and later versions. See license.txt or <http://www.gnu.org/licenses/>.


// C78 encoder and decoders

#pragma once

//#include "C78Encoder.hpp"
//#include "C78Decoder.hpp"

namespace Code78{

inline bool isLastByte(uint8_t enc_data){
  return ((enc_data & 0x80) == 0);
}

// End namespace: C78
}

