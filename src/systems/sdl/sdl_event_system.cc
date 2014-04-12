// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006, 2007 Elliot Glaysher
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

#include "systems/sdl/sdl_event_system.h"

#include <SDL/SDL.h>

#include <functional>

#include "machine/rlmachine.h"
#include "systems/base/event_listener.h"
#include "systems/base/graphics_system.h"
#include "systems/sdl/sdl_system.h"

using std::bind;
using std::placeholders::_1;

SDLEventSystem::SDLEventSystem(SDLSystem& sys, Gameexe& gexe)
    : EventSystem(gexe),
      shift_pressed_(false),
      ctrl_pressed_(false),
      mouse_inside_window_(true),
      mouse_pos_(),
      button1_state_(0),
      button2_state_(0),
      last_get_currsor_time_(0),
      last_mouse_move_time_(0),
      system_(sys),
      raw_handler_(NULL) {}

void SDLEventSystem::ExecuteEventSystem(RLMachine& machine) {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_KEYDOWN: {
        if (raw_handler_)
          raw_handler_->pushInput(event);
        else
          HandleKeyDown(machine, event);
        break;
      }
      case SDL_KEYUP: {
        if (raw_handler_)
          raw_handler_->pushInput(event);
        else
          HandleKeyUp(machine, event);
        break;
      }
      case SDL_MOUSEMOTION: {
        if (raw_handler_)
          raw_handler_->pushInput(event);
        HandleMouseMotion(machine, event);
        break;
      }
      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEBUTTONUP: {
        if (raw_handler_)
          raw_handler_->pushInput(event);
        else
          HandleMouseButtonEvent(machine, event);
        break;
      }
      case SDL_QUIT:
        machine.Halt();
        break;
      case SDL_ACTIVEEVENT:
        if (raw_handler_)
          raw_handler_->pushInput(event);
        HandleActiveEvent(machine, event);
        break;
      case SDL_VIDEOEXPOSE: {
        machine.system().graphics().ForceRefresh();
        break;
      }
    }
  }
}

bool SDLEventSystem::CtrlPressed() const {
  return system_.force_fast_forward() || ctrl_pressed_;
}

Point SDLEventSystem::GetCursorPos() {
  PreventCursorPosSpinning();
  return mouse_pos_;
}

void SDLEventSystem::GetCursorPos(Point& position, int& button1, int& button2) {
  PreventCursorPosSpinning();
  position = mouse_pos_;
  button1 = button1_state_;
  button2 = button2_state_;
}

void SDLEventSystem::FlushMouseClicks() {
  button1_state_ = 0;
  button2_state_ = 0;
}

unsigned int SDLEventSystem::TimeOfLastMouseMove() {
  return last_mouse_move_time_;
}

unsigned int SDLEventSystem::GetTicks() const { return SDL_GetTicks(); }

void SDLEventSystem::Wait(unsigned int milliseconds) const {
  SDL_Delay(milliseconds);
}

bool SDLEventSystem::ShiftPressed() const { return shift_pressed_; }

void SDLEventSystem::InjectMouseMovement(RLMachine& machine, const Point& loc) {
  mouse_pos_ = loc;
  BroadcastEvent(machine, bind(&EventListener::MouseMotion, _1, mouse_pos_));
}

void SDLEventSystem::InjectMouseDown(RLMachine& machine) {
  button1_state_ = 1;
  button2_state_ = 0;

  DispatchEvent(
      machine,
      bind(&EventListener::MouseButtonStateChanged, _1, MOUSE_LEFT, 1));
}

void SDLEventSystem::InjectMouseUp(RLMachine& machine) {
  button1_state_ = 2;
  button2_state_ = 0;

  DispatchEvent(
      machine,
      bind(&EventListener::MouseButtonStateChanged, _1, MOUSE_LEFT, 1));
}

void SDLEventSystem::PreventCursorPosSpinning() {
  unsigned int newTime = GetTicks();

  if ((system_.graphics().screen_update_mode() !=
       GraphicsSystem::SCREENUPDATEMODE_MANUAL) &&
      (newTime - last_get_currsor_time_) < 20) {
    // Prevent spinning on input. When we're not in manual mode, we don't get
    // convenient refresh() calls to insert pauses at. Instead, we need to sort
    // of intuit about what's going on and the easiest way to slow down is to
    // track when the bytecode keeps spamming us for the cursor.
    system_.set_force_wait(true);
  }

  last_get_currsor_time_ = newTime;
}

