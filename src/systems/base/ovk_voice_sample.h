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
// -----------------------------------------------------------------------

#ifndef SRC_SYSTEMS_BASE_OVK_VOICE_SAMPLE_H_
#define SRC_SYSTEMS_BASE_OVK_VOICE_SAMPLE_H_

#include <boost/filesystem/path.hpp>
#include <vorbis/vorbisfile.h>

#include "systems/base/voice_archive.h"

class OVKVoiceSample : public VoiceSample {
 public:
  // Creates a sample from a full .ogg |file|.
  explicit OVKVoiceSample(boost::filesystem::path file);

  // Creates a sample from an ogg file embedded in the archive at |file|.
  OVKVoiceSample(boost::filesystem::path file, int offset, int length);
  virtual ~OVKVoiceSample();

  // Overridden from VoiceSample:
  virtual char* Decode(int* size) override;

 private:
  static size_t ogg_readfunc(void* ptr,
                             size_t size,
                             size_t nmemb,
                             OVKVoiceSample* datasource);
  static int ogg_seekfunc(OVKVoiceSample* datasource,
                          ogg_int64_t new_offset,
                          int whence);
  static long ogg_tellfunc(OVKVoiceSample* datasource);  // NOLINT

  FILE* stream_;
  int offset_;
  int length_;
};

#endif  // SRC_SYSTEMS_BASE_OVK_VOICE_SAMPLE_H_
