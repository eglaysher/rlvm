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

#include "NullSystem/NullSoundSystem.hpp"

// -----------------------------------------------------------------------
// NullSoundSystem
// -----------------------------------------------------------------------

NullSoundSystem::NullSoundSystem(Gameexe& gexe)
  : SoundSystem(gexe),
    sound_system_log_("NullSoundSystem")
{}

// -----------------------------------------------------------------------

NullSoundSystem::~NullSoundSystem() {}

// -----------------------------------------------------------------------

int NullSoundSystem::bgmStatus() const
{
  sound_system_log_.recordFunction("bgmStatus");
  return 0;
}

// -----------------------------------------------------------------------

void NullSoundSystem::bgmPlay(RLMachine& machine, const std::string& bgmName, 
                              bool loop)
{
  sound_system_log_.recordFunction("bgmPlay", bgmName, loop);
  bgm_name_ = bgmName;
}

// -----------------------------------------------------------------------

void NullSoundSystem::bgmPlay(RLMachine& machine, const std::string& bgmName, 
                              bool loop, int fadeInMs)
{
  sound_system_log_.recordFunction("bgmPlay", bgmName, loop, fadeInMs);
  bgm_name_ = bgmName;
}

// -----------------------------------------------------------------------

void NullSoundSystem::bgmPlay(RLMachine& machine, const std::string& bgmName, bool loop,
                       int fadeInMs, int fadeOutMs)
{
  sound_system_log_.recordFunction("bgmPlay", bgmName, loop, fadeInMs, fadeOutMs);
  bgm_name_ = bgmName;
}

// -----------------------------------------------------------------------

void NullSoundSystem::bgmStop()
{
  sound_system_log_.recordFunction("bgmStop");
}

// -----------------------------------------------------------------------

void NullSoundSystem::bgmPause()
{
  sound_system_log_.recordFunction("bgmPause");
}

// -----------------------------------------------------------------------

void NullSoundSystem::bgmUnPause()
{
  sound_system_log_.recordFunction("bgmUnPause");
}

// -----------------------------------------------------------------------

void NullSoundSystem::bgmFadeOut(int fadeOutMs)
{
  sound_system_log_.recordFunction("bgmFadeOut", fadeOutMs);
}

// -----------------------------------------------------------------------

std::string NullSoundSystem::bgmName() const
{
  sound_system_log_.recordFunction("bgmName");
  return bgm_name_;
}

// -----------------------------------------------------------------------

void NullSoundSystem::wavPlay(RLMachine& machine, const std::string& wavFile, 
                              bool loop)
{
  sound_system_log_.recordFunction("wavPlay", wavFile, loop);
}

// -----------------------------------------------------------------------

void NullSoundSystem::wavPlay(RLMachine& machine, const std::string& wavFile,
                              bool loop, const int channel)
{
  sound_system_log_.recordFunction("wavPlay", wavFile, loop, channel);
}

// -----------------------------------------------------------------------

void NullSoundSystem::wavPlay(RLMachine& machine, const std::string& wavFile,
                              bool loop, const int channel, const int fadeinMs)
{
  sound_system_log_.recordFunction("wavPlay", wavFile, loop, channel, fadeinMs);
}

// -----------------------------------------------------------------------

bool NullSoundSystem::wavPlaying(RLMachine& machine, const int channel)
{
  sound_system_log_.recordFunction("wavPlaying", channel);
  
  // Might want to do something about this eventually.
  return false;
}

// -----------------------------------------------------------------------

void NullSoundSystem::wavStop(const int channel)
{
  sound_system_log_.recordFunction("wavStop", channel);
}

// -----------------------------------------------------------------------

void NullSoundSystem::wavStopAll()
{
  sound_system_log_.recordFunction("wavStopAll");
}

// -----------------------------------------------------------------------

void NullSoundSystem::wavFadeOut(const int channel, const int fadetime)
{
  sound_system_log_.recordFunction("wavFadeOut", channel, fadetime);
}

// -----------------------------------------------------------------------

void NullSoundSystem::playSe(RLMachine& machine, const int seNum)
{
  sound_system_log_.recordFunction("playSe", seNum);
}


