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


#ifndef __SDLSoundSystem_hpp__
#define __SDLSoundSystem_hpp__

#include "Systems/Base/SoundSystem.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/filesystem/operations.hpp>

#include "lru_cache.hpp"
#include <string>

// -----------------------------------------------------------------------

extern "C" struct Mix_Chunk;

/**
 *
 */
class SDLSoundSystem : public SoundSystem
{
private:
  LRUCache<boost::filesystem::path, Mix_Chunk*> m_seCache;

public: 
  SDLSoundSystem(Gameexe& gexe);
  ~SDLSoundSystem();

  virtual void playSe(RLMachine& machine, const int seNum);  
};	// end of class SDLSoundSystem

/**
 * Changes an incoming RealLive volume to the range SDL_Mixer expects.
 *
 * @param inVol RealLive volume (ranged 0-256)
 * @return SDL_Mixer volume (ranged 0-128)
 */
inline int realLiveVolumeToSDLMixerVolume(int inVol)
{
  return inVol / 2;
}

#endif
