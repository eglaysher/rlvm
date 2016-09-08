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

#include "systems/sdl/sdl_sound_system.h"

#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <sstream>
#include <string>

#include "systems/base/system.h"
#include "systems/base/system_error.h"
#include "systems/base/voice_archive.h"
#include "systems/sdl/resample.h"
#include "systems/sdl/sdl_music.h"
#include "systems/sdl/sdl_sound_chunk.h"
#include "utilities/exception.h"

namespace fs = boost::filesystem;

// -----------------------------------------------------------------------
// RealLive Sound Qualities table
// -----------------------------------------------------------------------
struct RealLiveSoundQualities {
  int rate;
  Uint16 format;
};

// A mapping between SoundQualities() and the values need to be passed
// to Mix_OpenAudio()
static RealLiveSoundQualities s_real_live_sound_qualities[] = {
    {11025, AUDIO_S8},   // 11 k_hz, 8 bit stereo
    {11025, AUDIO_S16},  // 11 k_hz, 16 bit stereo
    {22050, AUDIO_S8},   // 22 k_hz, 8 bit stereo
    {22050, AUDIO_S16},  // 22 k_hz, 16 bit stereo
    {44100, AUDIO_S8},   // 44 k_hz, 8 bit stereo
    {44100, AUDIO_S16},  // 44 k_hz, 16 bit stereo
    {48000, AUDIO_S8},   // 48 k_hz, 8 bit stereo
    {48000, AUDIO_S16}   // 48 h_kz, 16 bit stereo
};

// -----------------------------------------------------------------------
// SDLSoundSystem (private)
// -----------------------------------------------------------------------
SDLSoundSystem::SDLSoundChunkPtr SDLSoundSystem::GetSoundChunk(
    const std::string& file_name,
    SoundChunkCache& cache) {
  SDLSoundChunkPtr sample = cache.fetch(file_name);
  if (sample == NULL) {
    fs::path file_path = system().FindFile(file_name, SOUND_FILETYPES);
    if (file_path.empty()) {
      std::ostringstream oss;
      oss << "Could not find sound file \"" << file_name << "\".";
      throw rlvm::Exception(oss.str());
    }

    sample.reset(new SDLSoundChunk(file_path));
    cache.insert(file_name, sample);
  }

  return sample;
}

SDLSoundSystem::SDLSoundChunkPtr SDLSoundSystem::BuildKoeChunk(char* data,
                                                               int length) {
  return SDLSoundChunkPtr(new SDLSoundChunk(data, length));
}

void SDLSoundSystem::WavPlayImpl(const std::string& wav_file,
                                 const int channel,
                                 bool loop) {
  if (is_pcm_enabled()) {
    SDLSoundChunkPtr sample = GetSoundChunk(wav_file, wav_cache_);
    SetChannelVolumeImpl(channel);
    int loop_num = loop ? -1 : 0;
    sample->PlayChunkOn(channel, loop_num);
  }
}

void SDLSoundSystem::SetChannelVolumeImpl(int channel) {
  int base = channel == KOE_CHANNEL ? GetKoeVolume_mod() : pcm_volume_mod();
  int adjusted = compute_channel_volume(GetChannelVolume(channel), base);
  Mix_Volume(channel, realLiveVolumeToSDLMixerVolume(adjusted));
}

std::shared_ptr<SDLMusic> SDLSoundSystem::LoadMusic(
    const std::string& bgm_name) {
  DSTable::const_iterator ds_it = ds_table().find(boost::to_lower_copy(bgm_name));
  if (ds_it != ds_table().end())
    return SDLMusic::CreateMusic(system(), ds_it->second);

  CDTable::const_iterator cd_it = cd_table().find(boost::to_lower_copy(bgm_name));
  if (cd_it != cd_table().end()) {
    std::ostringstream oss;
    oss << "CD music not supported yet. Could not play track \"" << bgm_name
        << "\"";
    throw std::runtime_error(oss.str());
  }

  std::ostringstream oss;
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

  int audio_rate = s_real_live_sound_qualities[sound_quality()].rate;
  Uint16 audio_format = s_real_live_sound_qualities[sound_quality()].format;
  int audio_channels = 2;
  int audio_buffers = 4096;

  /* This is where we open up our audio device.  Mix_OpenAudio takes
     as its parameters the audio format we'd /like/ to have. */
  if (Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers)) {
    std::ostringstream oss;
    oss << "Couldn't initialize audio: " << Mix_GetError();
    throw SystemError(oss.str());
  }

  // Jagarl's sound system wants information on the audio settings.
  int freq, channels;
  Uint16 format;
  if (Mix_QuerySpec(&freq, &format, &channels)) {
    WAVFILE::freq = freq;
    WAVFILE::format = format;
    WAVFILE::channels = channels;
  }

  Mix_AllocateChannels(NUM_TOTAL_CHANNELS);

  Mix_ChannelFinished(&SDLSoundChunk::SoundChunkFinishedPlayback);

  SetMusicHook(NULL);
}

