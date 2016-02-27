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

#ifndef SRC_SYSTEMS_BASE_SOUND_SYSTEM_H_
#define SRC_SYSTEMS_BASE_SOUND_SYSTEM_H_

#include <boost/serialization/access.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/version.hpp>
#include <map>
#include <string>
#include <utility>

#include "systems/base/voice_cache.h"

class Gameexe;
class System;

const int NUM_BASE_CHANNELS = 16;
const int NUM_EXTRA_WAVPLAY_CHANNELS = 8;
const int NUM_KOE_CHANNELS = 1;
const int NUM_TOTAL_CHANNELS =
    NUM_BASE_CHANNELS + NUM_EXTRA_WAVPLAY_CHANNELS + NUM_KOE_CHANNELS;

// The koe channel is the last one.
const int KOE_CHANNEL = NUM_BASE_CHANNELS + NUM_EXTRA_WAVPLAY_CHANNELS;

// Global sound settings and data, saved and restored when rlvm is shutdown and
// started up.
struct SoundSystemGlobals {
  SoundSystemGlobals();
  explicit SoundSystemGlobals(Gameexe& gexe);

  // Number passed in from RealLive that represents what we want the
  // sound system to do. Right now is fairly securely set to 5 since I
  // have no idea how to change this property at runtime.
  //
  // 0              11 k_hz               8 bit
  // 1              11 k_hz               16 bit
  // 2              22 k_hz               8 bit
  // 3              22 k_hz               16 bit
  // 4              44 k_hz               8 bit
  // 5              44 k_hz               16 bit
  // 6              48 k_hz               8 bit
  // 7              48 h_kz               16 bit
  int sound_quality;

  // Whether music playback is enabled
  bool bgm_enabled;

  // Volume for the music
  int bgm_volume_mod;

  // Whether the Wav functions are enabled
  bool pcm_enabled;

  // Volume of wave files relative to other sound playback.
  int pcm_volume_mod;

  // Whether the Se functions are enabled
  bool se_enabled;

  // Volume of interface sound effects relative to other sound playback.
  int se_volume_mod;

  // Voice playback mode (see setKoeMode() for details).
  int koe_mode;

  // Whether we play any voiceovers.
  bool koe_enabled;

  // Volume of the koe relative to other sound playback.
  int GetKoeVolume_mod;

  // Whether we fade the background music when a voiceover is playing.
  bool bgm_koe_fade;

  // How much to modify the bgm volume if |bgm_koe_fade| is on.
  int bgm_koe_fade_vol;

  // Maps between a koePlay character number, and whether we enable voices for
  // them.
  std::map<int, int> character_koe_enabled;

  // boost::serialization support
  template <class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar& sound_quality& bgm_enabled& bgm_volume_mod& pcm_enabled& pcm_volume_mod&
        se_enabled& se_volume_mod;

    if (version >= 1) {
      ar& koe_mode& koe_enabled& GetKoeVolume_mod& bgm_koe_fade& bgm_koe_fade_vol&
          character_koe_enabled;
    }
  }
};

BOOST_CLASS_VERSION(SoundSystemGlobals, 1)

// -----------------------------------------------------------------------

// Generalized interface to sound commands.
class SoundSystem {
 public:
  // Defines a piece of background music who's backed by a file, usually
  // VisualArt's nwa format.
  struct DSTrack {
    DSTrack();
    DSTrack(const std::string name,
            const std::string file,
            int from,
            int to,
            int loop);

    std::string name;
    std::string file;
    int from;
    int to;
    int loop;
  };

  // Defines a piece of background music who's backed by a cd audio track.
  struct CDTrack {
    CDTrack();
    CDTrack(const std::string name, int from, int to, int loop);

    std::string name;
    int from;
    int to;
    int loop;
  };

 protected:
  // Type for a parsed \#SE table.
  typedef std::map<int, std::pair<std::string, int>> SeTable;

  // Type for parsed \#DSTRACK entries.
  typedef std::map<std::string, DSTrack> DSTable;

  // Type for parsed \#CDTRACK entries.
  typedef std::map<std::string, CDTrack> CDTable;

  // Stores data about an ongoing volume adjustment (such as those started by
  // fun wavSetVolume(int, int, int).)
  struct VolumeAdjustTask {
    VolumeAdjustTask(unsigned int current_time,
                     int in_start_volume,
                     int in_final_volume,
                     int fade_time_in_ms);

