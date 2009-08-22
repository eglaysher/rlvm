// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2007 Elliot Glaysher
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

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

#include "Systems/SDL/SDLSoundChunk.hpp"

#include "Systems/Base/SoundSystem.hpp"
#include "Systems/SDL/SDLAudioLocker.hpp"

#include <SDL/SDL_mixer.h>

// -----------------------------------------------------------------------

SDLSoundChunk::PlayingTable SDLSoundChunk::s_playing_table;

// -----------------------------------------------------------------------

SDLSoundChunk::SDLSoundChunk(const boost::filesystem::path& path)
  : sample_(Mix_LoadWAV(path.external_file_string().c_str())) {
}

// -----------------------------------------------------------------------

SDLSoundChunk::SDLSoundChunk(char* data, int length)
    : sample_(Mix_LoadWAV_RW(SDL_RWFromMem(data, length+0x2c), 1)),
      data_(data) {
}

// -----------------------------------------------------------------------

SDLSoundChunk::~SDLSoundChunk() {
  Mix_FreeChunk(sample_);
  data_.reset();
}

// -----------------------------------------------------------------------

void SDLSoundChunk::playChunkOn(int channel, int loops) { {
    SDLAudioLocker locker;
    s_playing_table[channel] = shared_from_this();
  }

  if (Mix_PlayChannel(channel, sample_, loops) == -1) {
    // TODO: Throw something here.
  }
}

// -----------------------------------------------------------------------

void SDLSoundChunk::fadeInChunkOn(int channel, int loops, int ms) { {
    SDLAudioLocker locker;
    s_playing_table[channel] = shared_from_this();
  }

  if (Mix_FadeInChannel(channel, sample_, loops, ms) == -1) {
    // TODO: Throw something here.
  }
}

// -----------------------------------------------------------------------

// static
void SDLSoundChunk::SoundChunkFinishedPlayback(int channel) {
  // Don't need an SDLAudioLocker because we're in the audio callback right
  // now.
  //
  // Decrease the refcount of the SDLSoundChunk that just finished
  // playing.
  s_playing_table[channel].reset();
}

// -----------------------------------------------------------------------

// static
int SDLSoundChunk::FindNextFreeExtraChannel() {
  SDLAudioLocker locker;

  for (int i = NUM_BASE_CHANNELS;
      i < NUM_BASE_CHANNELS + NUM_EXTRA_WAVPLAY_CHANNELS; ++i) {
    if (s_playing_table[i].get() == 0)
      return i;
  }

  return -1;
}

// -----------------------------------------------------------------------

// static
void SDLSoundChunk::StopChannel(int channel) {
  Mix_HaltChannel(channel);
}

// -----------------------------------------------------------------------

void SDLSoundChunk::StopAllChannels() {
  Mix_HaltChannel(-1);
}

// -----------------------------------------------------------------------

void SDLSoundChunk::FadeOut(const int channel, const int fadetime) {
  Mix_FadeOutChannel(channel, fadetime);
}