SDLSoundSystem::~SDLSoundSystem() {
  Mix_HookMusic(NULL, NULL);

  Mix_CloseAudio();
  SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

void SDLSoundSystem::ExecuteSoundSystem() {
  SoundSystem::ExecuteSoundSystem();

  if (queued_music_ && !SDLMusic::IsCurrentlyPlaying()) {
    queued_music_->FadeIn(queued_music_loop_, queued_music_fadein_);
    queued_music_.reset();
  }
}

void SDLSoundSystem::SetBgmEnabled(const int in) {
  SDLMusic::SetBgmEnabled(in);
  SoundSystem::SetBgmEnabled(in);
}

void SDLSoundSystem::SetBgmVolumeMod(const int in) {
  SoundSystem::SetBgmVolumeMod(in);
  SDLMusic::SetComputedBgmVolume(compute_channel_volume(in, bgm_volume_script()));
}

void SDLSoundSystem::SetBgmVolumeScript(const int level, int fade_in_ms) {
  SoundSystem::SetBgmVolumeScript(level, fade_in_ms);
  if (fade_in_ms == 0) {
    // If a fade was requested by the script, we don't want to set the volume
    // here right now. This is only slightly cleaner than having separate
    // methods because of the function casting in the modules.
    SDLMusic::SetComputedBgmVolume(compute_channel_volume(bgm_volume_mod(), level));
  }
}

void SDLSoundSystem::SetChannelVolume(const int channel, const int level) {
  SoundSystem::SetChannelVolume(channel, level);
  SetChannelVolumeImpl(channel);
}

void SDLSoundSystem::WavPlay(const std::string& wav_file, bool loop) {
  int channel_number = SDLSoundChunk::FindNextFreeExtraChannel();
  if (channel_number == -1) {
    std::ostringstream oss;
    oss << "Couldn't find a free channel for wavPlay()";
    throw std::runtime_error(oss.str());
  }

  WavPlayImpl(wav_file, channel_number, loop);
}

void SDLSoundSystem::WavPlay(const std::string& wav_file,
                             bool loop,
                             const int channel) {
  CheckChannel(channel, "SDLSoundSystem::wav_play");
  WavPlayImpl(wav_file, channel, loop);
}

void SDLSoundSystem::WavPlay(const std::string& wav_file,
                             bool loop,
                             const int channel,
                             const int fadein_ms) {
  CheckChannel(channel, "SDLSoundSystem::wav_play");

  if (is_pcm_enabled()) {
    SDLSoundChunkPtr sample = GetSoundChunk(wav_file, wav_cache_);
    SetChannelVolumeImpl(channel);

    int loop_num = loop ? -1 : 0;
    sample->FadeInChunkOn(channel, loop_num, fadein_ms);
  }
}

bool SDLSoundSystem::WavPlaying(const int channel) {
  CheckChannel(channel, "SDLSoundSystem::wav_playing");
  return Mix_Playing(channel);
}

void SDLSoundSystem::WavStop(const int channel) {
  CheckChannel(channel, "SDLSoundSystem::wav_stop");

  if (is_pcm_enabled()) {
    SDLSoundChunk::StopChannel(channel);
  }
}

void SDLSoundSystem::WavStopAll() {
  if (is_pcm_enabled()) {
    SDLSoundChunk::StopAllChannels();
  }
}

void SDLSoundSystem::WavFadeOut(const int channel, const int fadetime) {
  CheckChannel(channel, "SDLSoundSystem::wav_fade_out");

  if (is_pcm_enabled())
    SDLSoundChunk::FadeOut(channel, fadetime);
}

void SDLSoundSystem::PlaySe(const int se_num) {
  if (is_se_enabled()) {
    SeTable::const_iterator it = se_table().find(se_num);
    if (it == se_table().end()) {
      std::ostringstream oss;
      oss << "No #SE entry found for sound effect number " << se_num;
      throw rlvm::Exception(oss.str());
    }

    const std::string& file_name = it->second.first;
    int channel = it->second.second;

    // Make sure there isn't anything playing on the current channel
    Mix_HaltChannel(channel);

    if (file_name == "") {
      // Just stop a channel in case of an empty file name.
      return;
    }

    SDLSoundChunkPtr sample = GetSoundChunk(file_name, wav_cache_);

    // SE chunks have no volume other than the modifier.
    Mix_Volume(channel, realLiveVolumeToSDLMixerVolume(se_volume_mod()));
    sample->PlayChunkOn(channel, 0);
  }
}

bool SDLSoundSystem::HasSe(const int se_num) {
  SeTable::const_iterator it = se_table().find(se_num);
  return it != se_table().end();
}

int SDLSoundSystem::BgmStatus() const {
  std::shared_ptr<SDLMusic> currently_playing = SDLMusic::CurrnetlyPlaying();
  if (currently_playing) {
    return currently_playing->BgmStatus();
  } else {
    return 0;
  }
}

void SDLSoundSystem::BgmPlay(const std::string& bgm_name, bool loop) {
  if (!boost::iequals(GetBgmName(), bgm_name)) {
    std::shared_ptr<SDLMusic> bgm = LoadMusic(bgm_name);
    bgm->Play(loop);
  }
}

void SDLSoundSystem::BgmPlay(const std::string& bgm_name,
                             bool loop,
                             int fade_in_ms) {
  if (!boost::iequals(GetBgmName(), bgm_name)) {
    std::shared_ptr<SDLMusic> bgm = LoadMusic(bgm_name);
    bgm->FadeIn(loop, fade_in_ms);
  }
}

void SDLSoundSystem::BgmPlay(const std::string& bgm_name,
                             bool loop,
                             int fade_in_ms,
                             int fade_out_ms) {
  if (!boost::iequals(GetBgmName(), bgm_name)) {
    queued_music_ = LoadMusic(bgm_name);
    queued_music_loop_ = loop;
    queued_music_fadein_ = fade_in_ms;

    BgmFadeOut(fade_out_ms);
  }
}

void SDLSoundSystem::BgmStop() {
  std::shared_ptr<SDLMusic> currently_playing = SDLMusic::CurrnetlyPlaying();
  if (currently_playing)
    currently_playing->Stop();
}

void SDLSoundSystem::BgmPause() {
  std::shared_ptr<SDLMusic> currently_playing = SDLMusic::CurrnetlyPlaying();
  if (currently_playing)
    currently_playing->Pause();
}

void SDLSoundSystem::BgmUnPause() {
  std::shared_ptr<SDLMusic> currently_playing = SDLMusic::CurrnetlyPlaying();
  if (currently_playing)
    currently_playing->Unpause();
}

void SDLSoundSystem::BgmFadeOut(int fade_out_ms) {
  std::shared_ptr<SDLMusic> currently_playing = SDLMusic::CurrnetlyPlaying();
  if (currently_playing)
    currently_playing->FadeOut(fade_out_ms);
}

std::string SDLSoundSystem::GetBgmName() const {
  std::shared_ptr<SDLMusic> currently_playing = SDLMusic::CurrnetlyPlaying();
  if (currently_playing)
    return currently_playing->GetName();
  else
    return "";
}

bool SDLSoundSystem::BgmLooping() const {
  std::shared_ptr<SDLMusic> currently_playing = SDLMusic::CurrnetlyPlaying();
  if (currently_playing)
    return currently_playing->IsLooping();
  else
    return false;
}

bool SDLSoundSystem::KoePlaying() const { return Mix_Playing(KOE_CHANNEL); }

void SDLSoundSystem::KoeStop() { SDLSoundChunk::StopChannel(KOE_CHANNEL); }

void SDLSoundSystem::KoePlayImpl(int id) {
  if (!is_koe_enabled()) {
    return;
  }

  // Get the VoiceSample.
  std::shared_ptr<VoiceSample> sample = voice_cache_.Find(id);
  if (!sample) {
    std::ostringstream oss;
    oss << "No sample for " << id;
    throw std::runtime_error(oss.str());
  }

  int length;
  char* data = sample->Decode(&length);

  // TODO(erg): SDL is supposed to have a real resampler, but doesn't, so for
  // example, 48k -> 41k is at best tone shifted, and at worst, is a pure
  // static. So we have to do our own manual resampling.
  //
  // The correct way to deal with this is to move off SDL's audio subsystem
  // entirely or to contribute upstream to SDL so that its default behaviour is
  // sane. There's no time left to do either of those, as there's a fairly
  // hard deadline of 9/11 for the birthday release, and this is the last
  // blocker.
  data = EnsureDataIsCorrectBitrate(data, &length);

  SDLSoundChunkPtr koe = BuildKoeChunk(data, length);
  SetChannelVolumeImpl(KOE_CHANNEL);
  koe->PlayChunkOn(KOE_CHANNEL, 0);
}

void SDLSoundSystem::Reset() {
  BgmStop();
  WavStopAll();

  SoundSystem::Reset();
}

void SDLSoundSystem::SetMusicHook(void (*mix_func)(void* udata,
                                                   Uint8* stream,
                                                   int len)) {
  if (!mix_func)
    mix_func = &SDLMusic::MixMusic;

  Mix_HookMusic(mix_func, NULL);
}
