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

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

#include "Systems/SDL/SDLSoundSystem.hpp"
#include "Systems/SDL/SDLSoundChunk.hpp"
#include "Systems/SDL/SDLMusic.hpp"
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

boost::shared_ptr<SDLMusic> SDLSoundSystem::LoadMusic(
  RLMachine& machine, const std::string& bgmName)
{
  const DSTable& dsTable = getDSTable();
  DSTable::const_iterator dsIt = dsTable.find(bgmName);
  if(dsIt != dsTable.end())
    return SDLMusic::CreateMusic(machine, dsIt->second);

  const CDTable& cdTable = getCDTable();
  CDTable::const_iterator cdIt = cdTable.find(bgmName);
  if(cdIt != cdTable.end())
  {
    ostringstream oss;
    oss << "CD music not supported yet. Could not play track \"" 
        << bgmName << "\"";
    throw std::runtime_error(oss.str());
  }

  ostringstream oss;
  oss << "Could not find music track \"" << bgmName << "\"";
  throw std::runtime_error(oss.str());
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

  // Jagarl's sound system wants information on the audio settings.
	int freq, channels; Uint16 format;
	if(Mix_QuerySpec(&freq, &format, &channels) ) {
		WAVFILE::freq = freq;
		WAVFILE::format = format;
		WAVFILE::channels = channels;
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

void SDLSoundSystem::executeSoundSystem(RLMachine& machine)
{
  SoundSystem::executeSoundSystem(machine);

  if(m_queuedMusic && !SDLMusic::IsCurrentlyPlaying())
  {
    m_queuedMusic->fadeIn(m_queuedMusicLoop, m_queuedMusicFadein);
    m_queuedMusic.reset();
  }
}

// -----------------------------------------------------------------------

void SDLSoundSystem::setBgmEnabled(const int in)
{
  SDLMusic::SetBgmEnabled(in);
  SoundSystem::setBgmEnabled(in);
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

bool SDLSoundSystem::wavPlaying(RLMachine& machine, const int channel)
{
  checkChannel(channel, "SDLSoundSystem::wavPlaying");
  return Mix_Playing(channel);
}

// -----------------------------------------------------------------------

void SDLSoundSystem::wavStop(const int channel)
{
  checkChannel(channel, "SDLSoundSystem::wavStop");

  if(pcmEnabled())
  {
    SDLSoundChunk::StopChannel(channel);
  }
}

// -----------------------------------------------------------------------

void SDLSoundSystem::wavStopAll()
{
  if(pcmEnabled())
  {
    SDLSoundChunk::StopAllChannels();
  }
}

// -----------------------------------------------------------------------

void SDLSoundSystem::wavFadeOut(const int channel, const int fadetime)
{
  checkChannel(channel, "SDLSoundSystem::wavFadeOut");

  if(pcmEnabled())
    SDLSoundChunk::FadeOut(channel, fadetime);
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
    Mix_Volume(channel, realLiveVolumeToSDLMixerVolume(seVolume()));
    sample->playChunkOn(channel, 0);
  }
}

// -----------------------------------------------------------------------

int SDLSoundSystem::bgmStatus() const
{
  boost::shared_ptr<SDLMusic> currentlyPlaying = SDLMusic::CurrnetlyPlaying();
  if(currentlyPlaying) 
  {
    return currentlyPlaying->bgmStatus();
  }
  else
    return 0;
}

// -----------------------------------------------------------------------

void SDLSoundSystem::bgmPlay(RLMachine& machine, const std::string& bgmName, 
                             bool loop)
{
  boost::shared_ptr<SDLMusic> bgm = LoadMusic(machine, bgmName);
  bgm->play(loop);
}

// -----------------------------------------------------------------------

void SDLSoundSystem::bgmPlay(RLMachine& machine, const std::string& bgmName,
                             bool loop, int fadeInMs)
{
  boost::shared_ptr<SDLMusic> bgm = LoadMusic(machine, bgmName);
  bgm->fadeIn(loop, fadeInMs);
}

// -----------------------------------------------------------------------

void SDLSoundSystem::bgmPlay(RLMachine& machine, const std::string& bgmName,
                             bool loop, int fadeInMs, int fadeOutMs)
{
  m_queuedMusic = LoadMusic(machine, bgmName);
  m_queuedMusicLoop = loop;
  m_queuedMusicFadein = fadeInMs;
  
  bgmFadeOut(fadeOutMs);
}

// -----------------------------------------------------------------------

void SDLSoundSystem::bgmStop()
{
  boost::shared_ptr<SDLMusic> currentlyPlaying = SDLMusic::CurrnetlyPlaying();
  if(currentlyPlaying) 
    currentlyPlaying->stop();
}

// -----------------------------------------------------------------------

void SDLSoundSystem::bgmPause()
{
  boost::shared_ptr<SDLMusic> currentlyPlaying = SDLMusic::CurrnetlyPlaying();
  if(currentlyPlaying) 
    currentlyPlaying->pause();
}

// -----------------------------------------------------------------------

void SDLSoundSystem::bgmUnPause()
{
  boost::shared_ptr<SDLMusic> currentlyPlaying = SDLMusic::CurrnetlyPlaying();
  if(currentlyPlaying) 
    currentlyPlaying->unpause();
}

// -----------------------------------------------------------------------

void SDLSoundSystem::bgmFadeOut(int fadeOutMs) {
  boost::shared_ptr<SDLMusic> currentlyPlaying = SDLMusic::CurrnetlyPlaying();
  if(currentlyPlaying) 
    currentlyPlaying->fadeOut(fadeOutMs);
}

// -----------------------------------------------------------------------

std::string SDLSoundSystem::bgmName() const {
  boost::shared_ptr<SDLMusic> currentlyPlaying = SDLMusic::CurrnetlyPlaying();
  if(currentlyPlaying)
    return currentlyPlaying->name();
  else
    return "";
}

// -----------------------------------------------------------------------

void SDLSoundSystem::reset() {
  bgmStop();
  wavStopAll();

  SoundSystem::reset();
}
