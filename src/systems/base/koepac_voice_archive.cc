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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
//
// -----------------------------------------------------------------------
//
// Parts of this file have been copied from koedec.cc in the xclannad
// distribution and they are:
//
// Copyright (c) 2004-2006  Kazunori "jagarl" Ueno
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. The name of the author may not be used to endorse or promote products
//    derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// -----------------------------------------------------------------------

#include "systems/base/koepac_voice_archive.h"

#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/path.hpp>

#include <algorithm>
#include <cstring>
#include <fstream>
#include <sstream>
#include <vector>

#include "utilities/exception.h"
#include "xclannad/endian.hpp"

using std::ifstream;
using std::ostringstream;
namespace fs = boost::filesystem;

namespace {

// KOEPAC translation tables provided by Jagarl.

/* 8bit -> 16bit への変換テーブル。本来は signed short だが
** とりあえず unsigned で扱っている
*/

uint16_t koe_8bit_trans_tbl[256] = {
    0x8000, 0x81ff, 0x83f9, 0x85ef, 0x87e1, 0x89cf, 0x8bb9, 0x8d9f, 0x8f81,
    0x915f, 0x9339, 0x950f, 0x96e1, 0x98af, 0x9a79, 0x9c3f, 0x9e01, 0x9fbf,
    0xa179, 0xa32f, 0xa4e1, 0xa68f, 0xa839, 0xa9df, 0xab81, 0xad1f, 0xaeb9,
    0xb04f, 0xb1e1, 0xb36f, 0xb4f9, 0xb67f, 0xb801, 0xb97f, 0xbaf9, 0xbc6f,
    0xbde1, 0xbf4f, 0xc0b9, 0xc21f, 0xc381, 0xc4df, 0xc639, 0xc78f, 0xc8e1,
    0xca2f, 0xcb79, 0xccbf, 0xce01, 0xcf3f, 0xd079, 0xd1af, 0xd2e1, 0xd40f,
    0xd539, 0xd65f, 0xd781, 0xd89f, 0xd9b9, 0xdacf, 0xdbe1, 0xdcef, 0xddf9,
    0xdeff, 0xe001, 0xe0ff, 0xe1f9, 0xe2ef, 0xe3e1, 0xe4cf, 0xe5b9, 0xe69f,
    0xe781, 0xe85f, 0xe939, 0xea0f, 0xeae1, 0xebaf, 0xec79, 0xed3f, 0xee01,
    0xeebf, 0xef79, 0xf02f, 0xf0e1, 0xf18f, 0xf239, 0xf2df, 0xf381, 0xf41f,
    0xf4b9, 0xf54f, 0xf5e1, 0xf66f, 0xf6f9, 0xf77f, 0xf801, 0xf87f, 0xf8f9,
    0xf96f, 0xf9e1, 0xfa4f, 0xfab9, 0xfb1f, 0xfb81, 0xfbdf, 0xfc39, 0xfc8f,
    0xfce1, 0xfd2f, 0xfd79, 0xfdbf, 0xfe01, 0xfe3f, 0xfe79, 0xfeaf, 0xfee1,
    0xff0f, 0xff39, 0xff5f, 0xff81, 0xff9f, 0xffb9, 0xffcf, 0xffe1, 0xffef,
    0xfff9, 0xffff, 0x0000, 0x0001, 0x0007, 0x0011, 0x001f, 0x0031, 0x0047,
    0x0061, 0x007f, 0x00a1, 0x00c7, 0x00f1, 0x011f, 0x0151, 0x0187, 0x01c1,
    0x01ff, 0x0241, 0x0287, 0x02d1, 0x031f, 0x0371, 0x03c7, 0x0421, 0x047f,
    0x04e1, 0x0547, 0x05b1, 0x061f, 0x0691, 0x0707, 0x0781, 0x07ff, 0x0881,
    0x0907, 0x0991, 0x0a1f, 0x0ab1, 0x0b47, 0x0be1, 0x0c7f, 0x0d21, 0x0dc7,
    0x0e71, 0x0f1f, 0x0fd1, 0x1087, 0x1141, 0x11ff, 0x12c1, 0x1387, 0x1451,
    0x151f, 0x15f1, 0x16c7, 0x17a1, 0x187f, 0x1961, 0x1a47, 0x1b31, 0x1c1f,
    0x1d11, 0x1e07, 0x1f01, 0x1fff, 0x2101, 0x2207, 0x2311, 0x241f, 0x2531,
    0x2647, 0x2761, 0x287f, 0x29a1, 0x2ac7, 0x2bf1, 0x2d1f, 0x2e51, 0x2f87,
    0x30c1, 0x31ff, 0x3341, 0x3487, 0x35d1, 0x371f, 0x3871, 0x39c7, 0x3b21,
    0x3c7f, 0x3de1, 0x3f47, 0x40b1, 0x421f, 0x4391, 0x4507, 0x4681, 0x47ff,
    0x4981, 0x4b07, 0x4c91, 0x4e1f, 0x4fb1, 0x5147, 0x52e1, 0x547f, 0x5621,
    0x57c7, 0x5971, 0x5b1f, 0x5cd1, 0x5e87, 0x6041, 0x61ff, 0x63c1, 0x6587,
    0x6751, 0x691f, 0x6af1, 0x6cc7, 0x6ea1, 0x707f, 0x7261, 0x7447, 0x7631,
    0x781f, 0x7a11, 0x7c07, 0x7fff};

/* ADPCM・・・じゃないらしい。ただのDPCMのナめたテーブル。
** 自動生成すりゃいいんだけど256byteだったら
** テーブルでも問題ないでしょ
*/

uint8_t koe_ad_trans_tbl[256] = {
    0x00, 0xff, 0x01, 0xfe, 0x02, 0xfd, 0x03, 0xfc, 0x04, 0xfb, 0x05, 0xfa,
    0x06, 0xf9, 0x07, 0xf8, 0x08, 0xf7, 0x09, 0xf6, 0x0a, 0xf5, 0x0b, 0xf4,
    0x0c, 0xf3, 0x0d, 0xf2, 0x0e, 0xf1, 0x0f, 0xf0, 0x10, 0xef, 0x11, 0xee,
    0x12, 0xed, 0x13, 0xec, 0x14, 0xeb, 0x15, 0xea, 0x16, 0xe9, 0x17, 0xe8,
    0x18, 0xe7, 0x19, 0xe6, 0x1a, 0xe5, 0x1b, 0xe4, 0x1c, 0xe3, 0x1d, 0xe2,
    0x1e, 0xe1, 0x1f, 0xe0, 0x20, 0xdf, 0x21, 0xde, 0x22, 0xdd, 0x23, 0xdc,
    0x24, 0xdb, 0x25, 0xda, 0x26, 0xd9, 0x27, 0xd8, 0x28, 0xd7, 0x29, 0xd6,
    0x2a, 0xd5, 0x2b, 0xd4, 0x2c, 0xd3, 0x2d, 0xd2, 0x2e, 0xd1, 0x2f, 0xd0,
    0x30, 0xcf, 0x31, 0xce, 0x32, 0xcd, 0x33, 0xcc, 0x34, 0xcb, 0x35, 0xca,
    0x36, 0xc9, 0x37, 0xc8, 0x38, 0xc7, 0x39, 0xc6, 0x3a, 0xc5, 0x3b, 0xc4,
    0x3c, 0xc3, 0x3d, 0xc2, 0x3e, 0xc1, 0x3f, 0xc0, 0x40, 0xbf, 0x41, 0xbe,
    0x42, 0xbd, 0x43, 0xbc, 0x44, 0xbb, 0x45, 0xba, 0x46, 0xb9, 0x47, 0xb8,
    0x48, 0xb7, 0x49, 0xb6, 0x4a, 0xb5, 0x4b, 0xb4, 0x4c, 0xb3, 0x4d, 0xb2,
    0x4e, 0xb1, 0x4f, 0xb0, 0x50, 0xaf, 0x51, 0xae, 0x52, 0xad, 0x53, 0xac,
    0x54, 0xab, 0x55, 0xaa, 0x56, 0xa9, 0x57, 0xa8, 0x58, 0xa7, 0x59, 0xa6,
    0x5a, 0xa5, 0x5b, 0xa4, 0x5c, 0xa3, 0x5d, 0xa2, 0x5e, 0xa1, 0x5f, 0xa0,
    0x60, 0x9f, 0x61, 0x9e, 0x62, 0x9d, 0x63, 0x9c, 0x64, 0x9b, 0x65, 0x9a,
    0x66, 0x99, 0x67, 0x98, 0x68, 0x97, 0x69, 0x96, 0x6a, 0x95, 0x6b, 0x94,
    0x6c, 0x93, 0x6d, 0x92, 0x6e, 0x91, 0x6f, 0x90, 0x70, 0x8f, 0x71, 0x8e,
    0x72, 0x8d, 0x73, 0x8c, 0x74, 0x8b, 0x75, 0x8a, 0x76, 0x89, 0x77, 0x88,
    0x78, 0x87, 0x79, 0x86, 0x7a, 0x85, 0x7b, 0x84, 0x7c, 0x83, 0x7d, 0x82,
    0x7e, 0x81, 0x7f, 0x80};

}  // namespace

