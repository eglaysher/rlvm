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

#include "Systems/SDL/SDLMusic.hpp"
#include "Utilities.h"

#include <vector>
#include <map>
#include <iostream>
#include <sstream>
#include <SDL/SDL_mixer.h>

#include <boost/function.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace boost;
using boost::assign::map_list_of;
namespace fs = boost::filesystem;

// -----------------------------------------------------------------------

const int STOP_AT_END = -1;
const int STOP_NOW = -2;


boost::shared_ptr<SDLMusic> SDLMusic::s_currentlyPlaying;


// -----------------------------------------------------------------------
// SDLMusic
// -----------------------------------------------------------------------

SDLMusic::SDLMusic(const SoundSystem::DSTrack& track, WAVFILE* wav)
  : m_file(wav), m_track(track), m_fadetimeTotal(0)
{
  // Advance the audio stream to the starting point
  if(track.from > 0)
    wav->Seek(track.from);
}

// -----------------------------------------------------------------------

SDLMusic::~SDLMusic()
{
  Mix_HookMusic(NULL, NULL);

  delete m_file;

  if(s_currentlyPlaying.get() == this)
    s_currentlyPlaying.reset();
}

// -----------------------------------------------------------------------

void SDLMusic::play(bool loop)
{
  setLoopPoint(loop);
  s_currentlyPlaying = shared_from_this();
	Mix_HookMusic(&SDLMusic::MixMusic, this);
}

// -----------------------------------------------------------------------

void SDLMusic::stop()
{
  Mix_HookMusic(NULL, NULL);
}

// -----------------------------------------------------------------------

void SDLMusic::fadeIn(bool loop, int fadeInMs)
{
  // Set up, then just play normally.
  cerr << "Doesn't deal with fadeIn properly yet..." << endl;

  play(loop);
}

// -----------------------------------------------------------------------

void SDLMusic::fadeOut(int fadeOutMs)
{
  m_fadeCount = 0;
  if(fadeOutMs <= 0) 
    fadeOutMs = 1;
  m_fadetimeTotal = fadeOutMs;
}

// -----------------------------------------------------------------------

// static
void SDLMusic::MixMusic(void *udata, Uint8 *stream, int len)
{
  // TODO: Make the done states reset s_currentlyPlaying. Or go back
  // with a finish hook (and copy the finishing logic from either
  // xclannad or the official SDL_Mixer)
  SDLMusic* music = (SDLMusic*)udata;

	int count;
	if (music->m_loopPoint == STOP_NOW) { // We are done.
		memset(stream, 0, len);
		return;
	}
	count = music->m_file->Read( (char*)stream, 4, len/4);

	if (count != len/4) {
		memset(stream+count*4, 0, len-count*4);
		if (music->m_loopPoint == STOP_AT_END) {
			music->m_loopPoint = STOP_NOW;
		} else {
			music->m_file->Seek(music->m_loopPoint);
			music->m_file->Read( (char*)(stream+count*4), 4, len/4-count);
		}
	}
	if (music->m_fadetimeTotal) {
		int count_total = music->m_fadetimeTotal*(WAVFILE::freq/1000);
		if (music->m_fadeCount > count_total || 
        music->m_fadetimeTotal == 1) {
			music->m_loopPoint = STOP_NOW;
			memset(stream, 0, len);
			return;
		}

		int cur_vol =
      SDL_MIX_MAXVOLUME * (count_total - music->m_fadeCount) /
      count_total;
		char* stream_dup = new char[len];
		memcpy(stream_dup, stream, len);
		memset(stream, 0, len);
		SDL_MixAudio(stream, (Uint8*)stream_dup, len, cur_vol);
		music->m_fadeCount += len/4;
	}
}

// -----------------------------------------------------------------------

// void SDLMusic::MusicFinishedHook()
// {
//   s_currentlyPlaying.reset();
// }

// -----------------------------------------------------------------------

template<typename TYPE>
WAVFILE* buildMusicImplementation(FILE* file, int size)
{
  return WAVFILE::MakeConverter(new TYPE(file, size));
}

// -----------------------------------------------------------------------

boost::shared_ptr<SDLMusic> SDLMusic::CreateMusic(
  RLMachine& machine, const SoundSystem::DSTrack& track)
{
  typedef vector<pair<string, function<WAVFILE*(FILE*, int)> > > FileTypes;
  static FileTypes types = 
    map_list_of
    ("wav", &buildMusicImplementation<WAVFILE_Stream>)
    ("nwa", &buildMusicImplementation<NWAFILE>)
    ("mp3", &buildMusicImplementation<MP3FILE>);
//    ("ogg", &buildMusicImplementationWithImpl<OggFILE>);

  fs::path filePath = findFile(machine, track.file, SOUND_FILETYPES);
  const string& rawPath = filePath.external_file_string();
  for(FileTypes::const_iterator it = types.begin(); it != types.end(); ++it)
  {
    if(iends_with(rawPath, it->first))
    {
      FILE* f = fopen(rawPath.c_str(), "r");
      if(f == 0)
      {
        ostringstream oss;
        oss << "Could not open \"" << filePath << "\" for reading.";
        throw std::runtime_error(oss.str());
      }

      fseek(f, 0, SEEK_END);
      int size = ftell(f);
      rewind(f);

      WAVFILE* w = it->second(f, size);
      if(w)
        return shared_ptr<SDLMusic>(new SDLMusic(track, w));
    }
  }

  ostringstream oss;
  oss << "Unsupported music file: \"" << filePath << "\"";
  throw std::runtime_error(oss.str());
}

// -----------------------------------------------------------------------
// SDLMusic (private)
// -----------------------------------------------------------------------
void SDLMusic::setLoopPoint(bool loop)
{
  if(loop)
    m_loopPoint = m_track.loop;
  else
    m_loopPoint = STOP_AT_END;
}
