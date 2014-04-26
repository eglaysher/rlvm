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

#ifndef SRC_SYSTEMS_BASE_VOICE_CACHE_H_
#define SRC_SYSTEMS_BASE_VOICE_CACHE_H_

#include <memory>

#include "lru_cache.hpp"

class SoundSystem;
class VoiceArchive;
class VoiceSample;

class VoiceCache {
 public:
  explicit VoiceCache(SoundSystem& sound_system);
  ~VoiceCache();

  std::shared_ptr<VoiceSample> Find(int id);

 private:
  // Searches for a file archive of voices.
  std::shared_ptr<VoiceArchive> FindArchive(int file_no) const;

  // Searches for an unarchived ogg or mp3 file.
  std::shared_ptr<VoiceSample> FindUnpackedSample(int file_no,
                                                    int index) const;

  SoundSystem& sound_system_;

  // A mapping between a file id number and the underlying file object.
  LRUCache<int, std::shared_ptr<VoiceArchive>> file_cache_;
};  // class VoiceCache

#endif  // SRC_SYSTEMS_BASE_VOICE_CACHE_H_
