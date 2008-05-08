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
// the Free Software Foundation; either version 2 of the License, or
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
#include <boost/shared_ptr.hpp>
#include <boost/filesystem/operations.hpp>

#include "lru_cache.hpp"
#include <string>

// -----------------------------------------------------------------------

class SDLSoundChunk;
class SDLMusic;

// -----------------------------------------------------------------------

/**
 *
 */
class SDLSoundSystem : public SoundSystem
{
private:
  typedef boost::shared_ptr<SDLSoundChunk> SDLSoundChunkPtr;
  typedef boost::shared_ptr<SDLMusic> SDLMusicPtr;

  typedef LRUCache<boost::filesystem::path, 
                   SDLSoundChunkPtr> SoundChunkCache;

  SoundChunkCache m_seCache;
  SoundChunkCache m_wavCache;

  /// The 
  SDLMusicPtr m_queuedMusic;

  /// Whether the next piece of music loops
  bool m_queuedMusicLoop;

  /// The fadein time for queued piece of music
  int m_queuedMusicFadein;

  /** 
   * Retrieves a sound chunk from the passed in cache (or loads it if
   * it's not in the cache and then stuffs it into the cache.)
   * 
   * @param machine Current machine context
   * @param fileName Name of the file (minus extension)
   * @param cache Which cache to check (and store) the 
   * @return The loaded sound chunk
   */
  static SDLSoundChunkPtr getSoundChunk(
    RLMachine& machine, 
    const std::string& fileName, 
    SoundChunkCache& cache);

  /** 
   * Implementation to play a wave file. Two wavPlay() versions use
   * this underlying implementation, which is split out so the one
   * that takes a raw channel can verify its input.
   * 
   * @param machine Current machine context
   * @param wavFile Name of the file (minux extension)
   * @param channel Channel to play on (both NUM_BASE_CHANNELS and
   *                NUM_EXTRA_WAVPLAY_CHANNELS are legal here.)
   * @param loop    Whether to loop this sound endlessly
   */
  void wavPlayImpl(RLMachine& machine, const std::string& wavFile,
                   const int channel, bool loop);

  /** 
   * Creates an SDLMusic object from a name. Throws if the bgm isn't
   * found.
   */
  boost::shared_ptr<SDLMusic> LoadMusic(
    RLMachine& machine, const std::string& bgmName);

public: 
  SDLSoundSystem(Gameexe& gexe);
  ~SDLSoundSystem();

  virtual void executeSoundSystem(RLMachine& machine);

  virtual void setBgmEnabled(const int in);
  virtual void setChannelVolume(const int channel, const int level);

  virtual void wavPlay(RLMachine& machine, const std::string& wavFile, bool loop);
  virtual void wavPlay(RLMachine& machine, const std::string& wavFile, bool loop,
                       const int channel);
  virtual void wavPlay(RLMachine& machine, const std::string& wavFile, bool loop,
                       const int channel, const int fadeinMs);
  virtual void wavStop(const int channel);
  virtual void wavStopAll();
  virtual void wavFadeOut(const int channel, const int fadetime);

  virtual void playSe(RLMachine& machine, const int seNum);  

  virtual void bgmPlay(RLMachine& machine, const std::string& bgmName, bool loop);
  virtual void bgmPlay(RLMachine& machine, const std::string& bgmName, bool loop,
                       int fadeInMs);
  virtual void bgmPlay(RLMachine& machine, const std::string& bgmName, bool loop,
                       int fadeInMs, int fadeOutMs);
  virtual void bgmFadeOut(int fadeOutMs);
};	// end of class SDLSoundSystem

#endif
