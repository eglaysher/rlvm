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

#ifndef SRC_SYSTEMS_SDL_SDLEVENTSYSTEM_HPP_
#define SRC_SYSTEMS_SDL_SDLEVENTSYSTEM_HPP_

#include "Systems/Base/EventSystem.hpp"
#include "Systems/Base/Rect.hpp"

#include <SDL/SDL_events.h>

class SDLSystem;

class SDLEventSystem : public EventSystem {
 public:
  SDLEventSystem(SDLSystem& sys, Gameexe& gexe);

  // We provide this accessor to let the Graphics system querry what
  // to do when redrawing the mouse.
  bool mouseInsideWindow() const { return mouse_inside_window_; }

  // Implementation of EventSystem:
  virtual void executeEventSystem(RLMachine& machine);
  virtual void wait(unsigned int milliseconds) const;
  virtual bool shiftPressed() const { return shift_pressed_; }
  virtual bool ctrlPressed() const;
  virtual Point getCursorPos();
  virtual void getCursorPos(Point& position, int& button1, int& button2);
  virtual void flushMouseClicks();
  virtual unsigned int timeOfLastMouseMove();
  virtual void injectMouseMovement(RLMachine& machine, const Point& loc);
  virtual void injectMouseDown(RLMachine& machine);
  virtual void injectMouseUp(RLMachine& machine);

 private:
  // Called from getCursorPos() functions to force a pause if it's been less
  // than 10ms since the last getCursorPos() call.
  void preventCursorPosSpinning();

  // RealLive event system commands
  void handleKeyDown(RLMachine& machine, SDL_Event& event);
  void handleKeyUp(RLMachine& machine, SDL_Event& event);
  void handleMouseMotion(RLMachine& machine, SDL_Event& event);
  void handleMouseButtonEvent(RLMachine& machine, SDL_Event& event);
  void handleActiveEvent(RLMachine& machine, SDL_Event& event);

  virtual unsigned int getTicksImpl() const;

  bool shift_pressed_, ctrl_pressed_;

  // Prevent a mouseup with no matching mousedown when the window is focused.
  bool prevent_orphan_mouse_up_after_focus_;

  // Whether the mouse cursor is currently inside the window bounds.
  bool mouse_inside_window_;

  Point mouse_pos_;

  int m_button1State, m_button2State;

  // The last time a getCursorPos() function was called.
  unsigned int last_get_currsor_time_;

  // The last time we received a mouse move notification.
  unsigned int last_mouse_move_time_;

  // Our owning system.
  SDLSystem& system_;
};

#endif  // SRC_SYSTEMS_SDL_SDLEVENTSYSTEM_HPP_
