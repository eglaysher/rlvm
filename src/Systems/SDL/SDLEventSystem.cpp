// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006, 2007 Elliot Glaysher
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

void SDLEventSystem::handleKeyDown(SDL_Event& e)
{
  switch(e.key.keysym.sym)
  {
    case SDLK_LSHIFT:
    case SDLK_RSHIFT:
    {
      m_shiftPressed = true;
      break;
    }
    case SDLK_LCTRL:
    case SDLK_RCTRL:
    {
      m_ctrlPressed = true;
      break;
    }
  default:
	break;
  }
}

// -----------------------------------------------------------------------

void SDLEventSystem::handleKeyUp(SDL_Event& e)
{
  switch(e.key.keysym.sym)
  {
    case SDLK_LSHIFT:
    case SDLK_RSHIFT:
    {
      m_shiftPressed = false;
      break;
    }
    case SDLK_LCTRL:
    case SDLK_RCTRL:
    {
      m_ctrlPressed = false;
      break;
    }
  default:
	break;
  }
}

// -----------------------------------------------------------------------

void SDLEventSystem::handleMouseMotion(SDL_Event& event)
{
  m_mouseXPos = event.motion.x;
  m_mouseYPos = event.motion.y;
}

// -----------------------------------------------------------------------

void SDLEventSystem::handleMouseButtonDown(SDL_Event& event)
{
  if(event.button.button == SDL_BUTTON_LEFT)
    m_button1State = 1;
  else if(event.button.button == SDL_BUTTON_RIGHT)
    m_button2State = 1;
}

// -----------------------------------------------------------------------

void SDLEventSystem::handleMouseButtonUp(SDL_Event& event)
{
  if(event.button.button == SDL_BUTTON_LEFT)
    m_button1State = 2;
  else if(event.button.button == SDL_BUTTON_RIGHT)
    m_button2State = 2;
}

// -----------------------------------------------------------------------
// Public implementation
// -----------------------------------------------------------------------

SDLEventSystem::SDLEventSystem(Gameexe& gexe)
  : EventSystem(gexe), m_shiftPressed(false), m_ctrlPressed(false),
    m_unaccessedItems(false), m_mouseXPos(0), m_mouseYPos(0),
	m_button1State(0), m_button2State(0)
{}

// -----------------------------------------------------------------------

void SDLEventSystem::executeEventSystem(RLMachine& machine)
{
  if(handlers_begin() == handlers_end())
    executeRealLiveEventSystem(machine);
  else
    executeEventHandlerSystem(machine);
}

// -----------------------------------------------------------------------

void SDLEventSystem::addEventHandler(EventHandler* handler)
{
  EventSystem::addEventHandler(handler);

  if(m_ctrlPressed)
  {
    m_queuedActions.push(
      bind(&EventHandler::keyStateChanged, handler, RLKEY_LCTRL, true));
  }
}

// -----------------------------------------------------------------------

void SDLEventSystem::removeEventHandler(EventHandler* handler)
{
  EventSystem::removeEventHandler(handler);

  while(m_queuedActions.size())
    m_queuedActions.pop();
}

// -----------------------------------------------------------------------

void SDLEventSystem::executeEventHandlerSystem(RLMachine& machine)
{
  while(m_queuedActions.size())
  {
    m_queuedActions.front()();
    m_queuedActions.pop();
  }

  SDL_Event event;
  while(SDL_PollEvent(&event))
  {
    switch(event.type)
    {
    case SDL_KEYDOWN:
    {
      KeyCode code = KeyCode(event.key.keysym.sym);
      for_each(handlers_begin(), handlers_end(),
               bind(&EventHandler::keyStateChanged, _1,
                    code, true));

      // Still keep track of what keys are held down      
      handleKeyDown(event);
      break;
    }
    case SDL_KEYUP:
    {
      KeyCode code = KeyCode(event.key.keysym.sym);
      for_each(handlers_begin(), handlers_end(),
               bind(&EventHandler::keyStateChanged, _1,
                    code, false));

      // Still keep track of what keys are released
      handleKeyUp(event);
      break;
    }
    case SDL_MOUSEMOTION:
    {
      // Handle this somehow.
      int x = event.motion.x;
      int y = event.motion.y;
      for_each(handlers_begin(), handlers_end(),
               bind(&EventHandler::mouseMotion, _1,
                    x, y));
      handleMouseMotion(event);
      break;
    }
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
    {
      bool pressed = event.type == SDL_MOUSEBUTTONDOWN;
      MouseButton button;
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

      for_each(handlers_begin(), handlers_end(),
               bind(&EventHandler::mouseButtonStateChanged, _1,
                    button, pressed));
      m_unaccessedItems = true;
      break;
    }
    case SDL_QUIT:
      machine.halt();
      break;
    case SDL_VIDEOEXPOSE:
    {
      machine.system().graphics().markScreenAsDirty();
      break;
    }
    }
  }  
}

// -----------------------------------------------------------------------

void SDLEventSystem::executeRealLiveEventSystem(RLMachine& machine)
{
  SDL_Event event;
  while(SDL_PollEvent(&event))
  {
    switch(event.type)
    {
    case SDL_KEYDOWN:
      handleKeyDown(event);
      break;
    case SDL_KEYUP:
      handleKeyUp(event);
      break;
    case SDL_MOUSEMOTION:
      handleMouseMotion(event);
      break;
    case SDL_MOUSEBUTTONDOWN:
      handleMouseButtonDown(event);
      m_unaccessedItems = true;
      break;
    case SDL_MOUSEBUTTONUP:
      handleMouseButtonUp(event);
      m_unaccessedItems = true;
      break;
    case SDL_QUIT:
      machine.halt();
      break;
    case SDL_VIDEOEXPOSE:
    {
      machine.system().graphics().markScreenAsDirty();
      break;
    }
    }
  }
}

// -----------------------------------------------------------------------

void SDLEventSystem::getCursorPos(int& xPos, int& yPos)
{
  xPos = m_mouseXPos;
  yPos = m_mouseYPos;
}

// -----------------------------------------------------------------------

void SDLEventSystem::getCursorPos(int& xPos, int& yPos, int& button1, 
                                  int& button2)
{
//  cerr << "Mouse pos: {" << m_mouseXPos << ", " << m_mouseYPos << "}" << endl;
  xPos = m_mouseXPos;
  yPos = m_mouseYPos;
  button1 = m_button1State;
  button2 = m_button2State;

  m_unaccessedItems = false;
}

// -----------------------------------------------------------------------

void SDLEventSystem::flushMouseClicks()
{
  if(!m_unaccessedItems)
  {
    m_button1State = 0;
    m_button2State = 0;
    m_unaccessedItems = false;
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
