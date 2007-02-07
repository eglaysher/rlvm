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

#include "MachineBase/RLMachine.hpp"
#include "Systems/SDL/SDLEventSystem.hpp"

#include <SDL/SDL.h>

#include <iostream>

using namespace std;

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
  }
}

// -----------------------------------------------------------------------

void SDLEventSystem::handleMouseMotion(SDL_Event& event)
{
  m_mouseXPos = event.motion.x;
  m_mouseYPos = event.motion.y;
//  cerr << "Mouse pos: {" << event.motion.x << ", " << event.motion.y << "}" << endl;
}

// -----------------------------------------------------------------------

void SDLEventSystem::handleMouseButtonDown(SDL_Event& event)
{
  if(event.button.button == SDL_BUTTON_LEFT)
    m_button1State = 1;
  else
    m_button2State = 1;
}

// -----------------------------------------------------------------------

void SDLEventSystem::handleMouseButtonUp(SDL_Event& event)
{
  if(event.button.button == SDL_BUTTON_LEFT)
    m_button1State = 2;
  else
    m_button2State = 2;
}

// -----------------------------------------------------------------------
// Public implementation
// -----------------------------------------------------------------------

SDLEventSystem::SDLEventSystem()
  : m_shiftPressed(false), m_ctrlPressed(false)
{}

// -----------------------------------------------------------------------

void SDLEventSystem::executeEventSystem(RLMachine& machine)
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
      break;
    case SDL_MOUSEBUTTONUP:
      handleMouseButtonUp(event);
      break;
    case SDL_QUIT:
      machine.halt();
      break;
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
  xPos = m_mouseXPos;
  yPos = m_mouseYPos;
  button1 = m_button1State;
  button2 = m_button2State;
}

// -----------------------------------------------------------------------

void SDLEventSystem::flushMouseClicks()
{
  m_button1State = 0;
  m_button2State = 0;
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
