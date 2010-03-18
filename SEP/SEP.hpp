// SEP (S-Packet Encapsulation Protocol) definitions
//
// The S-Packet Encapsulation Protocol encodes an endless series of S-Packets over a raw serial data stream.

#pragma once

#include "../DataTransfer/DataSource.hpp"

namespace SEP {
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

// Status codes
  typedef uint8_t Status_t;
// Successful / Acceptable
  static const Status_t Status__Good = 0;
// Rejected / Aborted
  static const Status_t Status__Bad = 1;
// Busy / In Progress / Unavailable
  static const Status_t Status__Busy = 2;
// Completed / Finished
  static const Status_t Status__Complete = 3;

  inline bool isControlByte(uint8_t byte){
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

