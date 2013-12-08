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

#ifndef SRC_SYSTEMS_SDL_SDL_MUSIC_H_
#define SRC_SYSTEMS_SDL_SDL_MUSIC_H_

#include <SDL2/SDL_mixer.h>

#include <memory>
#include <string>

#include "systems/base/sound_system.h"
#include "xclannad/wavfile.h"

// Encapsulates access to SDLMussic.
//
// This system is the way it is for a good reason. The first shot of
// this had two subclasses of SDLMusic, one which implemented formats
// (ogg, mp3, wav, etc.), the other which mucks about with SDL_Mixer's
// Mix_HookMusic() method for just nwa files.
//
// Jagarl used Mix_HookMusic along with his own loaders for all file
// formats. He did this because in the \#DSTRACK definition, there is
// a point in the file where you loop back to when the track
// ends. This is why ripped NWA files end so abruptly if you play them
// in a music player: they're designed to be looped forever and you
// can't get that effect with built ins.
//
// So instead of taking just jagarl's nwatowav.cc, I'm also stealing
// wavfile.{cc,h}, and some binding code.
class SDLMusic : public std::enable_shared_from_this<SDLMusic> {
 public:
  virtual ~SDLMusic();

  // Whether we were told to loop when we were play()ed.
  bool IsLooping() const;

  // Whether we are currently fading out.
  bool IsFading() const;

  // TODO(erg): Now that I understand wtf is going on, redo this part here.
  void Play(bool loop);
  void Stop();
  void FadeIn(bool loop, int fade_in_ms);
  void FadeOut(int fade_out_ms);
  void Pause();
  void Unpause();
  std::string GetName() const;

  // Returns the current playing status of the current track. Uses the
  // same return codes as SoundSystem::bgmStatus().
  int BgmStatus() const;

  // Creates a MusicImpl object from the incoming description of the
  // music.
  static std::shared_ptr<SDLMusic> CreateMusic(
      System& system,
      const SoundSystem::DSTrack& track);

  // Returns the currently playing SDLMusic object. Returns NULL if no
  // music is currently playing.
  static std::shared_ptr<SDLMusic> CurrnetlyPlaying() {
    return s_currently_playing;
  }

  // Whether music is currently playing.
  static bool IsCurrentlyPlaying() { return s_currently_playing.get(); }

  // Whether we should output music.
  static void SetBgmEnabled(const int in) { s_bgm_enabled = in; }

  // What volume we should play this at normally.
  static void SetComputedBgmVolume(const int in) {
    s_computed_bgm_vol = in / 2;
  }

 private:
  // Builds an SDLMusic object.
  SDLMusic(const SoundSystem::DSTrack& track, WAVFILE* wav);

  // Callback function to Mix_HookMusic.
  //
  // This function was ripped off almost verbatim from xclannad! Specifically
  // the static method WavChunk::callback in music2/music.cc.
  static void MixMusic(void* udata, Uint8* stream, int len);

  // Strongly coupled because of access to SDLMusic::MixMusic.
  friend class SDLSoundSystem;

  // Underlying data stream. (These classes stolen from xclannad.)
  WAVFILE* file_;

  // The underlying track information
  const SoundSystem::DSTrack& track_;

  // No idea.
  int fade_count_;

  // Number of milliseconds left to fade out the
  int fadetime_total_;

  // Number of milliseconds to fade in.
  int fade_in_ms_;

  // The starting loop point.
  int loop_point_;

  // Whether the music is currently paused.
  bool music_paused_;

  // The currently playing track.
  static std::shared_ptr<SDLMusic> s_currently_playing;

  // Whether we should even be playing music.
  static bool s_bgm_enabled;

  // The volume we should play music at as a [0,128] range.
  static int s_computed_bgm_vol;
};

// -----------------------------------------------------------------------

#endif  // SRC_SYSTEMS_SDL_SDL_MUSIC_H_
