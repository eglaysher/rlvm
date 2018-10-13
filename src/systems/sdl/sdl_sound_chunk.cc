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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
//
// -----------------------------------------------------------------------

#include "systems/sdl/sdl_sound_chunk.h"

#include <SDL2/SDL_mixer.h>
#include <boost/algorithm/string.hpp>
#include <string>

#include "systems/base/sound_system.h"
#include "systems/sdl/sdl_audio_locker.h"
#include "xclannad/wavfile.h"

SDLSoundChunk::PlayingTable SDLSoundChunk::s_playing_table;

SDLSoundChunk::SDLSoundChunk(const boost::filesystem::path& path)
    : sample_(LoadSample(path)) {}

SDLSoundChunk::SDLSoundChunk(char* data, int length)
    : sample_(Mix_LoadWAV_RW(SDL_RWFromMem(data, length + 0x2c), 1)),
      data_(data) {}

SDLSoundChunk::~SDLSoundChunk() {
  Mix_FreeChunk(sample_);
  data_.reset();
}

Mix_Chunk* SDLSoundChunk::LoadSample(const boost::filesystem::path& path) {
  if (boost::iequals(path.extension().string(), ".nwa")) {
    // Hack to load NWA sounds into a MixChunk. I was resisted doing this
    // because I assumed there was a better way, but this is essentially what
    // jagarl does in xclannad too :(
    FILE* f = fopen(path.native().c_str(), "r");
    if (!f)
      return NULL;
    int size = 0;
    char* data = NWAFILE::ReadAll(f, size);
    fclose(f);

    Mix_Chunk* chunk = Mix_LoadWAV_RW(SDL_RWFromMem(data, size), 1);
    delete[] data;

    return chunk;
  } else {
    return Mix_LoadWAV(path.native().c_str());
  }
}

void SDLSoundChunk::PlayChunkOn(int channel, int loops) {
  {
    SDLAudioLocker locker;
    s_playing_table[channel] = shared_from_this();
  }

  if (Mix_PlayChannel(channel, sample_, loops) == -1) {
    // TODO(erg): Throw something here.
  }
}

void SDLSoundChunk::FadeInChunkOn(int channel, int loops, int ms) {
  {
    SDLAudioLocker locker;
    s_playing_table[channel] = shared_from_this();
  }

  if (Mix_FadeInChannel(channel, sample_, loops, ms) == -1) {
    // TODO(erg): Throw something here.
  }
}

// static
void SDLSoundChunk::SoundChunkFinishedPlayback(int channel) {
  // Don't need an SDLAudioLocker because we're in the audio callback right
  // now.
  //
  // Decrease the refcount of the SDLSoundChunk that just finished
  // playing.
  s_playing_table[channel].reset();
}

// static
int SDLSoundChunk::FindNextFreeExtraChannel() {
  SDLAudioLocker locker;

  for (int i = NUM_BASE_CHANNELS;
       i < NUM_BASE_CHANNELS + NUM_EXTRA_WAVPLAY_CHANNELS;
       ++i) {
    if (s_playing_table[i].get() == 0)
      return i;
  }

  return -1;
}

// static
void SDLSoundChunk::StopChannel(int channel) { Mix_HaltChannel(channel); }

void SDLSoundChunk::StopAllChannels() { Mix_HaltChannel(-1); }

void SDLSoundChunk::FadeOut(const int channel, const int fadetime) {
  Mix_FadeOutChannel(channel, fadetime);
}
