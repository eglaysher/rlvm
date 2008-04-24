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

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

#include "Systems/SDL/SDLSoundSystem.hpp"
#include "Systems/SDL/SDLSoundChunk.hpp"
#include "Systems/Base/SystemError.hpp"
#include "Utilities.h"

#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <sstream>

using boost::shared_ptr;
using namespace std;
namespace fs = boost::filesystem;

// -----------------------------------------------------------------------

SDLSoundSystem::SDLSoundSystem(Gameexe& gexe)
  : SoundSystem(gexe), m_seCache(5), m_wavCache(5)
{
  SDL_InitSubSystem(SDL_INIT_AUDIO);

  /* We're going to be requesting certain things from our audio
     device, so we set them up beforehand */
  int audio_rate = 22050;
  Uint16 audio_format = AUDIO_S16; /* 16-bit stereo */
  int audio_channels = 2;
  int audio_buffers = 4096;

  /* This is where we open up our audio device.  Mix_OpenAudio takes
     as its parameters the audio format we'd /like/ to have. */
  if(Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers)) {
    throw SystemError("Couldn't initialize audio");
  }

  Mix_AllocateChannels(NUM_BASE_CHANNELS + NUM_EXTRA_WAVPLAY_CHANNELS);

  Mix_ChannelFinished(&SDLSoundChunk::SoundChunkFinishedPlayback);
}

// -----------------------------------------------------------------------

SDLSoundSystem::~SDLSoundSystem()
{
  Mix_CloseAudio();
  SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

// -----------------------------------------------------------------------

void SDLSoundSystem::setChannelVolume(const int channel, const int level)
{
  int adjustedVolume = computeChannelVolume(channel, pcmVolume());
  Mix_Volume(realLiveVolumeToSDLMixerVolume(adjustedVolume), level);
}

// -----------------------------------------------------------------------

void SDLSoundSystem::wavPlay(RLMachine& machine, const std::string& wavFile)
{
  fs::path filePath = findFile(machine, wavFile, SOUND_FILETYPES);
  
  // Find the next free channel
  shared_ptr<SDLSoundChunk> sample = m_wavCache.fetch(filePath);
  if(sample == NULL)
  {
    sample.reset(new SDLSoundChunk(filePath));
    m_seCache.insert(filePath, sample);
  }

  int channelNumber = SDLSoundChunk::FindNextFreeExtraChannel();
  if(channelNumber == -1)
  {
    ostringstream oss;
    oss << "Couldn't find a free channel for wavPlay()";
    throw std::runtime_error(oss.str());
  }

  Mix_Volume(channelNumber, realLiveVolumeToSDLMixerVolume(pcmVolume()));
  sample->playChunkOn(channelNumber, 0);
}

// -----------------------------------------------------------------------

void SDLSoundSystem::playSe(RLMachine& machine, const int seNum)
{
  SeTable::const_iterator it = seTable().find(seNum);
  if(it == seTable().end())
  {
    ostringstream oss;
    oss << "No #SE entry found for sound effect number " << seNum;
    throw rlvm::Exception(oss.str());
  }

  const string& fileName = it->second.first;
  int channel = it->second.second;

  if(fileName == "")
  {
    // Just stop a channel in case of an empty file name.
    Mix_HaltChannel(channel);
    return;
  }

  fs::path filePath = findFile(machine, fileName, SOUND_FILETYPES);

  // Make sure there isn't anything playing on the current channel
  Mix_HaltChannel(channel);

  shared_ptr<SDLSoundChunk> sample = m_seCache.fetch(filePath);
  if(sample == NULL)
  {
    sample.reset(new SDLSoundChunk(filePath));
    m_seCache.insert(filePath, sample);
  }

  // SE chunks have no per channel volume...
  Mix_Volume(channel, realLiveVolumeToSDLMixerVolume(pcmVolume()));
  sample->playChunkOn(channel, 0);
}

// -----------------------------------------------------------------------
