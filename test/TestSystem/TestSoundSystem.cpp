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

#include "TestSystem/TestSoundSystem.hpp"

#include <string>

// -----------------------------------------------------------------------
// TestSoundSystem
// -----------------------------------------------------------------------

TestSoundSystem::TestSoundSystem(System& system)
    : SoundSystem(system) {
}

TestSoundSystem::~TestSoundSystem() {}

int TestSoundSystem::bgmStatus() const {
  return 0;
}

void TestSoundSystem::bgmPlay(const std::string& bgm_name, bool loop) {
  bgm_name_ = bgm_name;
}

void TestSoundSystem::bgmPlay(const std::string& bgm_name,
                              bool loop, int fade_in_ms) {
  bgm_name_ = bgm_name;
}

void TestSoundSystem::bgmPlay(const std::string& bgm_name, bool loop,
                              int fade_in_ms, int fade_out_ms) {
  bgm_name_ = bgm_name;
}

void TestSoundSystem::bgmStop() {
}

void TestSoundSystem::bgmPause() {
}

void TestSoundSystem::bgmUnPause() {
}

void TestSoundSystem::bgmFadeOut(int fade_out_ms) {
}

std::string TestSoundSystem::bgmName() const {
  return bgm_name_;
}

bool TestSoundSystem::bgmLooping() const {
  return false;
}

void TestSoundSystem::wavPlay(const std::string& wav_file, bool loop) {
}

void TestSoundSystem::wavPlay(const std::string& wav_file, bool loop,
                              const int channel) {
}

void TestSoundSystem::wavPlay(const std::string& wav_file, bool loop,
                              const int channel, const int fadein_ms) {
}

bool TestSoundSystem::wavPlaying(const int channel) {
  // TODO: Might want to do something about this eventually.
  return false;
}

void TestSoundSystem::wavStop(const int channel) {
}

void TestSoundSystem::wavStopAll() {
}

void TestSoundSystem::wavFadeOut(const int channel, const int fadetime) {
}

void TestSoundSystem::playSe(const int se_num) {
}

bool TestSoundSystem::hasSe(const int se_num) {
  return false;
}

bool TestSoundSystem::koePlaying() const {
  return false;
}

void TestSoundSystem::koeStop() {
}

void TestSoundSystem::koePlayImpl(int id) {
}

