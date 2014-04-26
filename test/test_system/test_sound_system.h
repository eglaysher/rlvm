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

#ifndef TEST_TEST_SYSTEM_TEST_SOUND_SYSTEM_H_
#define TEST_TEST_SYSTEM_TEST_SOUND_SYSTEM_H_

#include "systems/base/sound_system.h"
#include <string>

class Gameexe;

// Test sound system
class TestSoundSystem : public SoundSystem {
 public:
  explicit TestSoundSystem(System& system);
  virtual ~TestSoundSystem();

  virtual int BgmStatus() const override;

  virtual void BgmPlay(const std::string& bgm_name, bool loop) override;
  virtual void BgmPlay(const std::string& bgm_name, bool loop,
                       int fade_in_ms) override;
  virtual void BgmPlay(const std::string& bgm_name,
                       bool loop,
                       int fade_in_ms,
                       int fade_out_ms) override;
  virtual void BgmStop() override;
  virtual void BgmPause() override;
  virtual void BgmUnPause() override;
  virtual void BgmFadeOut(int fade_out_ms) override;
  virtual std::string GetBgmName() const override;
  virtual bool BgmLooping() const override;

  virtual void WavPlay(const std::string& wav_file, bool loop) override;
  virtual void WavPlay(const std::string& wav_file,
                       bool loop,
                       const int channel) override;
  virtual void WavPlay(const std::string& wav_file,
                       bool loop,
                       const int channel,
                       const int fadein_ms) override;
  virtual bool WavPlaying(const int channel) override;
  virtual void WavStop(const int channel) override;
  virtual void WavStopAll() override;
  virtual void WavFadeOut(const int channel, const int fadetime) override;

  virtual void PlaySe(const int se_num) override;
  virtual bool HasSe(const int se_num) override;

  virtual bool KoePlaying() const override;
  virtual void KoeStop() override;

 private:
  virtual void KoePlayImpl(int id) override;

  std::string bgm_name_;
};  // end of class TestSoundSystem

#endif  // TEST_TEST_SYSTEM_TEST_SOUND_SYSTEM_H_
