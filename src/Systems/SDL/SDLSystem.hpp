// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006 Elliot Glaysher
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

#ifndef __SDLSystem_hpp__
#define __SDLSystem_hpp__

#include <boost/scoped_ptr.hpp>

#include "Systems/Base/System.hpp"

class SDLGraphicsSystem;
class SDLEventSystem;
class SDLTextSystem;
class SDLSoundSystem;

// -----------------------------------------------------------------------

class SDLSystem : public System
{
private:
  boost::scoped_ptr<SDLGraphicsSystem> graphicsSystem;
  boost::scoped_ptr<SDLEventSystem> eventSystem;
  boost::scoped_ptr<SDLTextSystem> textSystem;
  boost::scoped_ptr<SDLSoundSystem> soundSystem;
  Gameexe& m_gameexe;

  unsigned int m_lastTimePaused;

public:
  SDLSystem(Gameexe& gameexe);
  ~SDLSystem();

  virtual void run(RLMachine& machine);
  virtual GraphicsSystem& graphics();
  virtual EventSystem& event();
  virtual Gameexe& gameexe();
  virtual TextSystem& text();
  virtual SoundSystem& sound();
};

#endif
