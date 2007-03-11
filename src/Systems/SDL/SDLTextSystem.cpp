// This file is part of RLVM, a RealLive virtual machine clone.
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

/**
 * @file   SDLTextSystem.cpp
 * @author Elliot Glaysher
 * @date   Wed Mar  7 22:04:25 2007
 * 
 * @brief  SDL specialization of the text system
 */

#include "Systems/SDL/SDLTextSystem.hpp"
#include "Systems/SDL/SDLTextWindow.hpp"

SDLTextSystem::SDLTextSystem()
{}

// -----------------------------------------------------------------------

SDLTextSystem::~SDLTextSystem()
{}

// -----------------------------------------------------------------------

void SDLTextSystem::setActiveTextWindow(RLMachine& machine, int window)
{
  m_textWindow.reset(new SDLTextWindow(machine, window));
}

// -----------------------------------------------------------------------

void SDLTextSystem::render(RLMachine& machine)
{
  if(m_textWindow)
    m_textWindow->render(machine);
}

// -----------------------------------------------------------------------

TextWindow& SDLTextSystem::activeTextWindow()
{
  return *m_textWindow;
}
