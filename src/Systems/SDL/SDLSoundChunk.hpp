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

#ifndef __SDLSoundChunk_hpp__
#define __SDLSoundChunk_hpp__

#include <map>

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/filesystem/operations.hpp>

#include <SDL/SDL_mixer.h>

// -----------------------------------------------------------------------

/**
 * Encapsulates a Mix_Chunk object. We do this so we can refcounting
 * properly.
 */
class SDLSoundChunk : public boost::enable_shared_from_this<SDLSoundChunk>
{
private:
  /**
   * Static table which deliberatly creates cycles. When a chunk
   * starts playing, it's associated with its channel ID in this table
   * to make sure that SDLSoundChunk object isn't deallocated. The
   * SDL_mixer callback, SoundChunkFinishedPlayback(), will reset the
   * associate smart pointer.
   */
  typedef std::map<int, boost::shared_ptr<SDLSoundChunk> > PlayingTable;
  static PlayingTable s_playingTable;

  /// Sample
  Mix_Chunk* m_sample;

public:
  SDLSoundChunk(const boost::filesystem::path& path);

  /** 
   * Plays the chunk on the given channel. Wraps Mix_PlayChannel.
   * 
   * @param channel Channel number to use.
   * @param loops 
   * @note This method should always be used instead of
   *       Mix_PlayChannel with a raw Mix_Chunk because we do
   *       additional bookkeeping to handle memory deallocation for
   *       when a chunk finishes.
   */
  void playChunkOn(int channel, int loops);

  /** 
   * SDL_Mixer callback function passed in to Mix_ChannelFinished().
   * 
   * @param channel Channel that just finished playing.
   */
  static void SoundChunkFinishedPlayback(int channel);

  static int FindNextFreeExtraChannel();
};

// -----------------------------------------------------------------------

/**
 * Changes an incoming RealLive volume to the range SDL_Mixer expects.
 *
 * @param inVol RealLive volume (ranged 0-256)
 * @return SDL_Mixer volume (ranged 0-128)
 */
inline int realLiveVolumeToSDLMixerVolume(int inVol)
{
  return inVol / 2;
}

#endif