// -----------------------------------------------------------------------
// KOEPACVoiceSample
// -----------------------------------------------------------------------
class KOEPACVoiceSample : public VoiceSample {
 public:
  KOEPACVoiceSample(fs::path file, int offset, int length, int rate)
      : stream_(std::fopen(file.native().c_str(), "rb")),
        offset_(offset),
        length_(length),
        rate_(rate) {}

  virtual ~KOEPACVoiceSample() {
    if (stream_)
      fclose(stream_);
  }

  virtual char* Decode(int* size) override;

 private:
  FILE* stream_;
  int offset_;
  int length_;
  int rate_;
};

char* KOEPACVoiceSample::Decode(int* dest_len) {
  // This function has been mildly adapted from decode_koe in xclannad. I have
  // modified types so that it works on 64-bit systems and changed malloc()s to
  // new[]s, as the consumer of decode() will delete [] the returned pointer.

  // avg32 の声データ展開
  std::unique_ptr<char[]> table(new char[length_ * 2]);
  fseek(stream_, offset_, 0);
  fread(table.get(), 2, length_, stream_);

  int all_len = 0;
  for (int i = 0; i < length_; i++)
    all_len += read_little_endian_short(table.get() + i * 2);

  // データ読み込み
  uint8_t* src_orig = new uint8_t[all_len];
  uint16_t* dest_orig = new uint16_t[length_ * 0x1000 + 0x2c];

  if (src_orig == NULL || dest_orig == NULL)
    return NULL;
  uint8_t* src = src_orig;
  fread(src, 1, all_len, stream_);
  *dest_len = length_ * 0x400 * 4;
  const char* header = MakeWavHeader(rate_, 2, 2, *dest_len);
  memcpy(dest_orig, header, 0x2c);
  uint16_t* dest = dest_orig + 0x2c;

  // 展開
  for (int i = 0; i < length_; i++) {
    int slen = read_little_endian_short(table.get() + i * 2);
    if (slen == 0) {  // do nothing
      memset(dest, 0, 0x1000);
      dest += 0x800;
      src += 0;
    } else if (slen == 0x400) {  // table 変換
      for (int j = 0; j < 0x400; j++) {
        write_little_endian_short((char*)(dest + 0), koe_8bit_trans_tbl[*src]);
        write_little_endian_short((char*)(dest + 1), koe_8bit_trans_tbl[*src]);
        dest += 2;
        src++;
      }
    } else {  // DPCM
      uint8_t d = 0;
      uint16_t o2;
      for (int j = 0, k = 0; j < slen && k < 0x800; j++) {
        uint8_t s = src[j];
        if ((s + 1) & 0x0f) {
          d -= koe_ad_trans_tbl[s & 0x0f];
        } else {
          uint8_t s2;
          s >>= 4;
          s &= 0x0f;
          s2 = s;
          s = src[++j];
          s2 |= (s << 4) & 0xf0;
          d -= koe_ad_trans_tbl[s2];
        }
        o2 = koe_8bit_trans_tbl[d];
        write_little_endian_short((char*)(dest + k), o2);
        write_little_endian_short((char*)(dest + k + 1), o2);
        k += 2;
        s >>= 4;
        if ((s + 1) & 0x0f) {
          d -= koe_ad_trans_tbl[s & 0x0f];
        } else {
          d -= koe_ad_trans_tbl[src[++j]];
        }
        o2 = koe_8bit_trans_tbl[d];
        write_little_endian_short((char*)(dest + k), o2);
        write_little_endian_short((char*)(dest + k + 1), o2);
        k += 2;
      }
      dest += 0x800;
      src += slen;
    }
  }
  delete[] src_orig;

  return (char*)dest_orig;
}

