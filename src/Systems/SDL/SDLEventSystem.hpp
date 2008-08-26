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

#ifndef __SDLEventSystem_hpp__
#define __SDLEventSystem_hpp__

#include "Systems/Base/EventSystem.hpp"
#include "Systems/Base/Rect.hpp"

#include <SDL/SDL_events.h>

#include <queue>
#include <boost/function.hpp>

class SDLEventSystem : public EventSystem
{
private:
  bool shift_pressed_, ctrl_pressed_;

  /// Whether the mouse cursor is currently inside the window bounds.
  bool mouse_inside_window_;

  bool unaccessed_items_;

  Point mouse_pos_;

  int m_button1State, m_button2State;

  std::queue<boost::function<void(void)> > queued_actions_;

  /**
   * @name RealLive event system commands
   *
   * @{
   */
  void handleKeyDown(SDL_Event& event);
  void handleKeyUp(SDL_Event& event);
  void handleMouseMotion(SDL_Event& event);
  void handleMouseButtonEvent(SDL_Event& event);
  void handleActiveEvent(RLMachine& machine, SDL_Event& event);
  /// @}

  /**
   * Calls a EventListener member function on all event listeners, and then
   * event handlers, stopping when an object says they handled it.
   */
  void dispatchEvent(const boost::function<bool(EventListener&)>& event);
  void broadcastEvent(const boost::function<void(EventListener&)>& event);

public:
  SDLEventSystem(Gameexe& gexe);

  /// We provide this accessor to let the Graphics system querry what
  /// to do when redrawing the mouse.
  bool mouseInsideWindow() const { return mouse_inside_window_; }

  virtual void executeEventSystem(RLMachine& machine);

  virtual void addEventHandler(EventHandler* handler);
  virtual void removeEventHandler(EventHandler* handler);

  virtual bool shiftPressed() const { return shift_pressed_; }
  virtual bool ctrlPressed() const  { return ctrl_pressed_;  }

  virtual Point getCursorPos();
  virtual void getCursorPos(Point& position, int& button1, int& button2);

  /**
   * Resets the state of the mouse buttons.
   */
  virtual void flushMouseClicks();

  virtual unsigned int getTicks() const;
  virtual void wait(unsigned int milliseconds) const;


//  virtual void setTitle(const  title) const;
};

#endif
