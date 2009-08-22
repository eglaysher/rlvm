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

/**
 * Hack to ferry SDL_Events over to something like Guichan which wants to take
 * control of the input.
 */
class RawSDLInputHandler {
public:
  virtual void pushInput(SDL_Event event) = 0;
};

class SDLEventSystem : public EventSystem {
public:
  SDLEventSystem(Gameexe& gexe);

  /// We provide this accessor to let the Graphics system querry what
  /// to do when redrawing the mouse.
  bool mouseInsideWindow() const { return mouse_inside_window_; }

  void setRawSDLInputHandler(RawSDLInputHandler* handler) {
    raw_handler_ = handler;
  }

  virtual void executeEventSystem(RLMachine& machine);

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

  virtual void injectMouseMovement(RLMachine& machine, const Point& loc);
  virtual void injectMouseDown(RLMachine& machine);
  virtual void injectMouseUp(RLMachine& machine);

private:
  /**
   * @name RealLive event system commands
   *
   * @{
   */
  void handleKeyDown(RLMachine& machine, SDL_Event& event);
  void handleKeyUp(RLMachine& machine, SDL_Event& event);
  void handleMouseMotion(RLMachine& machine, SDL_Event& event);
  void handleMouseButtonEvent(RLMachine& machine, SDL_Event& event);
  void handleActiveEvent(RLMachine& machine, SDL_Event& event);
  /// @}

  bool shift_pressed_, ctrl_pressed_;

  /// Whether the mouse cursor is currently inside the window bounds.
  bool mouse_inside_window_;

  Point mouse_pos_;

  int m_button1State, m_button2State;

  /// Handles raw SDL events when appropriate. (Used for things like Guichan,
  /// et cetera who want to suck raw SDL events).
  RawSDLInputHandler* raw_handler_;
};

#endif  // SRC_SYSTEMS_SDL_SDLEVENTSYSTEM_HPP_