    unsigned int start_time;
    unsigned int end_time;

    int start_volume;
    int final_volume;

    // Calculate the volume for in_time
    int calculateVolumeFor(unsigned int in_time);
  };

  typedef std::map<int, VolumeAdjustTask> ChannelAdjustmentMap;

 public:
  explicit SoundSystem(System& system);
  virtual ~SoundSystem();

  // Gives the sound system a chance to run; done once per game loop.
  //
  // Overriders MUST call SoundSystem::execute_sound_system because we rely on
  // it to handle volume adjustment tasks.
  virtual void ExecuteSoundSystem();

  // ---------------------------------------------------------------------

  // Sets how much sound hertz.
  virtual void SetSoundQuality(const int quality);

  int sound_quality() const { return globals_.sound_quality; }

  SoundSystemGlobals& globals() { return globals_; }

  // After loading global memory, there may be a mismatch between global state
  // and what subclasses of SoundSystem think because they overloaded a setter,
  // so set all values from the data in globals().
  void RestoreFromGlobals();

  // ---------------------------------------------------------------------

  // BGM functions
  int bgm_enabled() const { return globals_.bgm_enabled; }
  virtual void SetBgmEnabled(const int in);

  // User configured volume setting
  int bgm_volume_mod() const { return globals_.bgm_volume_mod; }
  virtual void SetBgmVolumeMod(const int in);

  // Programmer configured volume setting
  int bgm_volume_script() const { return bgm_volume_script_; }
  virtual void SetBgmVolumeScript(const int level, const int fade_in_ms);

  // Status of the music subsystem
  //
  // - 0 Idle
  // - 1 Playing music
  // - 2 Fading out music
  virtual int BgmStatus() const = 0;

  virtual void BgmPlay(const std::string& bgm_name, bool loop) = 0;
  virtual void BgmPlay(const std::string& bgm_name,
                       bool loop,
                       int fade_in_ms) = 0;
  virtual void BgmPlay(const std::string& bgm_name,
                       bool loop,
                       int fade_in_ms,
                       int fade_out_ms) = 0;
  virtual void BgmStop() = 0;
  virtual void BgmPause() = 0;
  virtual void BgmUnPause() = 0;
  virtual void BgmFadeOut(int fade_out_ms) = 0;

  virtual std::string GetBgmName() const = 0;
  virtual bool BgmLooping() const = 0;

  // ---------------------------------------------------------------------

  // @name PCM/Wave functions

  // Sets whether the wav* functions play
  int is_pcm_enabled() const { return globals_.pcm_enabled; }
  virtual void SetIsPcmEnabled(const int in);

  int pcm_volume_mod() const { return globals_.pcm_volume_mod; }
  virtual void SetPcmVolumeMod(const int in);

  // Sets an individual channel volume
  virtual void SetChannelVolume(const int channel, const int level);

  // Change the volume smoothly; the change from the current volume to level
  // will take fade_time_in_ms
  void SetChannelVolume(const int channel,
                        const int level,
                        const int fade_time_in_ms);

  // Fetches an individual channel volume
  int GetChannelVolume(const int channel) const;

  virtual void WavPlay(const std::string& wav_file, bool loop) = 0;
  virtual void WavPlay(const std::string& wav_file,
                       bool loop,
                       const int channel) = 0;
  virtual void WavPlay(const std::string& wav_file,
                       bool loop,
                       const int channel,
                       const int fadein_ms) = 0;
  virtual bool WavPlaying(const int channel) = 0;
  virtual void WavStop(const int channel) = 0;
  virtual void WavStopAll() = 0;
  virtual void WavFadeOut(const int channel, const int fadetime) = 0;

  // ---------------------------------------------------------------------

  // Sound Effect functions

  // Whether we should have interface sound effects
  int is_se_enabled() const { return globals_.se_enabled; }
  virtual void SetIsSeEnabled(const int in);

  // The volume of interface sound effects relative to other sound
  // playback. (0-255)
  int se_volume_mod() const { return globals_.se_volume_mod; }
  virtual void SetSeVolumeMod(const int in);

  // Plays an interface sound effect. |se_num| is an index into the #SE table.
  virtual void PlaySe(const int se_num) = 0;

  // Returns whether there is a sound effect |se_num| in the table.
  virtual bool HasSe(const int se_num) = 0;

