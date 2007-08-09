// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006, 2007 Elliot Glaysher
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

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

#include <iostream>
#include <sstream>

#include "MachineBase/RLMachine.hpp"

#include "Systems/Base/GraphicsObject.hpp"
#include "Systems/Base/GraphicsObjectData.hpp"
#include "Systems/SDL/SDLSystem.hpp"
#include "Systems/SDL/SDLGraphicsSystem.hpp"
#include "Systems/SDL/SDLEventSystem.hpp"
#include "Systems/SDL/SDLTextSystem.hpp"

#include <SDL/SDL.h>

#include "libReallive/defs.h"
#include "libReallive/gameexe.h"

using namespace std;
using namespace libReallive;

SDLSystem::SDLSystem(Gameexe& gameexe)
  : m_gameexe(gameexe), m_lastTimePaused(0)
{
  // First, initialize SDL's video subsystem.
  if( SDL_Init( SDL_INIT_VIDEO) < 0 )
  {
    ostringstream ss;
    ss << "Video initialization failed: " << SDL_GetError();
    throw Error(ss.str());
  } 

  // Initialize the various subsystems
  graphicsSystem.reset(new SDLGraphicsSystem(gameexe));
  eventSystem.reset(new SDLEventSystem(gameexe));
  textSystem.reset(new SDLTextSystem(gameexe));
}

SDLSystem::~SDLSystem()
{
}

void SDLSystem::run(RLMachine& machine)
{
  // Give the event handler a chance to run.
  eventSystem->executeEventSystem(machine);

  textSystem->executeTextSystem(machine);

  // Only run the graphics system every 5 ms.
  graphicsSystem->executeGraphicsSystem(machine);

  // My pausing model is wrong. Really wrong. For an example of just
  // how wrong it is, take a look at the performance under CLANNAD's menu. 

  if(machine.inLongOperation())
  {
    eventSystem->wait(10);
  }
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

TextSystem& SDLSystem::text()
{
  return *textSystem;
}
