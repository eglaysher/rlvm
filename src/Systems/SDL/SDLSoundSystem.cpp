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

#include "Systems/SDL/SDLSoundSystem.hpp"

#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <sstream>
#include <string>

#include "Systems/Base/SystemError.hpp"
#include "Systems/Base/VoiceArchive.hpp"
#include "Systems/SDL/SDLMusic.hpp"
#include "Systems/SDL/SDLSoundChunk.hpp"
#include "Utilities/Exception.hpp"
#include "Utilities/File.hpp"

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
static RealLiveSoundQualities s_real_live_sound_qualities[] = {
  {11025, AUDIO_S8},    // 11 k_hz, 8 bit stereo
  {11025, AUDIO_S16},   // 11 k_hz, 16 bit stereo
  {22050, AUDIO_S8},    // 22 k_hz, 8 bit stereo
  {22050, AUDIO_S16},   // 22 k_hz, 16 bit stereo
  {44100, AUDIO_S8},    // 44 k_hz, 8 bit stereo
  {44100, AUDIO_S16},   // 44 k_hz, 16 bit stereo
  {48000, AUDIO_S8},    // 48 k_hz, 8 bit stereo
  {48000, AUDIO_S16}    // 48 h_kz, 16 bit stereo
};

// -----------------------------------------------------------------------
// SDLSoundSystem (private)
// -----------------------------------------------------------------------
SDLSoundSystem::SDLSoundChunkPtr SDLSoundSystem::getSoundChunk(
    const std::string& file_name, SoundChunkCache& cache) {
  SDLSoundChunkPtr sample = cache.fetch(file_name);
  if (sample == NULL) {
    fs::path file_path = findFile(system(), file_name, SOUND_FILETYPES);
    if (file_path.empty()) {
      ostringstream oss;
      oss << "Could not find sound file \"" << file_name << "\".";
      throw rlvm::Exception(oss.str());
    }

    sample.reset(new SDLSoundChunk(file_path));
    cache.insert(file_name, sample);
  }

  return sample;
}

// -----------------------------------------------------------------------

SDLSoundSystem::SDLSoundChunkPtr SDLSoundSystem::buildKoeChunk(
    char* data, int length) {
  return SDLSoundChunkPtr(new SDLSoundChunk(data, length));
}

// -----------------------------------------------------------------------

void SDLSoundSystem::wavPlayImpl(const std::string& wav_file,
                                 const int channel, bool loop) {
  if (pcmEnabled()) {
    SDLSoundChunkPtr sample = getSoundChunk(wav_file, wav_cache_);
    setChannelVolumeImpl(channel);
    int loop_num = loop ? -1 : 0;
    sample->playChunkOn(channel, loop_num);
  }
}

// -----------------------------------------------------------------------

void SDLSoundSystem::setChannelVolumeImpl(int channel) {
  int adjusted = computeChannelVolume(channelVolume(channel), pcmVolumeMod());
  Mix_Volume(channel, realLiveVolumeToSDLMixerVolume(adjusted));
}

// -----------------------------------------------------------------------

boost::shared_ptr<SDLMusic> SDLSoundSystem::LoadMusic(
    const std::string& bgm_name) {
  const DSTable& ds_table = getDSTable();
  DSTable::const_iterator ds_it = ds_table.find(boost::to_lower_copy(bgm_name));
  if (ds_it != ds_table.end())
    return SDLMusic::CreateMusic(system(), ds_it->second);

  const CDTable& cd_table = getCDTable();
  CDTable::const_iterator cd_it = cd_table.find(boost::to_lower_copy(bgm_name));
  if (cd_it != cd_table.end()) {
    ostringstream oss;
    oss << "CD music not supported yet. Could not play track \""
        << bgm_name << "\"";
    throw std::runtime_error(oss.str());
  }

  ostringstream oss;
  oss << "Could not find music track \"" << bgm_name << "\"";
  throw std::runtime_error(oss.str());
}

// -----------------------------------------------------------------------
// SDLSoundSystem
// -----------------------------------------------------------------------
SDLSoundSystem::SDLSoundSystem(System& system)
  : SoundSystem(system), se_cache_(5), wav_cache_(5) {
  SDL_InitSubSystem(SDL_INIT_AUDIO);

  /* We're going to be requesting certain things from our audio
     device, so we set them up beforehand */

  int audio_rate = s_real_live_sound_qualities[soundQuality()].rate;
  Uint16 audio_format = s_real_live_sound_qualities[soundQuality()].format;
  int audio_channels = 2;
  int audio_buffers = 4096;

  /* This is where we open up our audio device.  Mix_OpenAudio takes
     as its parameters the audio format we'd /like/ to have. */
  if (Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers)) {
    throw SystemError("Couldn't initialize audio");
  }

  // Jagarl's sound system wants information on the audio settings.
  int freq, channels; Uint16 format;
  if (Mix_QuerySpec(&freq, &format, &channels)) {
    WAVFILE::freq = freq;
    WAVFILE::format = format;
    WAVFILE::channels = channels;
  }

  Mix_AllocateChannels(NUM_TOTAL_CHANNELS);

  Mix_ChannelFinished(&SDLSoundChunk::SoundChunkFinishedPlayback);

  setMusicHook(NULL);
}

