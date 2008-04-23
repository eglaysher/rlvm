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

#include "Systems/Base/SoundSystem.hpp"
#include "libReallive/gameexe.h"
#include <boost/lexical_cast.hpp>
#include <iostream>

// -----------------------------------------------------------------------

using namespace std;
using boost::lexical_cast;
using namespace std;

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

void SoundSystem::setSeVolume(const int in)
{
  if(in < 0 || in > 255)
  {
    ostringstream oss;
    oss << "Invalid Se Volume " << in << ". Valid values are 0-255.";
    throw std::runtime_error(oss.str());
  }

  m_seVolume = in;
}

// -----------------------------------------------------------------------

int SoundSystem::seVolume() const
{
  return m_seVolume;
}
