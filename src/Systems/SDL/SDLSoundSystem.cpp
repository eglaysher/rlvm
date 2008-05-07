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
// RealLive Sound Qualities table
// -----------------------------------------------------------------------
struct RealLiveSoundQualities {
  int rate;
  Uint16 format;
};

/**
 * A maping between SoundQualities() and the values need to be passed
 * to Mix_OpenAudio()
 */
static RealLiveSoundQualities s_realLiveSoundQualities[] = {
  {11025, AUDIO_S8},    // 11 kHz, 8 bit stereo
  {11025, AUDIO_S16},   // 11 kHz, 16 bit stereo
  {22050, AUDIO_S8},    // 22 kHz, 8 bit stereo
  {22050, AUDIO_S16},   // 22 kHz, 16 bit stereo
  {44100, AUDIO_S8},    // 44 kHz, 8 bit stereo
  {44100, AUDIO_S16},   // 44 kHz, 16 bit stereo
  {48000, AUDIO_S8},    // 48 kHz, 8 bit stereo
  {48000, AUDIO_S16}    // 48 hKz, 16 bit stereo
};


// -----------------------------------------------------------------------
// SDLSoundSystem (private)
// -----------------------------------------------------------------------
SDLSoundSystem::SDLSoundChunkPtr SDLSoundSystem::getSoundChunk(
  RLMachine& machine, const std::string& fileName, SoundChunkCache& cache)
{
  fs::path filePath = findFile(machine, fileName, SOUND_FILETYPES);
  SDLSoundChunkPtr sample = cache.fetch(filePath);
  if(sample == NULL)
  {
    sample.reset(new SDLSoundChunk(filePath));
    cache.insert(filePath, sample);
  }

  return sample;
}

// -----------------------------------------------------------------------

void SDLSoundSystem::wavPlayImpl(
  RLMachine& machine, const std::string& wavFile, const int channel, bool loop)
{
  if(pcmEnabled())
  {
    SDLSoundChunkPtr sample = getSoundChunk(machine, wavFile, m_wavCache);
    Mix_Volume(channel, realLiveVolumeToSDLMixerVolume(pcmVolume()));
    int loopNum = loop ? -1 : 0;
    sample->playChunkOn(channel, loopNum);
  }  
}

// -----------------------------------------------------------------------
// SDLSoundSystem
// -----------------------------------------------------------------------
SDLSoundSystem::SDLSoundSystem(Gameexe& gexe)
  : SoundSystem(gexe), m_seCache(5), m_wavCache(5)
{
  SDL_InitSubSystem(SDL_INIT_AUDIO);

  /* We're going to be requesting certain things from our audio
     device, so we set them up beforehand */

  int audio_rate = s_realLiveSoundQualities[soundQuality()].rate;
  Uint16 audio_format = s_realLiveSoundQualities[soundQuality()].format;
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

void SDLSoundSystem::wavPlay(RLMachine& machine, const std::string& wavFile, bool loop)
{
  int channelNumber = SDLSoundChunk::FindNextFreeExtraChannel();
  if(channelNumber == -1)
  {
    ostringstream oss;
    oss << "Couldn't find a free channel for wavPlay()";
    throw std::runtime_error(oss.str());
  }

  wavPlayImpl(machine, wavFile, channelNumber, loop);
}

// -----------------------------------------------------------------------

void SDLSoundSystem::wavPlay(RLMachine& machine, const std::string& wavFile,
                             bool loop, const int channel)
{
  checkChannel(channel, "SDLSoundSystem::wavPlay");
  wavPlayImpl(machine, wavFile, channel, loop);
}

// -----------------------------------------------------------------------

void SDLSoundSystem::wavPlay(RLMachine& machine, const std::string& wavFile,
                             bool loop, const int channel, const int fadeinMs)
{
  checkChannel(channel, "SDLSoundSystem::wavPlay");

  if(pcmEnabled())
  {
    SDLSoundChunkPtr sample = getSoundChunk(machine, wavFile, m_wavCache);
    Mix_Volume(channel, realLiveVolumeToSDLMixerVolume(pcmVolume()));

    int loopNum = loop ? -1 : 0;
    sample->fadeInChunkOn(channel, loopNum, fadeinMs);
  }
}

// -----------------------------------------------------------------------

void SDLSoundSystem::playSe(RLMachine& machine, const int seNum)
{
  if(seEnabled())
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

    // Make sure there isn't anything playing on the current channel
    Mix_HaltChannel(channel);

    if(fileName == "")
    {
      // Just stop a channel in case of an empty file name.
      return;
    }

    SDLSoundChunkPtr sample = getSoundChunk(machine, fileName, m_wavCache);

    // SE chunks have no per channel volume...
    Mix_Volume(channel, realLiveVolumeToSDLMixerVolume(pcmVolume()));
    sample->playChunkOn(channel, 0);
  }
}

// -----------------------------------------------------------------------
