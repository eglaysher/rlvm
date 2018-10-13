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

#ifndef SRC_SYSTEMS_SDL_SDL_SOUND_CHUNK_H_
#define SRC_SYSTEMS_SDL_SDL_SOUND_CHUNK_H_

#include <boost/filesystem/operations.hpp>

#include <SDL2/SDL_mixer.h>

#include <map>
#include <memory>

// -----------------------------------------------------------------------

// Encapsulates a Mix_Chunk object. We do this so we can refcounting
// properly.
class SDLSoundChunk : public std::enable_shared_from_this<SDLSoundChunk> {
 public:
  // Builds a Mix_Chunk from a file.
  explicit SDLSoundChunk(const boost::filesystem::path& path);

  // Builds a Mix_Chunk from a chunk of memory.
  SDLSoundChunk(char* data, int length);

  virtual ~SDLSoundChunk();

  // Plays the chunk on the given channel. Wraps Mix_PlayChannel. Pass -1 to
  // |loops| for infinite loops.
  //
  // This method should always be used instead of Mix_PlayChannel with a raw
  // Mix_Chunk because we do additional bookkeeping to handle memory
  // deallocation for when a chunk finishes.
  void PlayChunkOn(int channel, int loops);

  // Fades a chunk in.
  void FadeInChunkOn(int channel, int loops, int ms);

  // SDL_Mixer callback function passed in to Mix_ChannelFinished().
  static void SoundChunkFinishedPlayback(int channel);

  static int FindNextFreeExtraChannel();

  static void StopChannel(int channel);
  static void StopAllChannels();

  static void FadeOut(const int channel, const int fadetime);

 private:
  // Used in the path constructor to actually create the Mix_Chunk, which
  // requires a hack for NWA support.
  Mix_Chunk* LoadSample(const boost::filesystem::path& path);

  // Static table which deliberately creates cycles. When a chunk
  // starts playing, it's associated with its channel ID in this table
  // to make sure that SDLSoundChunk object isn't deallocated. The
  // SDL_mixer callback, SoundChunkFinishedPlayback(), will reset the
  // associate smart pointer.
  typedef std::map<int, std::shared_ptr<SDLSoundChunk>> PlayingTable;
  static PlayingTable s_playing_table;

  // Wrapped chunk
  Mix_Chunk* sample_;

  // If this object was created from a memory chunk instead of a file, we have
  // to own the data that we pass to Mix_LoadWAV_RW(SDL_RWFromMem(...)).
  std::unique_ptr<char[]> data_;
};

// -----------------------------------------------------------------------

// Changes an incoming RealLive volume (0-256) to the range SDL_Mixer expects
// (0-128).
inline int realLiveVolumeToSDLMixerVolume(int in_vol) { return in_vol / 2; }

#endif  // SRC_SYSTEMS_SDL_SDL_SOUND_CHUNK_H_
