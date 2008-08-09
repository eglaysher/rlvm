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
  : soundQuality(5), bgmEnabled(true), bgmVolume(255), pcmEnabled(true),
    pcmVolume(255), seEnabled(true), seVolume(255)
{

}

// -----------------------------------------------------------------------

SoundSystemGlobals::SoundSystemGlobals(Gameexe& gexe)
  : soundQuality(gexe("SOUND_DEFAULT").to_int(5)),
    bgmEnabled(true), bgmVolume(255), pcmEnabled(true),
    pcmVolume(255), seEnabled(true), seVolume(255)
{}

// -----------------------------------------------------------------------
// SoundSystem::VolumeAdjustTask
// -----------------------------------------------------------------------
SoundSystem::VolumeAdjustTask::VolumeAdjustTask(
  unsigned int currentTime, int inStartVolume, int inFinalVolume,
  int fadeTimeInMs)
  : startTime(currentTime), endTime(currentTime + fadeTimeInMs),
    startVolume(inStartVolume), finalVolume(inFinalVolume)
{
}

int SoundSystem::VolumeAdjustTask::calculateVolumeFor(unsigned int inTime)
{
  int endOffset = endTime - startTime;
  int curOffset = inTime - startTime;
  double percent = double(curOffset) / endOffset;

  int candidateVol = startVolume + (percent * (finalVolume - startVolume));
  if(candidateVol < startVolume)
    candidateVol = startVolume;
  else if(candidateVol > finalVolume)
    candidateVol = finalVolume;

  return candidateVol;
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
  const std::string inName, const std::string inFile, int inFrom,
  int inTo, int inLoop)
  : name(inName), file(inFile), from(inFrom), to(inTo), loop(inLoop)
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
  const std::string inName, int inFrom, int inTo, int inLoop)
  : name(inName), from(inFrom), to(inTo), loop(inLoop)
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
    string rawNumber = se->key_parts().at(1);
    int entryNumber = lexical_cast<int>(rawNumber);

    string fileName = se->getStringAt(0);
    int targetChannel = se->getIntAt(1);

    se_table_[entryNumber] = make_pair(fileName, targetChannel);
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
  unsigned int curTime = machine.system().event().getTicks();

  ChannelAdjustmentMap::iterator it = pcm_adjustment_tasks_.begin();
  while(it != pcm_adjustment_tasks_.end())
  {
    if(curTime >= it->second.endTime)
    {
      setChannelVolume(it->first, it->second.finalVolume);
      pcm_adjustment_tasks_.erase(it++);
    }
    else
    {
      int volume = it->second.calculateVolumeFor(curTime);
      setChannelVolume(it->first, volume);
      ++it;
    }
  }
}

// -----------------------------------------------------------------------

void SoundSystem::setBgmEnabled(const int in)
{
  globals_.bgmEnabled = in;
}

// -----------------------------------------------------------------------

int SoundSystem::bgmEnabled() const
{
  return globals_.bgmEnabled;
}

// -----------------------------------------------------------------------

void SoundSystem::setBgmVolume(const int in)
{
  globals_.bgmVolume = in;
}

// -----------------------------------------------------------------------

int SoundSystem::bgmVolume() const
{
  return globals_.bgmVolume;
}

// -----------------------------------------------------------------------

void SoundSystem::setPcmEnabled(const int in)
{
  globals_.pcmEnabled = in;
}

// -----------------------------------------------------------------------

int SoundSystem::pcmEnabled() const
{
  return globals_.pcmEnabled;
}

// -----------------------------------------------------------------------

void SoundSystem::setPcmVolume(const int in)
{
  globals_.pcmVolume = in;
}

// -----------------------------------------------------------------------

int SoundSystem::pcmVolume() const
{
  return globals_.pcmVolume;
}

// -----------------------------------------------------------------------

void SoundSystem::setChannelVolume(const int channel, const int level)
{
  checkChannel(channel, "setChannelVolume");
  checkVolume(level, "setChannelVolume");

  channel_volume_[channel] = level;
}

// -----------------------------------------------------------------------

void SoundSystem::setChannelVolume(
  RLMachine& machine, const int channel, const int level,
  const int fadeTimeInMs)
{
  checkChannel(channel, "setChannelVolume");
  checkVolume(level, "setChannelVolume");

  unsigned int curTime = machine.system().event().getTicks();

  pcm_adjustment_tasks_.insert(
    make_pair(channel, VolumeAdjustTask(curTime, channel_volume_[channel],
                                        level, fadeTimeInMs)));
}

// -----------------------------------------------------------------------

int SoundSystem::channelVolume(const int channel)
{
  checkChannel(channel, "channelVolume");
  return channel_volume_[channel];
}

// -----------------------------------------------------------------------

void SoundSystem::setSeEnabled(const int in)
{
  globals_.seEnabled = in;
}

// -----------------------------------------------------------------------

int SoundSystem::seEnabled() const
{
  return globals_.seEnabled;
}

// -----------------------------------------------------------------------

void SoundSystem::setSeVolume(const int level)
{
  checkVolume(level, "setSeVolume");
  globals_.seVolume = level;
}

// -----------------------------------------------------------------------

int SoundSystem::seVolume() const
{
  return globals_.seVolume;
}

// -----------------------------------------------------------------------

void SoundSystem::reset()
{
  // empty
}

// -----------------------------------------------------------------------

// static
void SoundSystem::checkChannel(int channel, const char* functionName)
{
  if(channel < 0 || channel > NUM_BASE_CHANNELS)
  {
    ostringstream oss;
    oss << "Invalid channel number " << channel << " in " << functionName;
    throw std::runtime_error(oss.str());
  }
}

// -----------------------------------------------------------------------

// static
void SoundSystem::checkVolume(int level, const char* functionName)
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
  std::string trackName;
  bool looping;
  ar & trackName & looping;

  if(trackName != "")
    bgmPlay(*Serialization::g_currentMachine, trackName, looping);
}

// -----------------------------------------------------------------------

template<class Archive>
void SoundSystem::save(Archive& ar, unsigned int version) const
{
  std::string trackName;
  bool looping = false;

  if (bgmStatus() == 1)
    trackName = bgmName();

  ar & trackName & looping;
}

// -----------------------------------------------------------------------

// Explicit instantiations for text archives (since we hide the
// implementation)

template void SoundSystem::save<boost::archive::text_oarchive>(
  boost::archive::text_oarchive & ar, unsigned int version) const;

template void SoundSystem::load<boost::archive::text_iarchive>(
  boost::archive::text_iarchive & ar, unsigned int version);

