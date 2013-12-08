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

#ifndef SRC_SYSTEMS_SDL_SDL_SOUND_SYSTEM_H_
#define SRC_SYSTEMS_SDL_SDL_SOUND_SYSTEM_H_

#include <boost/filesystem/operations.hpp>
#include <SDL2/SDL.h>

#include <memory>
#include <string>

#include "systems/base/sound_system.h"
#include "lru_cache.hpp"

class SDLSoundChunk;
class SDLMusic;

class SDLSoundSystem : public SoundSystem {
 public:
  explicit SDLSoundSystem(System& system);
  ~SDLSoundSystem();

  virtual void ExecuteSoundSystem() override;

  virtual void SetBgmEnabled(const int in) override;
  virtual void SetBgmVolumeMod(const int in) override;
  virtual void SetBgmVolumeScript(const int level, int fade_in_ms) override;
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

  virtual void SetChannelVolume(const int channel, const int level) override;

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

  virtual void Reset() override;

  // Wrapper around SDL_mixer's hook function. We do this because we need to
  // have our own default music mixing function which is set at startup.
  void SetMusicHook(void (*mix_func)(void* udata, Uint8* stream, int len));

 private:
  typedef std::shared_ptr<SDLSoundChunk> SDLSoundChunkPtr;
  typedef std::shared_ptr<SDLMusic> SDLMusicPtr;
  typedef LRUCache<std::string, SDLSoundChunkPtr> SoundChunkCache;

  virtual void KoePlayImpl(int id) override;

  // Retrieves a sound chunk from the passed in cache (or loads it if
  // it's not in the cache and then stuffs it into the cache.)
  SDLSoundChunkPtr GetSoundChunk(const std::string& file_name,
                                 SoundChunkCache& cache);

  // Builds a SoundChunk from a piece of memory. This is used for playing
  // voice. These chunks are not put in a SoundChunkCache since there's no
  // string to cache on.
  static SDLSoundChunkPtr BuildKoeChunk(char* data, int length);

  // Implementation to play a wave file. Two wavPlay() versions use this
  // underlying implementation, which is split out so the one that takes a raw
  // channel can verify its input.
  //
  // Both NUM_BASE_CHANNELS and NUM_EXTRA_WAVPLAY_CHANNELS are legal inputs for
  // |channel|.
  void WavPlayImpl(const std::string& wav_file, const int channel, bool loop);

  // Computes and passes a volume to SDL_mixer for |channel|.
  void SetChannelVolumeImpl(int channel);

  // Creates an SDLMusic object from a name. Throws if the bgm isn't
  // found.
  std::shared_ptr<SDLMusic> LoadMusic(const std::string& bgm_name);

  SoundChunkCache se_cache_;
  SoundChunkCache wav_cache_;

  // The music to play next as soon as the current track finishes.
  SDLMusicPtr queued_music_;

  // Whether the next piece of music loops
  bool queued_music_loop_;

  // The fadein time for queued piece of music
  int queued_music_fadein_;
};  // end of class SDLSoundSystem

#endif  // SRC_SYSTEMS_SDL_SDL_SOUND_SYSTEM_H_
