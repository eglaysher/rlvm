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

#ifndef __SDLSystem_hpp__
#define __SDLSystem_hpp__

#include <boost/scoped_ptr.hpp>

#include "Systems/Base/System.hpp"

class SDLGraphicsSystem;
class SDLEventSystem;

class SDLSystem : public System
{
private:
  boost::scoped_ptr<SDLGraphicsSystem> graphicsSystem;
  boost::scoped_ptr<SDLEventSystem> eventSystem;
  Gameexe& m_gameexe;

public:
  SDLSystem(Gameexe& gameexe);
  ~SDLSystem();

  virtual void run(RLMachine& machine);
  virtual GraphicsSystem& graphics();
  virtual EventSystem& event();
  virtual Gameexe& gameexe();
};

#endif
