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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// -----------------------------------------------------------------------

#ifndef __SDLSoundSystem_hpp__
#define __SDLSoundSystem_hpp__

#include "Systems/Base/SoundSystem.hpp"
#include "lru_cache.hpp"

#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem/operations.hpp>
#include <SDL/SDL.h>

// -----------------------------------------------------------------------

class SDLSoundChunk;
class SDLMusic;

// -----------------------------------------------------------------------

/**
 *
 */
class SDLSoundSystem : public SoundSystem
{
public:
  SDLSoundSystem(System& system);
  ~SDLSoundSystem();

  virtual void executeSoundSystem();

  virtual void setBgmEnabled(const int in);
  virtual void setChannelVolume(const int channel, const int level);

  virtual void wavPlay(const std::string& wav_file, bool loop);
  virtual void wavPlay(const std::string& wav_file, bool loop,
                       const int channel);
  virtual void wavPlay(const std::string& wav_file, bool loop,
                       const int channel, const int fadein_ms);
  virtual bool wavPlaying(const int channel);
  virtual void wavStop(const int channel);
  virtual void wavStopAll();
  virtual void wavFadeOut(const int channel, const int fadetime);

  virtual void playSe(const int se_num);

  virtual int bgmStatus() const;
  virtual void bgmPlay(const std::string& bgm_name, bool loop);
  virtual void bgmPlay(const std::string& bgm_name, bool loop,
                       int fade_in_ms);
  virtual void bgmPlay(const std::string& bgm_name, bool loop,
                       int fade_in_ms, int fade_out_ms);
  virtual void bgmStop();
  virtual void bgmPause();
  virtual void bgmUnPause();
  virtual void bgmFadeOut(int fade_out_ms);
  virtual std::string bgmName() const;
  virtual bool bgmLooping() const;

  virtual void reset();

  /**
   * Wrapper around SDL_mixer's hook function. We do this because we need to
   * have our own default music mixing function which is set at startup.
   */
  void setMusicHook(void (*mix_func)(void *udata, Uint8 *stream, int len));

private:
  typedef boost::shared_ptr<SDLSoundChunk> SDLSoundChunkPtr;
  typedef boost::shared_ptr<SDLMusic> SDLMusicPtr;
  typedef LRUCache<std::string, SDLSoundChunkPtr> SoundChunkCache;

  virtual void koePlayImpl(int id);

  /**
   * Retrieves a sound chunk from the passed in cache (or loads it if
   * it's not in the cache and then stuffs it into the cache.)
   *
   * @param file_name Name of the file (minus extension)
   * @param cache Which cache to check (and store) the
   * @return The loaded sound chunk
   */
  SDLSoundChunkPtr getSoundChunk(
    const std::string& file_name,
    SoundChunkCache& cache);

  /**
   * Builds a SoundChunk from a piece of memory. This is used for playing
   * voice. These chunks are not put in a SoundChunkCache since there's no
   * string to cache on.
   */
  static SDLSoundChunkPtr buildKoeChunk(char* data, int length);

  /**
   * Implementation to play a wave file. Two wavPlay() versions use
   * this underlying implementation, which is split out so the one
   * that takes a raw channel can verify its input.
   *
   * @param wav_file Name of the file (minux extension)
   * @param channel Channel to play on (both NUM_BASE_CHANNELS and
   *                NUM_EXTRA_WAVPLAY_CHANNELS are legal here.)
   * @param loop    Whether to loop this sound endlessly
   */
  void wavPlayImpl(const std::string& wav_file,
                   const int channel, bool loop);

  /**
   * Creates an SDLMusic object from a name. Throws if the bgm isn't
   * found.
   */
  boost::shared_ptr<SDLMusic> LoadMusic(const std::string& bgm_name);

  SoundChunkCache se_cache_;
  SoundChunkCache wav_cache_;

  /// The
  SDLMusicPtr queued_music_;

  /// Whether the next piece of music loops
  bool queued_music_loop_;

  /// The fadein time for queued piece of music
  int queued_music_fadein_;
};	// end of class SDLSoundSystem

#endif
