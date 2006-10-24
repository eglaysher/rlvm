// This file is part of RLVM, a RealLive virutal machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006 El Riot
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

#include <iostream>
#include <sstream>

#include "Systems/SDL/SDLSystem.hpp"
#include "Systems/SDL/SDLGraphicsSystem.hpp"
#include "Systems/SDL/SDLEventSystem.hpp"

#include <SDL/SDL.h>

#include "libReallive/defs.h"
#include "libReallive/gameexe.h"

using namespace std;
using namespace libReallive;

SDLSystem::SDLSystem(Gameexe& gameexe)
  : m_gameexe(gameexe)
{
  // First, initialize SDL's video subsystem.
  if( SDL_Init( SDL_INIT_VIDEO) < 0 )
  {
    stringstream ss;
    ss << "Video initialization failed: " << SDL_GetError();
    throw Error(ss.str());
  } 

  // Initialize the various subsystems
  graphicsSystem.reset(new SDLGraphicsSystem);
  eventSystem.reset(new SDLEventSystem);
}

void SDLSystem::run(RLMachine& machine)
{
  // cerr << "Begin eventSystem: " << event().getTicks() << endl;

  // Give the event handler a chance to run
  eventSystem->executeEventSystem(machine);

//  cerr << "Begin graphicsSystem: " << event().getTicks() << endl;

  // Finally, run any screen updates needed
  graphicsSystem->executeGraphicsSystem();
}

GraphicsSystem& SDLSystem::graphics()
{
  return *graphicsSystem;
}

EventSystem& SDLSystem::event()
{
  return *eventSystem;
}

Gameexe& SDLSystem::gameexe()
{
  return m_gameexe;
}
