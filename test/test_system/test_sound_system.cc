// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2008 Elliot Glaysher
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

#include "test_system/test_sound_system.h"

#include <string>

// -----------------------------------------------------------------------
// TestSoundSystem
// -----------------------------------------------------------------------

TestSoundSystem::TestSoundSystem(System& system) : SoundSystem(system) {}

TestSoundSystem::~TestSoundSystem() {}

int TestSoundSystem::BgmStatus() const { return 0; }

void TestSoundSystem::BgmPlay(const std::string& bgm_name, bool loop) {
  bgm_name_ = bgm_name;
}

void TestSoundSystem::BgmPlay(const std::string& bgm_name,
                              bool loop,
                              int fade_in_ms) {
  bgm_name_ = bgm_name;
}

void TestSoundSystem::BgmPlay(const std::string& bgm_name,
                              bool loop,
                              int fade_in_ms,
                              int fade_out_ms) {
  bgm_name_ = bgm_name;
}

void TestSoundSystem::BgmStop() {}

void TestSoundSystem::BgmPause() {}

void TestSoundSystem::BgmUnPause() {}

void TestSoundSystem::BgmFadeOut(int fade_out_ms) {}

std::string TestSoundSystem::GetBgmName() const { return bgm_name_; }

bool TestSoundSystem::BgmLooping() const { return false; }

void TestSoundSystem::WavPlay(const std::string& wav_file, bool loop) {}

void TestSoundSystem::WavPlay(const std::string& wav_file,
                              bool loop,
                              const int channel) {}

void TestSoundSystem::WavPlay(const std::string& wav_file,
                              bool loop,
                              const int channel,
                              const int fadein_ms) {}

bool TestSoundSystem::WavPlaying(const int channel) {
  // TODO: Might want to do something about this eventually.
  return false;
}

void TestSoundSystem::WavStop(const int channel) {}

void TestSoundSystem::WavStopAll() {}

void TestSoundSystem::WavFadeOut(const int channel, const int fadetime) {}

void TestSoundSystem::PlaySe(const int se_num) {}

bool TestSoundSystem::HasSe(const int se_num) { return false; }

bool TestSoundSystem::KoePlaying() const { return false; }

void TestSoundSystem::KoeStop() {}

void TestSoundSystem::KoePlayImpl(int id) {}
