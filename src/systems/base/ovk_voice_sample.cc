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
// Parts of this file have been copied from koedec_ogg.cc in the xclannad
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

#include "systems/base/ovk_voice_sample.h"

#include <vorbis/vorbisfile.h>

#include <cstdio>
#include <cstring>
#include <string>
#include <sstream>

#include "utilities/exception.h"
#include "xclannad/endian.hpp"

using std::ifstream;
using std::ostringstream;
namespace fs = boost::filesystem;

namespace {

const int INITSIZE = 65536;

std::string oggErrorCodeToString(int code) {
  switch (code) {
    case OV_FALSE:
      return "Not true, or no data available";
    case OV_HOLE:
      return "Vorbisfile encountered missing or corrupt data in the bitstream. "
             "Recovery is normally automatic and this return code is for "
             "informational purposes only.";
    case OV_EREAD:
      return "Read error while fetching compressed data for decode";
    case OV_EFAULT:
      return "Internal inconsistency in decode state. Continuing is likely "
             "not possible.";
    case OV_EIMPL:
      return "Feature not implemented";
    case OV_EINVAL:
      return "Either an invalid argument, or incompletely initialized argument "
             "passed to libvorbisfile call";
    case OV_ENOTVORBIS:
      return "The given file/data was not recognized as Ogg Vorbis data.";
    case OV_EBADHEADER:
      return "The file/data is apparently an Ogg Vorbis stream, but contains a "
             "corrupted or undecipherable header.";
    case OV_EVERSION:
      return "The bitstream format revision of the given stream is not "
             "supported.";
    case OV_EBADLINK:
      return "The given link exists in the Vorbis data stream, but is not "
             "decipherable due to garbage or corruption.";
    case OV_ENOSEEK:
      return "The given stream is not seekable";
    default:
      return "Unknown error";
  }
}

}  // namespace

OVKVoiceSample::OVKVoiceSample(fs::path file)
    : stream_(std::fopen(file.native().c_str(), "rb")), offset_(0), length_(0) {
  std::fseek(stream_, 0, SEEK_END);
  length_ = ftell(stream_);
  std::fseek(stream_, 0, SEEK_SET);
}

OVKVoiceSample::OVKVoiceSample(fs::path file, int offset, int length)
    : stream_(std::fopen(file.native().c_str(), "rb")),
      offset_(offset),
      length_(length) {}

OVKVoiceSample::~OVKVoiceSample() {
  if (stream_)
    fclose(stream_);
}

char* OVKVoiceSample::Decode(int* size) {
  // This function has been mildly adapted from decode_koe_ogg in xclannad.
  fseek(stream_, offset_, 0);

  ov_callbacks callback;
  callback.read_func = (size_t (*)(void*, size_t, size_t, void*))ogg_readfunc;
  callback.seek_func = (int (*)(void*, ogg_int64_t, int))ogg_seekfunc;
  callback.close_func = NULL;
  callback.tell_func = (long int (*)(void*))ogg_tellfunc;  // NOLINT

  OggVorbis_File vf;
  int r = ov_open_callbacks(this, &vf, NULL, 0, callback);
  if (r != 0) {
    ostringstream oss;
    oss << "Ogg stream error in OVKVoiceSample::decode: "
        << oggErrorCodeToString(r);
    throw std::runtime_error(oss.str());
  }

  vorbis_info* vinfo = ov_info(&vf, 0);
  int rate = vinfo->rate;
  int channels = vinfo->channels;

  // We start offsetted from the beginning of the buffer since this is where the
  // WAV header will be placed.
  int buffer_pos = WAV_HEADER_SIZE;
  int buffer_size = INITSIZE;
  char* buffer = NULL;

  try {
    buffer = new char[buffer_size];

    do {
      r = ov_read(
          &vf, buffer + buffer_pos, buffer_size - buffer_pos, 0, 2, 1, 0);
      if (r <= 0)
        break;
      buffer_pos += r;
      if ((buffer_size - INITSIZE / 4) < buffer_pos) {
        int new_size = buffer_size + INITSIZE;
        char* new_buffer = new char[new_size];
        memcpy(new_buffer, buffer, buffer_size);

        delete[] buffer;
        buffer = new_buffer;
        buffer_size = new_size;
      }
    } while (1);
    ov_clear(&vf);

    *size = buffer_size;
    const char* header = MakeWavHeader(rate, channels, 2, buffer_pos);
    memcpy(buffer, header, WAV_HEADER_SIZE);
  }
  catch (...) {
    delete[] buffer;
    throw;
  }

  return buffer;
}

size_t OVKVoiceSample::ogg_readfunc(void* ptr,
                                    size_t size,
                                    size_t nmemb,
                                    OVKVoiceSample* info) {
  int pt = ftell(info->stream_) - info->offset_;
  if (pt + size * nmemb > info->length_) {
    nmemb = (info->length_ - pt) / size;
  }
  return fread(ptr, size, nmemb, info->stream_);
}

int OVKVoiceSample::ogg_seekfunc(OVKVoiceSample* info,
                                 ogg_int64_t new_offset,
                                 int whence) {
  int pt = 0;
  if (whence == SEEK_SET)
    pt = info->offset_ + new_offset;
  else if (whence == SEEK_CUR)
    pt = ftell(info->stream_) + new_offset;
  else if (whence == SEEK_END)
    pt = info->offset_ + info->length_ + new_offset;
  return fseek(info->stream_, pt, 0);
}

long OVKVoiceSample::ogg_tellfunc(OVKVoiceSample* info) {  // NOLINT
  int pos = ftell(info->stream_);
  if (pos == -1)
    return -1;
  return pos - info->offset_;
}