void SDLEventSystem::HandleKeyDown(RLMachine& machine, SDL_Event& event) {
  switch (event.key.keysym.sym) {
    case SDLK_LSHIFT:
    case SDLK_RSHIFT: {
      shift_pressed_ = true;
      break;
    }
    case SDLK_LCTRL:
    case SDLK_RCTRL: {
      ctrl_pressed_ = true;
      break;
    }
    case SDLK_RETURN:
    case SDLK_f: {
      if ((event.key.keysym.mod & KMOD_ALT) ||
          (event.key.keysym.mod & KMOD_META)) {
        machine.system().graphics().ToggleFullscreen();

        // Stop processing because we don't want to Dispatch this event, which
        // might advance the text.
        return;
      }

      break;
    }
    default:
      break;
  }

  KeyCode code = KeyCode(event.key.keysym.sym);
  DispatchEvent(machine, bind(&EventListener::KeyStateChanged, _1, code, true));
}

void SDLEventSystem::HandleKeyUp(RLMachine& machine, SDL_Event& event) {
  switch (event.key.keysym.sym) {
    case SDLK_LSHIFT:
    case SDLK_RSHIFT: {
      shift_pressed_ = false;
      break;
    }
    case SDLK_LCTRL:
    case SDLK_RCTRL: {
      ctrl_pressed_ = false;
      break;
    }
    case SDLK_F1: {
      machine.system().ShowSystemInfo(machine);
      break;
    }
    case SDLK_F12: {
      machine.system().DumpRenderTree(machine);
      break;
    }
    default:
      break;
  }

  KeyCode code = KeyCode(event.key.keysym.sym);
  DispatchEvent(machine,
                bind(&EventListener::KeyStateChanged, _1, code, false));
}

void SDLEventSystem::HandleMouseMotion(RLMachine& machine, SDL_Event& event) {
  if (mouse_inside_window_) {
    mouse_pos_ = Point(event.motion.x, event.motion.y);
    last_mouse_move_time_ = GetTicks();

    // Handle this somehow.
    BroadcastEvent(machine, bind(&EventListener::MouseMotion, _1, mouse_pos_));
  }
}

void SDLEventSystem::HandleMouseButtonEvent(RLMachine& machine,
                                            SDL_Event& event) {
  if (mouse_inside_window_) {
    bool pressed = event.type == SDL_MOUSEBUTTONDOWN;
    int press_code = pressed ? 1 : 2;

    if (event.button.button == SDL_BUTTON_LEFT)
      button1_state_ = press_code;
    else if (event.button.button == SDL_BUTTON_RIGHT)
      button2_state_ = press_code;

    MouseButton button = MOUSE_NONE;
    switch (event.button.button) {
      case SDL_BUTTON_LEFT:
        button = MOUSE_LEFT;
        break;
      case SDL_BUTTON_RIGHT:
        button = MOUSE_RIGHT;
        break;
      case SDL_BUTTON_MIDDLE:
        button = MOUSE_MIDDLE;
        break;
      case SDL_BUTTON_WHEELUP:
        button = MOUSE_WHEELUP;
        break;
      case SDL_BUTTON_WHEELDOWN:
        button = MOUSE_WHEELDOWN;
        break;
      default:
        break;
    }

    DispatchEvent(
        machine,
        bind(&EventListener::MouseButtonStateChanged, _1, button, pressed));
  }
}

void SDLEventSystem::HandleActiveEvent(RLMachine& machine, SDL_Event& event) {
  if (event.active.state & SDL_APPINPUTFOCUS) {
    // Assume the mouse is inside the window. Actually checking the mouse
    // state doesn't work in the case where we mouse click on another window
    // that's partially covered by rlvm's window and then alt-tab back.
    mouse_inside_window_ = true;

    machine.system().graphics().MarkScreenAsDirty(GUT_MOUSE_MOTION);
  } else if (event.active.state & SDL_APPMOUSEFOCUS) {
    mouse_inside_window_ = event.active.gain == 1;

    // Force a mouse refresh:
    machine.system().graphics().MarkScreenAsDirty(GUT_MOUSE_MOTION);
  }
}
