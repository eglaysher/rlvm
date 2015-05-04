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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
//
// -----------------------------------------------------------------------

#ifndef SRC_SYSTEMS_SDL_SDL_EVENT_SYSTEM_H_
#define SRC_SYSTEMS_SDL_SDL_EVENT_SYSTEM_H_

#include <SDL/SDL_events.h>

#include "systems/base/event_system.h"
#include "systems/base/rect.h"

class SDLSystem;

// Hack to ferry SDL_Events over to something like Guichan which wants to take
// control of the input.
class RawSDLInputHandler {
 public:
  virtual ~RawSDLInputHandler() {}

  virtual void pushInput(SDL_Event event) = 0;
};

class SDLEventSystem : public EventSystem {
 public:
  SDLEventSystem(SDLSystem& sys, Gameexe& gexe);

  // We provide this accessor to let the Graphics system query what
  // to do when redrawing the mouse.
  bool mouse_inside_window() const { return mouse_inside_window_; }

  void set_raw_sdl_input_handler(RawSDLInputHandler* handler) {
    raw_handler_ = handler;
  }

  // Implementation of EventSystem:
  virtual void ExecuteEventSystem(RLMachine& machine) override;
  virtual unsigned int GetTicks() const override;
  virtual void Wait(unsigned int milliseconds) const override;
  virtual bool ShiftPressed() const override;
  virtual bool CtrlPressed() const override;
  virtual Point GetCursorPos() override;
  virtual void GetCursorPos(Point& position,
                            int& button1,
                            int& button2) override;
  virtual void FlushMouseClicks() override;
  virtual unsigned int TimeOfLastMouseMove() override;
  virtual void InjectMouseMovement(RLMachine& machine,
                                   const Point& loc) override;
  virtual void InjectMouseDown(RLMachine& machine) override;
  virtual void InjectMouseUp(RLMachine& machine) override;

 private:
  // Called from GetCursorPos() functions to force a pause if it's been less
  // than 10ms since the last GetCursorPos() call.
  void PreventCursorPosSpinning();

  // RealLive event system commands
  void HandleKeyDown(RLMachine& machine, SDL_Event& event);
  void HandleKeyUp(RLMachine& machine, SDL_Event& event);
  void HandleMouseMotion(RLMachine& machine, SDL_Event& event);
  void HandleMouseButtonEvent(RLMachine& machine, SDL_Event& event);
  void HandleActiveEvent(RLMachine& machine, SDL_Event& event);

  bool shift_pressed_, ctrl_pressed_;

  // Whether the mouse cursor is currently inside the window bounds.
  bool mouse_inside_window_;

  Point mouse_pos_;

  int button1_state_, button2_state_;

  // The last time a GetCursorPos() function was called.
  unsigned int last_get_currsor_time_;

  // The last time we received a mouse move notification.
  unsigned int last_mouse_move_time_;

  // Our owning system.
  SDLSystem& system_;

  // Handles raw SDL events when appropriate. (Used for things like Guichan,
  // etc who want to suck raw SDL events).
  RawSDLInputHandler* raw_handler_;
};

#endif  // SRC_SYSTEMS_SDL_SDL_EVENT_SYSTEM_H_
