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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// -----------------------------------------------------------------------

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include "Systems/Base/SoundSystem.hpp"

#include "MachineBase/RLMachine.hpp"
#include "MachineBase/Serialization.hpp"
#include "Systems/Base/EventSystem.hpp"
#include "Systems/Base/System.hpp"
#include "libReallive/gameexe.h"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>

// -----------------------------------------------------------------------

using namespace std;
using boost::lexical_cast;
using namespace std;

// -----------------------------------------------------------------------
// SoundSystemGlobals
// -----------------------------------------------------------------------
SoundSystemGlobals::SoundSystemGlobals()
  : sound_quality(5), bgm_enabled(true), bgm_volume(255), pcm_enabled(true),
    pcm_volume(255), se_enabled(true), se_volume(255)
{}

// -----------------------------------------------------------------------

SoundSystemGlobals::SoundSystemGlobals(Gameexe& gexe)
  : sound_quality(gexe("SOUND_DEFAULT").to_int(5)),
    bgm_enabled(true), bgm_volume(255), pcm_enabled(true),
    pcm_volume(255), se_enabled(true), se_volume(255)
{}

// -----------------------------------------------------------------------
// SoundSystem::VolumeAdjustTask
// -----------------------------------------------------------------------
SoundSystem::VolumeAdjustTask::VolumeAdjustTask(
  unsigned int current_time, int in_start_volume, int in_final_volume,
  int fade_time_in_ms)
  : start_time(current_time), end_time(current_time + fade_time_in_ms),
    start_volume(in_start_volume), final_volume(in_final_volume)
{}

// -----------------------------------------------------------------------

int SoundSystem::VolumeAdjustTask::calculateVolumeFor(unsigned int in_time)
{
  int end_offset = end_time - start_time;
  int cur_offset = in_time - start_time;
  double percent = double(cur_offset) / end_offset;

  int candidate_vol = start_volume + (percent * (final_volume - start_volume));
  if(candidate_vol < start_volume)
    candidate_vol = start_volume;
  else if(candidate_vol > final_volume)
    candidate_vol = final_volume;

  return candidate_vol;
}

// -----------------------------------------------------------------------
// SoundSystem::DSTrack
// -----------------------------------------------------------------------
SoundSystem::DSTrack::DSTrack()
  : name(""), file(""), from(-1), to(-1), loop(-1)
{
}

// -----------------------------------------------------------------------

SoundSystem::DSTrack::DSTrack(
  const std::string in_name, const std::string in_file, int in_from,
  int in_to, int in_loop)
  : name(in_name), file(in_file), from(in_from), to(in_to), loop(in_loop)
{
}

// -----------------------------------------------------------------------
// SoundSystem::CDTrack
// -----------------------------------------------------------------------
SoundSystem::CDTrack::CDTrack()
  : name(""), from(-1), to(-1), loop(-1)
{
}

// -----------------------------------------------------------------------

SoundSystem::CDTrack::CDTrack(
  const std::string in_name, int in_from, int in_to, int in_loop)
  : name(in_name), from(in_from), to(in_to), loop(in_loop)
{
}

// -----------------------------------------------------------------------
// SoundSystem
// -----------------------------------------------------------------------
SoundSystem::SoundSystem(Gameexe& gexe)
  : globals_(gexe)
{
  std::fill_n(channel_volume_, NUM_BASE_CHANNELS, 255);

  // Read the \#SE.xxx entries from the Gameexe
  GameexeFilteringIterator se = gexe.filtering_begin("SE.");
  GameexeFilteringIterator end = gexe.filtering_end();
  for(; se != end; ++se)
  {
    string raw_number = se->key_parts().at(1);
    int entry_number = lexical_cast<int>(raw_number);

    string file_name = se->getStringAt(0);
    int target_channel = se->getIntAt(1);

    se_table_[entry_number] = make_pair(file_name, target_channel);
  }

  // Read the \#DSTRACK entries
  GameexeFilteringIterator dstrack = gexe.filtering_begin("DSTRACK");
  for(; dstrack != end; ++dstrack)
  {
    int from = dstrack->getIntAt(0);
    int to = dstrack->getIntAt(1);
    int loop = dstrack->getIntAt(2);
    const std::string& file = dstrack->getStringAt(3);
    std::string name = dstrack->getStringAt(4);
    boost::to_lower(name);

    ds_tracks_[name] = DSTrack(name, file, from, to, loop);
  }

  // Read the \#CDTRACK entries
  GameexeFilteringIterator cdtrack = gexe.filtering_begin("CDTRACK");
  for(; cdtrack != end; ++cdtrack)
  {
    int from = cdtrack->getIntAt(0);
    int to = cdtrack->getIntAt(1);
    int loop = cdtrack->getIntAt(2);
    std::string name = cdtrack->getStringAt(3);
    boost::to_lower(name);

    cd_tracks_[name] = CDTrack(name, from, to, loop);
  }
}

// -----------------------------------------------------------------------

SoundSystem::~SoundSystem()
{}

// -----------------------------------------------------------------------