// -----------------------------------------------------------------------
// KOEPACVoiceArchive
// -----------------------------------------------------------------------
KOEPACVoiceArchive::KOEPACVoiceArchive(fs::path file, int file_no)
    : VoiceArchive(file_no), file_(file) {
  ReadTable(file);
}

// -----------------------------------------------------------------------

KOEPACVoiceArchive::~KOEPACVoiceArchive() {}

// -----------------------------------------------------------------------

std::shared_ptr<VoiceSample> KOEPACVoiceArchive::FindSample(int sample_num) {
  std::vector<Entry>::const_iterator it =
      std::lower_bound(entries_.begin(), entries_.end(), sample_num);
  if (it != entries_.end()) {
    return std::shared_ptr<VoiceSample>(
        new KOEPACVoiceSample(file_, it->offset, it->length, rate_));
  }

  throw rlvm::Exception("Couldn't find sample in KOEPACVoiceArchive");
}

// -----------------------------------------------------------------------

void KOEPACVoiceArchive::ReadTable(boost::filesystem::path file) {
  fs::ifstream ifs(file, ifstream::in | ifstream::binary);
  if (!ifs) {
    std::ostringstream oss;
    oss << "Could not open file \"" << file << "\".";
    throw rlvm::Exception(oss.str());
  }

  // Copied from koedec.cc
  char head[0x20];
  ifs.read(head, 0x20);
  if (strncmp(head, "KOEPAC", 7) != 0) {
    std::ostringstream oss;
    oss << file << " does not appear to be in KOEPAC format";
    throw rlvm::Exception(oss.str());
  }

  int table_len = read_little_endian_int(head + 0x10);
  entries_.reserve(table_len);

  rate_ = read_little_endian_int(head + 0x18);
  if (rate_ == 0) {
    rate_ = 22050;
  }

  char* buf = new char[table_len * 8];
  ifs.read(buf, table_len * 8);
  for (int i = 0; i < table_len; i++) {
    int koe_num = read_little_endian_short(buf + i * 8);
    int length = read_little_endian_short(buf + i * 8 + 2);
    int offset = read_little_endian_int(buf + i * 8 + 4);
    entries_.emplace_back(koe_num, length, offset);
  }
  sort(entries_.begin(), entries_.end());

  delete[] buf;
}
