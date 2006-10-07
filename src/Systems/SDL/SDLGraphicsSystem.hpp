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

#ifndef __SDLGraphicsSystem_hpp_
#define __SDLGraphicsSystem_hpp_

#include <SDL/SDL.h>

#include <vector>

#include "Systems/Base/GraphicsSystem.hpp"

class SDLGraphicsSystem : public GraphicsSystem
{
private:
  // 
  std::vector<SDL_Surface*> displayContexts;

public:
  SDLGraphicsSystem(); 

  virtual void allocateDC(int dc, int width, int height);
  virtual void freeDC(int dc);

  virtual void wipe(int dc, int r, int g, int b);
};

#endif
