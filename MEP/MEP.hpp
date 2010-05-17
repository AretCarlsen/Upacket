// Copyright (C) 2010, Aret N Carlsen (aretcarlsen@autonomoustools.com).
// MEP packet handling (C++).
// Licensed under GPLv3 and later versions. See license.txt or <http://www.gnu.org/licenses/>.

// MEP (Micro Encapsulation Protocol) definitions
//
// The Micro Encapsulation Protocol encodes an endless series of packets over a raw serial data stream..
// Optimized for serial processing, small packets, and small addresses.

#pragma once

#include "../../DataTransfer/DataTransfer.hpp"
// Status codes
#include "../../Status/Status.hpp"

namespace MEP {
// Control prefix information
  typedef uint8_t ControlPrefix_t;
// Mask for 2-bit opcode data at the end of the control prefix
  static const ControlPrefix_t OpcodeMask = 0x03;
// The inverse of the opcode mask, if this portion of the opcode does not match
// then the opcode is taken as EndPacket + the first data byte of the new packet.
  static const ControlPrefix_t PrefixMask = 0xFC;

// A suggested default control prefix. Note that the control prefix may change during a "conversation".
  static const ControlPrefix_t DefaultControlPrefix = '<';  // 0x7C & ControlMask;  // == 0b01111100 == '|'

// Opcodes
  typedef uint8_t Opcode_t;
// Send control prefix as data (since it cannot be transmitted in the raw without invoking control mode).
  static const Opcode_t Opcode__SendControlPrefixAsData = 1;
// End the current packet. (status: Complete)
  static const Opcode_t Opcode__CompletePacket = 2;
// Abort the current packet. (status: Bad)
  static const Opcode_t Opcode__BadPacket = 3;

// Data
  typedef uint8_t Data_t;

  inline bool isControlByte(const uint8_t &byte){
    return (byte == DefaultControlPrefix);
  }
}

/*    Example packet transmission:
  ...     [data bytes]
  '<'     [control character]
  0x01    [send control prefix as data]
  '<'     [control character]
  0x02    [end packet]
*/

