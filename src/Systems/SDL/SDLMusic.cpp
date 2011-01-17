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

#include "Systems/SDL/SDLMusic.hpp"

#include <SDL/SDL_mixer.h>
#include <boost/algorithm/string.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/function.hpp>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "Systems/SDL/SDLAudioLocker.hpp"
#include "Utilities/Exception.hpp"
#include "Utilities/File.hpp"

using namespace std;
using namespace boost;
using boost::assign::map_list_of;
namespace fs = boost::filesystem;

// -----------------------------------------------------------------------

const int STOP_AT_END = -1;
const int STOP_NOW = -2;


boost::shared_ptr<SDLMusic> SDLMusic::s_currently_playing;
bool SDLMusic::s_bgm_enabled = true;

// -----------------------------------------------------------------------
// SDLMusic
// -----------------------------------------------------------------------

SDLMusic::SDLMusic(const SoundSystem::DSTrack& track, WAVFILE* wav)
    : file_(wav), track_(track), fadetime_total_(0), music_paused_(false) {
  // Advance the audio stream to the starting point
  if (track.from > 0)
    wav->Seek(track.from);
}

// -----------------------------------------------------------------------

SDLMusic::~SDLMusic() {
  SDLAudioLocker locker;
  delete file_;

  if (s_currently_playing.get() == this)
    s_currently_playing.reset();
}

// -----------------------------------------------------------------------

bool SDLMusic::isLooping() const {
  SDLAudioLocker locker;
  return loop_point_ != STOP_AT_END;
}

// -----------------------------------------------------------------------

bool SDLMusic::isFading() const {
  SDLAudioLocker locker;
  return fadetime_total_ > 0;
}

// -----------------------------------------------------------------------

void SDLMusic::play(bool loop) {
  SDLAudioLocker locker;
  setLoopPoint(loop);
  s_currently_playing = shared_from_this();
}

// -----------------------------------------------------------------------

void SDLMusic::stop() {
  SDLAudioLocker locker;
  if (s_currently_playing.get() == this)
    s_currently_playing.reset();
}

// -----------------------------------------------------------------------

void SDLMusic::fadeIn(bool loop, int fade_in_ms) {
  // Set up, then just play normally.
  cerr << "Doesn't deal with fade_in properly yet..." << endl;

  play(loop);
}

// -----------------------------------------------------------------------

void SDLMusic::fadeOut(int fade_out_ms) {
  SDLAudioLocker locker;
  fade_count_ = 0;
  if (fade_out_ms <= 0)
    fade_out_ms = 1;
  fadetime_total_ = fade_out_ms;
}

// -----------------------------------------------------------------------

void SDLMusic::pause() {
  SDLAudioLocker locker;
  music_paused_ = true;
}

// -----------------------------------------------------------------------

void SDLMusic::unpause() {
  SDLAudioLocker locker;
  music_paused_ = false;
}

// -----------------------------------------------------------------------

std::string SDLMusic::name() const {
  SDLAudioLocker locker;
  return track_.name;
}

// -----------------------------------------------------------------------

int SDLMusic::bgmStatus() const {
  SDLAudioLocker locker;

  if (music_paused_)
    return 0;
  else if (isFading())
    return 2;
  else
    return 1;
}

// -----------------------------------------------------------------------

// static
void SDLMusic::MixMusic(void *udata, Uint8 *stream, int len) {
  // Inside an SDL_LockAudio() section set up by SDL_Mixer! Don't lock here!
  SDLMusic* music = s_currently_playing.get();

  int count;
  if (!s_bgm_enabled || !music || music->music_paused_) {
    memset(stream, 0, len);
    return;
  }
  count = music->file_->Read((char*)stream, 4, len/4);

  if (count != len/4) {
    memset(stream+count*4, 0, len-count*4);
    if (music->loop_point_ == STOP_AT_END) {
      music->loop_point_ = STOP_NOW;
      s_currently_playing.reset();
    } else {
      music->file_->Seek(music->loop_point_);
      music->file_->Read( (char*)(stream+count*4), 4, len/4-count);
    }
  }
  if (music->fadetime_total_) {
    int count_total = music->fadetime_total_*(WAVFILE::freq/1000);
    if (music->fade_count_ > count_total ||
        music->fadetime_total_ == 1) {
      music->loop_point_ = STOP_NOW;
      s_currently_playing.reset();
      memset(stream, 0, len);
      return;
    }

    int cur_vol =
        SDL_MIX_MAXVOLUME * (count_total - music->fade_count_) /
        count_total;
    char stream_dup[len];  // NOLINT
    memcpy(stream_dup, stream, len);
    memset(stream, 0, len);
    SDL_MixAudio(stream, (Uint8*)stream_dup, len, cur_vol);
    music->fade_count_ += len/4;
  }
}

// -----------------------------------------------------------------------

template<typename TYPE>
WAVFILE* buildMusicImplementation(FILE* file, int size) {
  return WAVFILE::MakeConverter(new TYPE(file, size));
}

// -----------------------------------------------------------------------

boost::shared_ptr<SDLMusic> SDLMusic::CreateMusic(
    System& system, const SoundSystem::DSTrack& track) {
  typedef vector<pair<string, function<WAVFILE*(FILE*, int)> > > FileTypes;
  static FileTypes types =
      map_list_of
      ("wav", &buildMusicImplementation<WAVFILE_Stream>)
      ("nwa", &buildMusicImplementation<NWAFILE>)
      ("mp3", &buildMusicImplementation<MP3FILE>)
      ("ogg", &buildMusicImplementation<OggFILE>);

  fs::path file_path = findFile(system, track.file, SOUND_FILETYPES);
  if (file_path.empty()) {
    ostringstream oss;
    oss << "Could not find music file \"" << track.file << "\".";
    throw rlvm::Exception(oss.str());
  }

  const string& raw_path = file_path.external_file_string();
  for (FileTypes::const_iterator it = types.begin(); it != types.end(); ++it) {
    if (iends_with(raw_path, it->first)) {
      FILE* f = fopen(raw_path.c_str(), "r");
      if (f == 0) {
        ostringstream oss;
        oss << "Could not open \"" << file_path << "\" for reading.";
        throw std::runtime_error(oss.str());
      }

      fseek(f, 0, SEEK_END);
      int size = ftell(f);
      rewind(f);

      WAVFILE* w = it->second(f, size);
      if (w)
        return shared_ptr<SDLMusic>(new SDLMusic(track, w));
    }
  }

  ostringstream oss;
  oss << "Unsupported music file: \"" << file_path << "\"";
  throw std::runtime_error(oss.str());
}

// -----------------------------------------------------------------------
// SDLMusic (private)
// -----------------------------------------------------------------------
void SDLMusic::setLoopPoint(bool loop) {
  SDLAudioLocker locker;

  if (loop)
    loop_point_ = track_.loop;
  else
    loop_point_ = STOP_AT_END;
}
