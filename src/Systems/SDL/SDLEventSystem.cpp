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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// -----------------------------------------------------------------------

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/EventHandler.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/SDL/SDLEventSystem.hpp"

#include <SDL/SDL.h>
#include <boost/bind.hpp>

#include <iostream>

using namespace std;

using boost::bind;

// -----------------------------------------------------------------------
// Private implementation
// -----------------------------------------------------------------------

void SDLEventSystem::handleKeyDown(SDL_Event& event)
{
  switch(event.key.keysym.sym)
  {
  case SDLK_LSHIFT:
  case SDLK_RSHIFT:
  {
    shift_pressed_ = true;
    break;
  }
  case SDLK_LCTRL:
  case SDLK_RCTRL:
  {
    ctrl_pressed_ = true;
    break;
  }
  default:
    break;
  }

  KeyCode code = KeyCode(event.key.keysym.sym);
  dispatchEvent(bind(&EventHandler::keyStateChanged, _1, code, true));
}

// -----------------------------------------------------------------------

void SDLEventSystem::handleKeyUp(RLMachine& machine, SDL_Event& event)
{
  switch(event.key.keysym.sym)
  {
  case SDLK_LSHIFT:
  case SDLK_RSHIFT:
  {
    shift_pressed_ = false;
    break;
  }
  case SDLK_LCTRL:
  case SDLK_RCTRL:
  {
    ctrl_pressed_ = false;
    break;
  }
  case SDLK_F12:
  {
    machine.system().dumpRenderTree(machine);
    break;
  }
  default:
    break;
  }

  KeyCode code = KeyCode(event.key.keysym.sym);
  dispatchEvent(bind(&EventHandler::keyStateChanged, _1, code, false));
}

// -----------------------------------------------------------------------

void SDLEventSystem::handleMouseMotion(SDL_Event& event)
{
  if(mouse_inside_window_)
  {
    // Handle this somehow.
    mouse_pos_ = Point(event.motion.x, event.motion.y);

    // Handle this somehow.
    broadcastEvent(bind(&EventHandler::mouseMotion, _1, mouse_pos_));
  }
}

// -----------------------------------------------------------------------

void SDLEventSystem::handleMouseButtonEvent(SDL_Event& event)
{
  if(mouse_inside_window_)
  {
    bool pressed = event.type == SDL_MOUSEBUTTONDOWN;
    int press_code = pressed ? 1 : 2;

    if(event.button.button == SDL_BUTTON_LEFT)
      m_button1State = press_code;
    else if(event.button.button == SDL_BUTTON_RIGHT)
      m_button2State = press_code;

    MouseButton button = MOUSE_NONE;
    switch(event.button.button)
    {
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

    dispatchEvent(bind(&EventHandler::mouseButtonStateChanged, _1,
                         button, pressed));
    unaccessed_items_ = true;
  }
}

// -----------------------------------------------------------------------

void SDLEventSystem::handleActiveEvent(RLMachine& machine, SDL_Event& event)
{
  if(event.active.state & SDL_APPINPUTFOCUS) {
    mouse_inside_window_ = SDL_GetAppState() & SDL_APPMOUSEFOCUS;

    machine.system().graphics().markScreenAsDirty(GUT_MOUSE_MOTION);
  }
  else if(event.active.state & SDL_APPMOUSEFOCUS) {
    mouse_inside_window_ = event.active.gain == 1;

    // Force a mouse refresh:
    machine.system().graphics().markScreenAsDirty(GUT_MOUSE_MOTION);
  }
}

// -----------------------------------------------------------------------
// Public implementation
// -----------------------------------------------------------------------

SDLEventSystem::SDLEventSystem(Gameexe& gexe)
  : EventSystem(gexe), shift_pressed_(false), ctrl_pressed_(false),
    mouse_inside_window_(true),
    unaccessed_items_(false), mouse_pos_(),
    m_button1State(0), m_button2State(0)
{}

// -----------------------------------------------------------------------

void SDLEventSystem::executeEventSystem(RLMachine& machine)
{
  EventSystem::executeEventSystem(machine);

  SDL_Event event;
  while(SDL_PollEvent(&event))
  {
    switch(event.type)
    {
    case SDL_KEYDOWN:
    {
      handleKeyDown(event);
      break;
    }
    case SDL_KEYUP:
    {
      handleKeyUp(machine, event);
      break;
    }
    case SDL_MOUSEMOTION:
    {
      handleMouseMotion(event);
      break;
    }
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
    {
      handleMouseButtonEvent(event);
      break;
    }
    case SDL_QUIT:
      machine.halt();
      break;
    case SDL_ACTIVEEVENT:
      handleActiveEvent(machine, event);
      break;
    case SDL_VIDEOEXPOSE:
    {
      machine.system().graphics().forceRefresh();
      break;
    }
    }
  }
}

// -----------------------------------------------------------------------

Point SDLEventSystem::getCursorPos()
{
  return mouse_pos_;
}

// -----------------------------------------------------------------------

void SDLEventSystem::getCursorPos(Point& position, int& button1,
                                  int& button2)
{
  position = mouse_pos_;
  button1 = m_button1State;
  button2 = m_button2State;

  unaccessed_items_ = false;
}

// -----------------------------------------------------------------------

void SDLEventSystem::flushMouseClicks()
{
  if(!unaccessed_items_)
  {
    m_button1State = 0;
    m_button2State = 0;
    unaccessed_items_ = false;
  }
}

// -----------------------------------------------------------------------

unsigned int SDLEventSystem::getTicks() const
{
  return SDL_GetTicks();
}

// -----------------------------------------------------------------------

void SDLEventSystem::wait(unsigned int milliseconds) const
{
  SDL_Delay(milliseconds);
}

// -----------------------------------------------------------------------

void SDLEventSystem::injectMouseMovement(const Point& loc) {
  mouse_pos_ = loc;
  broadcastEvent(bind(&EventHandler::mouseMotion, _1, mouse_pos_));
}

// -----------------------------------------------------------------------

void SDLEventSystem::injectMouseDown() {
  m_button1State = 1;
  m_button2State = 0;

  dispatchEvent(bind(&EventHandler::mouseButtonStateChanged, _1,
                     MOUSE_LEFT, 1));
  unaccessed_items_ = true;
}

// -----------------------------------------------------------------------

void SDLEventSystem::injectMouseUp() {
  m_button1State = 2;
  m_button2State = 0;

  dispatchEvent(bind(&EventHandler::mouseButtonStateChanged, _1,
                     MOUSE_LEFT, 1));
  unaccessed_items_ = true;
}
