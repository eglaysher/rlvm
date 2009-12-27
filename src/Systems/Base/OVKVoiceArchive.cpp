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

#include "Systems/Base/OVKVoiceArchive.hpp"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>

#include "Systems/Base/OVKVoiceSample.hpp"
#include "Utilities/Exception.hpp"
#include "xclannad/endian.hpp"

using boost::shared_ptr;
using std::ifstream;
using std::ostringstream;
namespace fs = boost::filesystem;

// -----------------------------------------------------------------------
// OVKVoiceArchive
// -----------------------------------------------------------------------
OVKVoiceArchive::OVKVoiceArchive(fs::path file, int file_no)
    : VoiceArchive(file_no),
      file_(file) {
  readTable(file);
}

// -----------------------------------------------------------------------

OVKVoiceArchive::~OVKVoiceArchive() {
}

// -----------------------------------------------------------------------

shared_ptr<VoiceSample> OVKVoiceArchive::findSample(int sample_num) {
  std::vector<Entry>::const_iterator it =
      std::lower_bound(entries_.begin(), entries_.end(), sample_num);
  if (it != entries_.end()) {
    return shared_ptr<VoiceSample>(
        new OVKVoiceSample(file_, it->offset, it->length));
  }

  throw rlvm::Exception("Couldn't find sample in OVKVoiceArchive");
}

// -----------------------------------------------------------------------

void OVKVoiceArchive::readTable(boost::filesystem::path file) {
  fs::ifstream ifs(file, ifstream::in | ifstream::binary);
  if (!ifs) {
    ostringstream oss;
    oss << "Could not open file \"" << file << "\".";
    throw rlvm::Exception(oss.str());
  }

  // Copied from koedec.
  char head[0x20];
  ifs.read(head, 4);
  int table_len = read_little_endian_int(head);
  entries_.reserve(table_len);

  for (int i = 0; i < table_len; ++i) {
    ifs.read(head, 16);
    int length = read_little_endian_int(head);
    int offset = read_little_endian_int(head+4);
    int koe_num = read_little_endian_int(head+8);
    entries_.push_back(Entry(koe_num, length, offset));
  }
  sort(entries_.begin(), entries_.end());
}
