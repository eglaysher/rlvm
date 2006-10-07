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

#include <SDL/SDL.h>

#include "libReallive/defs.h"

using namespace std;
using namespace libReallive;

SDLSystem::SDLSystem()
{
  // First, initialize SDL's video subsystem.
  if( SDL_Init( SDL_INIT_VIDEO) < 0 ) {
    stringstream ss;
    ss << "Video initialization failed: " << SDL_GetError();
    throw Error(ss.str());
  } 
  else
    cerr << "SDL initialized!" << endl;

  graphicsSystem.reset(new SDLGraphicsSystem);
}
