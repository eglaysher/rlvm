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

#include "Systems/Base/VoiceCache.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/filesystem/path.hpp>
#include <iomanip>
#include <sstream>
#include <string>

#include "Systems/Base/KOEPACVoiceArchive.hpp"
#include "Systems/Base/NWKVoiceArchive.hpp"
#include "Systems/Base/OVKVoiceArchive.hpp"
#include "Systems/Base/OVKVoiceSample.hpp"
#include "Systems/Base/SoundSystem.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/VoiceArchive.hpp"
#include "Utilities/Exception.hpp"

#include <iostream>
using namespace std;

const int ID_RADIX = 100000;

using boost::iends_with;
using boost::shared_ptr;
using std::string;

namespace fs = boost::filesystem;

VoiceCache::VoiceCache(SoundSystem& sound_system)
    : sound_system_(sound_system),
      file_cache_(7) {
}

VoiceCache::~VoiceCache() {
}

boost::shared_ptr<VoiceSample> VoiceCache::find(int id) {
  int file_no = id / ID_RADIX;
  int index = id % ID_RADIX;

  shared_ptr<VoiceArchive> archive = file_cache_.fetch(file_no);
  if (archive) {
    return archive->findSample(index);
  } else {
    archive = findArchive(file_no);
    if (archive) {
      // Cache for later use.
      file_cache_.insert(file_no, archive);
      return archive->findSample(index);
    } else {
      // There aren't any archives with |file_no|. Look for an individual file
      // instead.
      shared_ptr<VoiceSample> sample = findUnpackedSample(file_no, index);
      if (sample) {
        return sample;
      } else {
        throw rlvm::Exception("No such voice archive or sample");
      }
    }
  }
}

shared_ptr<VoiceArchive> VoiceCache::findArchive(int file_no) const {
  std::ostringstream oss;
  oss << "z" << std::setw(4) << std::setfill('0') << file_no;

  fs::path file =
      sound_system_.system().findFile(oss.str(), KOE_ARCHIVE_FILETYPES);
  if (file.empty()) {
    return shared_ptr<VoiceArchive>();
  }

  string file_str = file.file_string();
  if (iends_with(file_str, "ovk")) {
    return shared_ptr<VoiceArchive>(new OVKVoiceArchive(file, file_no));
  } else if (iends_with(file_str, "nwk")) {
    return shared_ptr<VoiceArchive>(new NWKVoiceArchive(file, file_no));
  } else if (iends_with(file_str, "koe")) {
    return shared_ptr<VoiceArchive>(new KOEPACVoiceArchive(file, file_no));
  }

  return shared_ptr<VoiceArchive>();
}

shared_ptr<VoiceSample> VoiceCache::findUnpackedSample(
    int file_no, int index) const {
  // Loose voice files are packed into directories, like:
  // /KOE/0008/z000800073.ogg. We only need to search for the filename though.
  std::ostringstream oss;
  oss << "z"
      << std::setw(4) << std::setfill('0') << file_no
      << std::setw(5) << std::setfill('0') << index;

  fs::path file =
      sound_system_.system().findFile(oss.str(), KOE_LOOSE_FILETYPES);
  string file_str = file.file_string();

  if (iends_with(file_str, "ogg")) {
    return shared_ptr<VoiceSample>(new OVKVoiceSample(file));
  }

  return shared_ptr<VoiceSample>();
}
