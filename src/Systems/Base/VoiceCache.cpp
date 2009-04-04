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

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

#include "Systems/Base/VoiceCache.hpp"

#include <iomanip>
#include <sstream>
#include <boost/filesystem/path.hpp>

#include "Systems/Base/OVKVoiceArchive.hpp"
#include "Systems/Base/SoundSystem.hpp"
#include "Systems/Base/VoiceArchive.hpp"
#include "Utilities/Exception.hpp"
#include "Utilities/File.hpp"

const int ID_RADIX = 100000;

using boost::iends_with;
using boost::shared_ptr;

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
  if (!archive) {
    archive = findArchive(file_no);
    if (!archive) {
      throw rlvm::Exception("No such voice archive");
    }

    file_cache_.insert(file_no, archive);
  }

  return archive->findSample(index);
}

shared_ptr<VoiceArchive> VoiceCache::findArchive(int file_no) const {
  std::ostringstream oss;
  oss << "z" << std::setw(4) << std::setfill('0') << file_no;

  fs::path file = findFile(sound_system_.system(), oss.str(), KOE_FILETYPES);
  string file_str = file.file_string();

  if (iends_with(file_str, "ovk")) {
    return shared_ptr<VoiceArchive>(new OVKVoiceArchive(file, file_no));
  }

  return shared_ptr<VoiceArchive>();
}
