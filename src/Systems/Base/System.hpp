// This file is part of RLVM, a RealLive virtual machine clone.
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

#ifndef __System_hpp__
#define __System_hpp__

class GraphicsSystem;
class EventSystem;
class TextSystem;
class RLMachine;
class Gameexe;

/**
 * The system class provides a generalized interface to all the
 * components that make up a local system that may need to be
 * implemented differently on different systems, i.e., sound,
 * graphics, filesystem et cetera.
 *
 * The base System class is an abstract base class that 
 */
class System
{
public:
  virtual ~System() {}

  virtual void run(RLMachine& machine) = 0;

  virtual GraphicsSystem& graphics() = 0;
  virtual EventSystem& event() = 0;
  virtual Gameexe& gameexe() = 0;
  virtual TextSystem& text() = 0;
//  virtual SoundSystem& soundSystem() = 0;
};

#endif