  // ---------------------------------------------------------------------

  // Koe (voice) functions

  // Selects a voice playback mode, i.e. which form of communication to use for
  // strings having both text and voice data:
  //
  // - 0: Text and voice
  // - 1: Text only
  // - 2: Voice only
  //
  // TODO(erg): We keep track of this value, but we don't really USE it yet.
  int koe_mode() const { return globals_.koe_mode; }
  void setKoeMode(const int in) { globals_.koe_mode = in; }

  // Whether we should play voices (in general).
  int is_koe_enabled() const { return globals_.koe_enabled; }
  virtual void SetKoeEnabled(const int in);

  // Sets whether we play voices for certain characters.
  void SetUseKoeForCharacter(const int usekoe_id, const int enabled);

  // Returns whether we should play voices for certain characters. This
  // function is tied to UseKoe() family of functions and should not be queried
  // from within rlvm; use the |globals_.character_koe_enabled| map instead.
  int ShouldUseKoeForCharacter(const int usekoe_id) const;

  int GetKoeVolume_mod() const { return globals_.GetKoeVolume_mod; }
  virtual void SetKoeVolumeMod(const int level);

  // The volume for all voice levels (0-255). If |fadetime| is non-zero,
  // the volume will change smoothly, with the change taking |fadetime| ms,
  // otherwise it will change instantly.
  int GetKoeVolume() const;
  virtual void SetKoeVolume(const int level, const int fadetime);

  // Whether we fade the background when playing a voiceover.
  int bgm_koe_fade() const { return globals_.bgm_koe_fade; }
  void set_bgm_koe_fade(const int in) { globals_.bgm_koe_fade = in; }

  // Sets the amount by which the music volume is modified when the music/voice
  // fade flag is active.
  int bgm_koe_fadeVolume() const;
  void set_bgm_koe_fadeVolume(const int level);

  void KoePlay(int id);
  void KoePlay(int id, int charid);

  virtual bool KoePlaying() const = 0;
  virtual void KoeStop() = 0;

  virtual void Reset();

  System& system() { return system_; }

 protected:
  SeTable& se_table() { return se_table_; }
  const DSTable& ds_table() { return ds_tracks_; }
  const CDTable& cd_table() { return cd_tracks_; }

  // Computes the actual volume for a channel based on the per channel
  // and the per system volume.
  int compute_channel_volume(const int channel_volume,
                             const int system_volume) {
    return (channel_volume * system_volume) / 255;
  }

  // Plays a voice sample.
  virtual void KoePlayImpl(int id) = 0;

  static void CheckChannel(int channel, const char* function_name);
  static void CheckVolume(int level, const char* function_name);

  VoiceCache voice_cache_;

 private:
  System& system_;

  // Defined music tracks (files)
  DSTable ds_tracks_;

  // Defined music tracks (cd tracks)
  CDTable cd_tracks_;

  // The programmer controlled volume know for music.
  int bgm_volume_script_;

  // ---------------------------------------------------------------------

  // PCM/Wave sound effect data

  // Per channel volume
  unsigned char channel_volume_[NUM_TOTAL_CHANNELS];

  // Open tasks that adjust the volume of a wave channel. We do this
  // because SDL_mixer doesn't provide this functionality and I'm
  // guessing other mixers don't either.
  //
  // @note Depending on features in other systems, I may push this
  //       down to SDLSoundSystem later.
  ChannelAdjustmentMap pcm_adjustment_tasks_;

  std::unique_ptr<VolumeAdjustTask> bgm_adjustment_task_;

  // ---------------------------------------------------------------------

  // @name Interface sound effect data

  // Parsed #SE.index entries. Maps a sound effect number to the filename to
  // play and the channel to play it on.
  SeTable se_table_;

  // Maps each UseKoe id to one or more koePlay ids.
  std::multimap<int, int> usekoe_to_koeplay_mapping_;

  SoundSystemGlobals globals_;

  // boost::serialization support
  friend class boost::serialization::access;

  template <class Archive>
  void save(Archive& ar, const unsigned int file_version) const;

  template <class Archive>
  void load(Archive& ar, const unsigned int file_version);

  BOOST_SERIALIZATION_SPLIT_MEMBER()
};  // end of class SoundSystem

#endif  // SRC_SYSTEMS_BASE_SOUND_SYSTEM_H_
