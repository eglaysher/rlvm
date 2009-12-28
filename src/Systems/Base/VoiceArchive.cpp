// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2009 Elliot Glaysher
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
// -----------------------------------------------------------------------

#include <cstring>

#include "Systems/Base/VoiceArchive.hpp"

#include "xclannad/endian.hpp"

namespace {

// Header at the beginning of WAV data.
unsigned char orig_header[0x2c] = {
  0x52, 0x49, 0x46, 0x46, /* +00 "RIFF" */
  0x00, 0x00, 0x00, 0x00, /* +04 file size - 8 */
  0x57, 0x41, 0x56, 0x45, 0x66, 0x6d, 0x74, 0x20, /* +08 "WAVEfmt " */
  0x10, 0x00, 0x00, 0x00, /* +10 fmt size */
  0x01, 0x00,             /* +14 wFormatTag */
  0x02, 0x00,             /* +16 Channels */
  0x44, 0xac, 0x00, 0x00, /* +18 rate */
  0x10, 0xb1, 0x02, 0x00, /* +1c BytesPerSec = rate * BlockAlign */
  0x04, 0x00,             /* +20 BlockAlign = channels*BytesPerSample */
  0x10, 0x00,             /* +22 BitsPerSample */
  0x64, 0x61, 0x74, 0x61, /* +24 "data" */
  0x00, 0x00, 0x00, 0x00  /* +28 filesize - 0x2c */
};

}  // namespace

// -----------------------------------------------------------------------
// VoiceSample
// -----------------------------------------------------------------------
VoiceSample::~VoiceSample() {
}

// static
const char* VoiceSample::MakeWavHeader(int rate, int ch, int bps, int size) {
  static char header[0x2c];
  memcpy(header, (const char*)orig_header, 0x2c);
  write_little_endian_int(header+0x04, size-8);
  write_little_endian_int(header+0x28, size-0x2c);
  write_little_endian_int(header+0x18, rate);
  write_little_endian_int(header+0x1c, rate*ch*bps);
  header[0x16] = ch;
  header[0x20] = ch*bps;
  header[0x22] = bps*8;
  return header;
}

// -----------------------------------------------------------------------
// VoiceArchive
// -----------------------------------------------------------------------
VoiceArchive::VoiceArchive(int file_no)
    : file_no_(file_no) {
}

// -----------------------------------------------------------------------

VoiceArchive::~VoiceArchive() {
}

// -----------------------------------------------------------------------

VoiceArchive::Entry::Entry(int ikoe_num, int ilength, int ioffset)
    : koe_num(ikoe_num), length(ilength), offset(ioffset) {
}