void SoundSystem::executeSoundSystem(RLMachine& machine)
{
  unsigned int cur_time = machine.system().event().getTicks();

  ChannelAdjustmentMap::iterator it = pcm_adjustment_tasks_.begin();
  while(it != pcm_adjustment_tasks_.end())
  {
    if(cur_time >= it->second.end_time)
    {
      setChannelVolume(it->first, it->second.final_volume);
      pcm_adjustment_tasks_.erase(it++);
    }
    else
    {
      int volume = it->second.calculateVolumeFor(cur_time);
      setChannelVolume(it->first, volume);
      ++it;
    }
  }
}

// -----------------------------------------------------------------------

void SoundSystem::restoreFromGlobals()
{
  setBgmEnabled(bgmEnabled());
  setBgmVolume(bgmVolume());

  setPcmEnabled(pcmEnabled());
  setPcmVolume(pcmVolume());

  setSeEnabled(seEnabled());
  setSeVolume(seVolume());
}

// -----------------------------------------------------------------------

void SoundSystem::setBgmEnabled(const int in)
{
  globals_.bgm_enabled = in;
}

// -----------------------------------------------------------------------

int SoundSystem::bgmEnabled() const
{
  return globals_.bgm_enabled;
}

// -----------------------------------------------------------------------

void SoundSystem::setBgmVolume(const int in)
{
  globals_.bgm_volume = in;
}

// -----------------------------------------------------------------------

int SoundSystem::bgmVolume() const
{
  return globals_.bgm_volume;
}

// -----------------------------------------------------------------------

void SoundSystem::setPcmEnabled(const int in)
{
  globals_.pcm_enabled = in;
}

// -----------------------------------------------------------------------

int SoundSystem::pcmEnabled() const
{
  return globals_.pcm_enabled;
}

// -----------------------------------------------------------------------

void SoundSystem::setPcmVolume(const int in)
{
  globals_.pcm_volume = in;
}

// -----------------------------------------------------------------------

int SoundSystem::pcmVolume() const
{
  return globals_.pcm_volume;
}

// -----------------------------------------------------------------------

void SoundSystem::setChannelVolume(const int channel, const int level)
{
  checkChannel(channel, "set_channel_volume");
  checkVolume(level, "set_channel_volume");

  channel_volume_[channel] = level;
}

// -----------------------------------------------------------------------

void SoundSystem::setChannelVolume(
  RLMachine& machine, const int channel, const int level,
  const int fade_time_in_ms)
{
  checkChannel(channel, "set_channel_volume");
  checkVolume(level, "set_channel_volume");

  unsigned int cur_time = machine.system().event().getTicks();

  pcm_adjustment_tasks_.insert(
    make_pair(channel, VolumeAdjustTask(cur_time, channel_volume_[channel],
                                        level, fade_time_in_ms)));
}

// -----------------------------------------------------------------------

int SoundSystem::channelVolume(const int channel)
{
  checkChannel(channel, "channel_volume");
  return channel_volume_[channel];
}

// -----------------------------------------------------------------------

void SoundSystem::setSeEnabled(const int in)
{
  globals_.se_enabled = in;
}

// -----------------------------------------------------------------------

int SoundSystem::seEnabled() const
{
  return globals_.se_enabled;
}

// -----------------------------------------------------------------------

void SoundSystem::setSeVolume(const int level)
{
  checkVolume(level, "set_se_volume");
  globals_.se_volume = level;
}

// -----------------------------------------------------------------------

int SoundSystem::seVolume() const
{
  return globals_.se_volume;
}

// -----------------------------------------------------------------------

void SoundSystem::reset()
{
  // empty
}

// -----------------------------------------------------------------------

// static
void SoundSystem::checkChannel(int channel, const char* function_name)
{
  if(channel < 0 || channel > NUM_BASE_CHANNELS)
  {
    ostringstream oss;
    oss << "Invalid channel number " << channel << " in " << function_name;
    throw std::runtime_error(oss.str());
  }
}

// -----------------------------------------------------------------------

// static
void SoundSystem::checkVolume(int level, const char* function_name)
{
  if(level < 0 || level > 255)
  {
    ostringstream oss;
    oss << "Invalid volume \"" << level << "\". Valid values are 0-255.";
    throw std::runtime_error(oss.str());
  }
}

// -----------------------------------------------------------------------

template<class Archive>
void SoundSystem::load(Archive& ar, unsigned int version)
{
  std::string track_name;
  bool looping;
  ar & track_name & looping;

  if(track_name != "")
    bgmPlay(*Serialization::g_current_machine, track_name, looping);
}

// -----------------------------------------------------------------------

template<class Archive>
void SoundSystem::save(Archive& ar, unsigned int version) const
{
  std::string track_name;
  bool looping = false;

  if (bgmStatus() == 1)
    track_name = bgmName();

  ar & track_name & looping;
}

// -----------------------------------------------------------------------

// Explicit instantiations for text archives (since we hide the
// implementation)

template void SoundSystem::save<boost::archive::text_oarchive>(
  boost::archive::text_oarchive & ar, unsigned int version) const;

template void SoundSystem::load<boost::archive::text_iarchive>(
  boost::archive::text_iarchive & ar, unsigned int version);

