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

#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/SoundSystem.hpp"
#include "Systems/Base/EventSystem.hpp"
#include "libReallive/gameexe.h"
#include <boost/lexical_cast.hpp>
#include <iostream>

// -----------------------------------------------------------------------

using namespace std;
using boost::lexical_cast;
using namespace std;

// -----------------------------------------------------------------------
// Error handling methods
// -----------------------------------------------------------------------
namespace {

void checkChannel(int channel, const char* functionName) {
  if(channel < 0 || channel > NUM_BASE_CHANNELS)
  {
    ostringstream oss;
    oss << "Invalid channel number " << channel << " in " << functionName;
    throw std::runtime_error(oss.str());
  }
}

void checkVolume(int level, const char* functionName) {
  if(level < 0 || level > 255)
  {
    ostringstream oss;
    oss << "Invalid volume \"" << level << "\". Valid values are 0-255.";
    throw std::runtime_error(oss.str());
  }
}

}

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
// SoundSystem
// -----------------------------------------------------------------------
SoundSystem::SoundSystem(Gameexe& gexe)
  : m_bgmEnabled(true),
    m_bgmVolume(255),
    m_bgmStatus(0),
    m_pcmEnabled(true),
    m_pcmVolume(255),
    m_seEnabled(true),
    m_seVolume(255)
{
  std::fill_n(m_channelVolume, NUM_BASE_CHANNELS, 255);

  // Read the #SE.xxx entries from the Gameexe
  GameexeFilteringIterator it = gexe.filtering_begin("SE.");
  GameexeFilteringIterator end = gexe.filtering_end();
  for(; it != end; ++it)
  {
    string rawNumber = it->key().substr(it->key().find_first_of(".") + 1);
    int entryNumber = lexical_cast<int>(rawNumber);

    string fileName = it->getStringAt(0);
    int targetChannel = it->getIntAt(1);

    m_seTable[entryNumber] = make_pair(fileName, targetChannel);
  }
}

// -----------------------------------------------------------------------

SoundSystem::~SoundSystem()
{}

// -----------------------------------------------------------------------

void SoundSystem::executeSoundSystem(RLMachine& machine)
{
  unsigned int curTime = machine.system().event().getTicks();

  ChannelAdjustmentMap::iterator it = m_pcmAdjustmentTasks.begin();
  while(it != m_pcmAdjustmentTasks.end())
  {
    if(curTime >= it->second.endTime) 
    {
      setChannelVolume(it->first, it->second.finalVolume);
      m_pcmAdjustmentTasks.erase(it++);
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

void SoundSystem::setPcmEnabled(const int in)
{
  m_pcmEnabled = in;
}

// -----------------------------------------------------------------------

int SoundSystem::pcmEnabled() const
{
  return m_pcmEnabled;
}

// -----------------------------------------------------------------------

void SoundSystem::setPcmVolume(const int in)
{
  m_pcmVolume = in;
}

// -----------------------------------------------------------------------

int SoundSystem::pcmVolume() const
{
  return m_pcmVolume;
}

// -----------------------------------------------------------------------

void SoundSystem::setChannelVolume(const int channel, const int level)
{
  checkChannel(channel, "setChannelVolume");
  checkVolume(level, "setChannelVolume");

  m_channelVolume[channel] = level; 
}

// -----------------------------------------------------------------------

void SoundSystem::setChannelVolume(
  RLMachine& machine, const int channel, const int level, 
  const int fadeTimeInMs)
{
  checkChannel(channel, "setChannelVolume");
  checkVolume(level, "setChannelVolume");

  unsigned int curTime = machine.system().event().getTicks();

  m_pcmAdjustmentTasks.insert(
    make_pair(channel, VolumeAdjustTask(curTime, m_channelVolume[channel], level, fadeTimeInMs)));
}

// -----------------------------------------------------------------------

int SoundSystem::channelVolume(const int channel)
{
  checkChannel(channel, "channelVolume");
  return m_channelVolume[channel];
}

// -----------------------------------------------------------------------

void SoundSystem::setSeEnabled(const int in)
{
  m_seEnabled = in;
}

// -----------------------------------------------------------------------

int SoundSystem::seEnabled() const
{
  return m_seEnabled;
}

// -----------------------------------------------------------------------

void SoundSystem::setSeVolume(const int level)
{
  checkVolume(level, "setSeVolume");
  m_seVolume = level;
}

// -----------------------------------------------------------------------

int SoundSystem::seVolume() const
{
  return m_seVolume;
}
