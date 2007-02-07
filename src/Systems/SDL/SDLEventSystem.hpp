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

#ifndef __SDLEventSystem_hpp__
#define __SDLEventSystem_hpp__

#include "Systems/Base/EventSystem.hpp"

#include <SDL/SDL_events.h>

class SDLEventSystem : public EventSystem
{
private:
  bool m_shiftPressed, m_ctrlPressed;

  int m_mouseXPos, m_mouseYPos;

  int m_button1State, m_button2State;

  void handleKeyDown(SDL_Event& e);
  void handleKeyUp(SDL_Event& e);
  void handleMouseMotion(SDL_Event& e);
  void handleMouseButtonUp(SDL_Event& event);
  void handleMouseButtonDown(SDL_Event& event);

public:
  SDLEventSystem();

  virtual void executeEventSystem(RLMachine& machine);
  
  virtual bool shiftPressed() const { return m_shiftPressed; }
  virtual bool ctrlPressed() const  { return m_ctrlPressed;  }

  virtual void getCursorPos(int& xPos, int& yPos);
  virtual void getCursorPos(int& xPos, int& yPos, int& button1, int& button2);

  /** 
   * Resets the state of the mouse buttons.
   */
  virtual void flushMouseClicks();

  virtual unsigned int getTicks() const;
  virtual void wait(unsigned int milliseconds) const;

//  virtual void setTitle(const  title) const;
};

#endif