// -----------------------------------------------------------------------

SDLSoundSystem::~SDLSoundSystem() {
  Mix_HookMusic(NULL, NULL);

  Mix_CloseAudio();
  SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

// -----------------------------------------------------------------------

void SDLSoundSystem::executeSoundSystem() {
  SoundSystem::executeSoundSystem();

  if (queued_music_ && !SDLMusic::IsCurrentlyPlaying()) {
    queued_music_->fadeIn(queued_music_loop_, queued_music_fadein_);
    queued_music_.reset();
  }
}

// -----------------------------------------------------------------------

void SDLSoundSystem::setBgmEnabled(const int in) {
  SDLMusic::SetBgmEnabled(in);
  SoundSystem::setBgmEnabled(in);
}

// -----------------------------------------------------------------------

void SDLSoundSystem::setBgmVolumeMod(const int in) {
  SoundSystem::setBgmVolumeMod(in);
  SDLMusic::SetComputedBgmVolume(computeChannelVolume(in, bgmVolumeScript()));
}

// -----------------------------------------------------------------------

void SDLSoundSystem::setBgmVolumeScript(const int in) {
  SoundSystem::setBgmVolumeScript(in);
  SDLMusic::SetComputedBgmVolume(computeChannelVolume(bgmVolumeMod(), in));
}

// -----------------------------------------------------------------------

void SDLSoundSystem::setChannelVolume(const int channel, const int level) {
  SoundSystem::setChannelVolume(channel, level);
  setChannelVolumeImpl(channel);
}

// -----------------------------------------------------------------------

void SDLSoundSystem::wavPlay(const std::string& wav_file, bool loop) {
  int channel_number = SDLSoundChunk::FindNextFreeExtraChannel();
  if (channel_number == -1) {
    ostringstream oss;
    oss << "Couldn't find a free channel for wavPlay()";
    throw std::runtime_error(oss.str());
  }

  wavPlayImpl(wav_file, channel_number, loop);
}

// -----------------------------------------------------------------------

void SDLSoundSystem::wavPlay(const std::string& wav_file,
                             bool loop, const int channel) {
  checkChannel(channel, "SDLSoundSystem::wav_play");
  wavPlayImpl(wav_file, channel, loop);
}

// -----------------------------------------------------------------------

void SDLSoundSystem::wavPlay(const std::string& wav_file, bool loop,
                             const int channel, const int fadein_ms) {
  checkChannel(channel, "SDLSoundSystem::wav_play");

  if (pcmEnabled()) {
    SDLSoundChunkPtr sample = getSoundChunk(wav_file, wav_cache_);
    setChannelVolumeImpl(channel);

    int loop_num = loop ? -1 : 0;
    sample->fadeInChunkOn(channel, loop_num, fadein_ms);
  }
}

// -----------------------------------------------------------------------

bool SDLSoundSystem::wavPlaying(const int channel) {
  checkChannel(channel, "SDLSoundSystem::wav_playing");
  return Mix_Playing(channel);
}

// -----------------------------------------------------------------------

void SDLSoundSystem::wavStop(const int channel) {
  checkChannel(channel, "SDLSoundSystem::wav_stop");

  if (pcmEnabled()) {
    SDLSoundChunk::StopChannel(channel);
  }
}

// -----------------------------------------------------------------------

void SDLSoundSystem::wavStopAll() {
  if (pcmEnabled()) {
    SDLSoundChunk::StopAllChannels();
  }
}

// -----------------------------------------------------------------------

void SDLSoundSystem::wavFadeOut(const int channel, const int fadetime) {
  checkChannel(channel, "SDLSoundSystem::wav_fade_out");

  if (pcmEnabled())
    SDLSoundChunk::FadeOut(channel, fadetime);
}

// -----------------------------------------------------------------------

void SDLSoundSystem::playSe(const int se_num) {
  if (seEnabled()) {
    SeTable::const_iterator it = seTable().find(se_num);
    if (it == seTable().end()) {
      ostringstream oss;
      oss << "No #SE entry found for sound effect number " << se_num;
      throw rlvm::Exception(oss.str());
    }

    const string& file_name = it->second.first;
    int channel = it->second.second;

    // Make sure there isn't anything playing on the current channel
    Mix_HaltChannel(channel);

    if (file_name == "") {
      // Just stop a channel in case of an empty file name.
      return;
    }

    SDLSoundChunkPtr sample = getSoundChunk(file_name, wav_cache_);

    // SE chunks have no per channel volume...
    Mix_Volume(channel, realLiveVolumeToSDLMixerVolume(seVolume()));
    sample->playChunkOn(channel, 0);
  }
}

// -----------------------------------------------------------------------

int SDLSoundSystem::bgmStatus() const {
  boost::shared_ptr<SDLMusic> currently_playing = SDLMusic::CurrnetlyPlaying();
  if (currently_playing) {
    return currently_playing->bgmStatus();
  } else {
    return 0;
  }
}

// -----------------------------------------------------------------------

void SDLSoundSystem::bgmPlay(const std::string& bgm_name,
                             bool loop) {
  if (!boost::iequals(bgmName(), bgm_name)) {
    boost::shared_ptr<SDLMusic> bgm = LoadMusic(bgm_name);
    bgm->play(loop);
  }
}

// -----------------------------------------------------------------------

void SDLSoundSystem::bgmPlay(const std::string& bgm_name,
                             bool loop, int fade_in_ms) {
  if (!boost::iequals(bgmName(), bgm_name)) {
    boost::shared_ptr<SDLMusic> bgm = LoadMusic(bgm_name);
    bgm->fadeIn(loop, fade_in_ms);
  }
}

// -----------------------------------------------------------------------

void SDLSoundSystem::bgmPlay(const std::string& bgm_name,
                             bool loop, int fade_in_ms, int fade_out_ms) {
  if (!boost::iequals(bgmName(), bgm_name)) {
    queued_music_ = LoadMusic(bgm_name);
    queued_music_loop_ = loop;
    queued_music_fadein_ = fade_in_ms;

    bgmFadeOut(fade_out_ms);
  }
}

// -----------------------------------------------------------------------

void SDLSoundSystem::bgmStop() {
  boost::shared_ptr<SDLMusic> currently_playing = SDLMusic::CurrnetlyPlaying();
  if (currently_playing)
    currently_playing->stop();
}

// -----------------------------------------------------------------------

void SDLSoundSystem::bgmPause() {
  boost::shared_ptr<SDLMusic> currently_playing = SDLMusic::CurrnetlyPlaying();
  if (currently_playing)
    currently_playing->pause();
}

// -----------------------------------------------------------------------

void SDLSoundSystem::bgmUnPause() {
  boost::shared_ptr<SDLMusic> currently_playing = SDLMusic::CurrnetlyPlaying();
  if (currently_playing)
    currently_playing->unpause();
}

// -----------------------------------------------------------------------

void SDLSoundSystem::bgmFadeOut(int fade_out_ms) {
  boost::shared_ptr<SDLMusic> currently_playing = SDLMusic::CurrnetlyPlaying();
  if (currently_playing)
    currently_playing->fadeOut(fade_out_ms);
}

// -----------------------------------------------------------------------

std::string SDLSoundSystem::bgmName() const {
  boost::shared_ptr<SDLMusic> currently_playing = SDLMusic::CurrnetlyPlaying();
  if (currently_playing)
    return currently_playing->name();
  else
    return "";
}

// -----------------------------------------------------------------------

bool SDLSoundSystem::bgmLooping() const {
  boost::shared_ptr<SDLMusic> currently_playing = SDLMusic::CurrnetlyPlaying();
  if (currently_playing)
    return currently_playing->isLooping();
  else
    return false;
}

// -----------------------------------------------------------------------

bool SDLSoundSystem::koePlaying() const {
  return Mix_Playing(KOE_CHANNEL);
}

// -----------------------------------------------------------------------

void SDLSoundSystem::koeStop() {
  SDLSoundChunk::StopChannel(KOE_CHANNEL);
}

// -----------------------------------------------------------------------

void SDLSoundSystem::koePlayImpl(int id) {
  if (!koeEnabled()) {
    return;
  }

  // Get the VoiceSample.
  boost::shared_ptr<VoiceSample> sample = voice_cache_.find(id);
  if (!sample) {
    ostringstream oss;
    oss << "No sample for " << id;
    throw std::runtime_error(oss.str());
  }

  int length;
  char* data = sample->decode(&length);

  SDLSoundChunkPtr koe = buildKoeChunk(data, length);
  Mix_Volume(KOE_CHANNEL, realLiveVolumeToSDLMixerVolume(koeVolume()));
  koe->playChunkOn(KOE_CHANNEL, 0);
}

// -----------------------------------------------------------------------

void SDLSoundSystem::reset() {
  bgmStop();
  wavStopAll();

  SoundSystem::reset();
}

// -----------------------------------------------------------------------

void SDLSoundSystem::setMusicHook(
  void (*mix_func)(void *udata, Uint8 *stream, int len)) {
  if (!mix_func)
    mix_func = &SDLMusic::MixMusic;

  Mix_HookMusic(mix_func, NULL);
}
