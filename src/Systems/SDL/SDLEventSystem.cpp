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

#include "Systems/SDL/SDLEventSystem.hpp"

#include <boost/bind.hpp>
#include <SDL/SDL.h>

#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/EventListener.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/SDL/SDLSystem.hpp"

using boost::bind;

SDLEventSystem::SDLEventSystem(SDLSystem& sys, Gameexe& gexe)
    : EventSystem(gexe), shift_pressed_(false), ctrl_pressed_(false),
      prevent_orphan_mouse_up_after_focus_(false),
      mouse_inside_window_(true),
      mouse_pos_(),
      m_button1State(0),
      m_button2State(0),
      last_get_currsor_time_(0),
      last_mouse_move_time_(0),
      system_(sys),
      raw_handler_(NULL) {
}

void SDLEventSystem::executeEventSystem(RLMachine& machine) {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_KEYDOWN: {
        if (raw_handler_)
          raw_handler_->pushInput(event);
        else
          handleKeyDown(machine, event);
        break;
      }
      case SDL_KEYUP: {
        if (raw_handler_)
          raw_handler_->pushInput(event);
        else
          handleKeyUp(machine, event);
        break;
      }
      case SDL_MOUSEMOTION: {
        if (raw_handler_)
          raw_handler_->pushInput(event);
        handleMouseMotion(machine, event);
        break;
      }
      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEBUTTONUP: {
        if (raw_handler_)
          raw_handler_->pushInput(event);
        else
          handleMouseButtonEvent(machine, event);
        break;
      }
      case SDL_QUIT:
        machine.halt();
        break;
      case SDL_ACTIVEEVENT:
        if (raw_handler_)
          raw_handler_->pushInput(event);
        handleActiveEvent(machine, event);
        break;
      case SDL_VIDEOEXPOSE: {
        machine.system().graphics().forceRefresh();
        break;
      }
    }
  }
}

bool SDLEventSystem::ctrlPressed() const {
  return system_.forceFastForward() || ctrl_pressed_;
}

Point SDLEventSystem::getCursorPos() {
  preventCursorPosSpinning();
  return mouse_pos_;
}

void SDLEventSystem::getCursorPos(Point& position, int& button1,
                                  int& button2) {
  preventCursorPosSpinning();
  position = mouse_pos_;
  button1 = m_button1State;
  button2 = m_button2State;
}

void SDLEventSystem::flushMouseClicks() {
  m_button1State = 0;
  m_button2State = 0;
}

unsigned int SDLEventSystem::timeOfLastMouseMove() {
  return last_mouse_move_time_;
}

void SDLEventSystem::wait(unsigned int milliseconds) const {
  SDL_Delay(milliseconds);
}

void SDLEventSystem::injectMouseMovement(RLMachine& machine, const Point& loc) {
  mouse_pos_ = loc;
  broadcastEvent(machine, bind(&EventListener::mouseMotion, _1, mouse_pos_));
}

void SDLEventSystem::injectMouseDown(RLMachine& machine) {
  m_button1State = 1;
  m_button2State = 0;

  dispatchEvent(machine, bind(&EventListener::mouseButtonStateChanged, _1,
                              MOUSE_LEFT, 1));
}

void SDLEventSystem::injectMouseUp(RLMachine& machine) {
  m_button1State = 2;
  m_button2State = 0;

  dispatchEvent(machine, bind(&EventListener::mouseButtonStateChanged, _1,
                              MOUSE_LEFT, 1));
}

void SDLEventSystem::preventCursorPosSpinning() {
  unsigned int newTime = getTicks();

  if ((system_.graphics().screenUpdateMode() !=
       GraphicsSystem::SCREENUPDATEMODE_MANUAL) &&
      (newTime - last_get_currsor_time_) < 20) {
    // Prevent spinning on input. When we're not in manual mode, we don't get
    // convenient refresh() calls to insert pauses at. Instead, we need to sort
    // of intuit about what's going on and the easiest way to slow down is to
    // track when the bytecode keeps spamming us for the cursor.
    system_.setForceWait(true);
  }

  last_get_currsor_time_ = newTime;
}

void SDLEventSystem::handleKeyDown(RLMachine& machine, SDL_Event& event) {
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
      machine.system().graphics().toggleFullscreen();

      // Stop processing because we don't want to dispatch this event, which
      // might advance the text.
      return;
    }

    break;
  }
  default:
    break;
  }

  KeyCode code = KeyCode(event.key.keysym.sym);
  dispatchEvent(machine, bind(&EventListener::keyStateChanged, _1, code, true));
}

void SDLEventSystem::handleKeyUp(RLMachine& machine, SDL_Event& event) {
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
    machine.system().showSystemInfo(machine);
    break;
  }
  case SDLK_F12: {
    machine.system().dumpRenderTree(machine);
    break;
  }
  default:
    break;
  }

  KeyCode code = KeyCode(event.key.keysym.sym);
  dispatchEvent(machine, bind(&EventListener::keyStateChanged, _1, code,
                              false));
}

void SDLEventSystem::handleMouseMotion(RLMachine& machine, SDL_Event& event) {
  if (mouse_inside_window_) {
    mouse_pos_ = Point(event.motion.x, event.motion.y);
    last_mouse_move_time_ = getTicks();

    // If we're receiving mouse motion events, it's because there's no longer a
    // native dialog.
    system_.graphics().setHideCursorByPausing(false);

    // Handle this somehow.
    broadcastEvent(machine, bind(&EventListener::mouseMotion, _1, mouse_pos_));
  }
}

void SDLEventSystem::handleMouseButtonEvent(RLMachine& machine,
                                            SDL_Event& event) {
  // If we are paused, we want to redirect the event to the Platform to raise
  // the window.
  if (system_.system_paused()) {
    system_.raiseSyscomUI(machine);
    return;
  }

  if (mouse_inside_window_) {
    bool pressed = event.type == SDL_MOUSEBUTTONDOWN;
    int press_code = pressed ? 1 : 2;

    // We may be dealing with an errant mouse-up event after
    if (press_code == 2 && ignore_next_mouseup_event()) {
      set_ignore_next_mouseup_event(false);
      return;
    }
    set_ignore_next_mouseup_event(false);

    if (event.button.button == SDL_BUTTON_LEFT)
      m_button1State = press_code;
    else if (event.button.button == SDL_BUTTON_RIGHT)
      m_button2State = press_code;

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

    dispatchEvent(machine, bind(&EventListener::mouseButtonStateChanged, _1,
                                button, pressed));
  }
}

void SDLEventSystem::handleActiveEvent(RLMachine& machine, SDL_Event& event) {
  if (event.active.state & SDL_APPINPUTFOCUS) {
    // If we are paused, we want to redirect the event to the Platform to raise
    // the window.
    if (system_.system_paused()) {
      system_.raiseSyscomUI(machine);
      return;
    }

    // Assume the mouse is inside the window. Actually checking the mouse
    // state doesn't work in the case where we mouse click on another window
    // that's partially covered by rlvm's window and then alt-tab back.
    mouse_inside_window_ = true;

    machine.system().graphics().markScreenAsDirty(GUT_MOUSE_MOTION);
  } else if (event.active.state & SDL_APPMOUSEFOCUS) {
    mouse_inside_window_ = event.active.gain == 1;

    // Force a mouse refresh:
    machine.system().graphics().markScreenAsDirty(GUT_MOUSE_MOTION);
  }
}

unsigned int SDLEventSystem::getTicksImpl() const {
  return SDL_GetTicks();
}
