// Copyright (C) 2010, Aret N Carlsen (aretcarlsen@autonomoustools.com).
// Posix CRC32 checksum calculation (C++).
// Licensed under GPLv3 and later versions. See license.txt or <http://www.gnu.org/licenses/>.


// Posix CRC-32 Checksum definitions
//
// These are separated from the Engine class in order to allow system-specific
// storage of the precalculated 1kB table.

namespace PosixCRC32Checksum {

// Get the ith precalculated checksum table entry.
  uint32_t getChecksumTableEntry(uint8_t i);
}

