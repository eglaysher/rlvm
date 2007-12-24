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

/**
 * @file   SDLTextSystem.cpp
 * @author Elliot Glaysher
 * @date   Wed Mar  7 22:04:25 2007
 * 
 * @brief  SDL specialization of the text system
 */

#include "MachineBase/RLMachine.hpp"

#include "Systems/SDL/SDLTextSystem.hpp"
#include "Systems/SDL/SDLTextWindow.hpp"
#include "Systems/SDL/SDLSurface.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/SystemError.hpp"
#include "Systems/Base/TextKeyCursor.hpp"

#include "libReallive/gameexe.h"

#include "algoplus.hpp"
#include "Utilities.h"

#include <boost/bind.hpp>
#include "SDL_ttf.h"
#include <sstream>
#include <stdexcept>

using namespace std;
using namespace boost;

// -----------------------------------------------------------------------

SDLTextSystem::SDLTextSystem(Gameexe& gameexe)
  : TextSystem(gameexe)
{
  if(TTF_Init()==-1) {
    ostringstream oss;
    oss << "Error initializing SDL_ttf: " << TTF_GetError();
    throw SystemError(oss.str());
  }
}

// -----------------------------------------------------------------------

SDLTextSystem::~SDLTextSystem()
{
  TTF_Quit();
}

// -----------------------------------------------------------------------

void SDLTextSystem::executeTextSystem(RLMachine& machine)
{
  // Check to see if the cursor is displayed
  WindowMap::iterator it = m_textWindow.find(m_activeWindow);
  if(it != m_textWindow.end() && it->second->isVisible() && 
     m_inPauseState && !isReadingBacklog())
  {
    if(!m_textKeyCursor)
      setKeyCursor(machine, 0);

    m_textKeyCursor->execute(machine);
  }

  // Let each window update any TextWindowButton s.
  for(WindowMap::iterator it = m_textWindow.begin(); it != m_textWindow.end(); ++it)
  {
    it->second->execute(machine);
  }
}

// -----------------------------------------------------------------------

void SDLTextSystem::render(RLMachine& machine)
{
  if(systemVisible())
  {
    for(WindowMap::iterator it = m_textWindow.begin(); it != m_textWindow.end(); ++it)
    {
      it->second->render(machine);
    }

    WindowMap::iterator it = m_textWindow.find(m_activeWindow);

    if(it != m_textWindow.end() && it->second->isVisible() && 
       m_inPauseState && !isReadingBacklog())
    {
      if(!m_textKeyCursor)
        setKeyCursor(machine, 0);

      m_textKeyCursor->render(machine, *it->second);
    }
  }
}

// -----------------------------------------------------------------------

void SDLTextSystem::hideTextWindow(int winNumber)
{
  WindowMap::iterator it = m_textWindow.find(winNumber);
  if(it != m_textWindow.end())
  {
    it->second->setVisible(0);
  }
}

// -----------------------------------------------------------------------

void SDLTextSystem::hideAllTextWindows()
{
  for(WindowMap::iterator it = m_textWindow.begin(); it != m_textWindow.end(); ++it)
  {
    it->second->setVisible(0);
  }
}

// -----------------------------------------------------------------------

void SDLTextSystem::clearAllTextWindows()
{
  for(WindowMap::iterator it = m_textWindow.begin(); it != m_textWindow.end(); ++it)
  {
    it->second->clearWin();
  }
}

// -----------------------------------------------------------------------

TextWindow& SDLTextSystem::textWindow(RLMachine& machine, int textWindow)
{
  WindowMap::iterator it = m_textWindow.find(textWindow);
  if(it == m_textWindow.end())
  {
    it = m_textWindow.insert(
      textWindow, new SDLTextWindow(machine, textWindow)).first;
  }

  return *it->second;
}

// -----------------------------------------------------------------------

void SDLTextSystem::updateWindowsForChangeToWindowAttr()
{
  // Check each text window to see if it needs updating
  for(WindowMap::iterator it = m_textWindow.begin(); 
      it != m_textWindow.end(); ++it)
  {
    if(!it->second->windowAttrMod())
      it->second->setRGBAF(windowAttr());
  }
}

// -----------------------------------------------------------------------

void SDLTextSystem::setDefaultWindowAttr(const std::vector<int>& attr)
{
  TextSystem::setDefaultWindowAttr(attr);
  updateWindowsForChangeToWindowAttr();
}

// -----------------------------------------------------------------------

void SDLTextSystem::setWindowAttrR(int i)
{
  TextSystem::setWindowAttrR(i);
  updateWindowsForChangeToWindowAttr();
}

// -----------------------------------------------------------------------

void SDLTextSystem::setWindowAttrG(int i)
{
  TextSystem::setWindowAttrG(i);
  updateWindowsForChangeToWindowAttr();
}

// -----------------------------------------------------------------------

void SDLTextSystem::setWindowAttrB(int i)
{
  TextSystem::setWindowAttrB(i);
  updateWindowsForChangeToWindowAttr();
}

// -----------------------------------------------------------------------

void SDLTextSystem::setWindowAttrA(int i)
{
  TextSystem::setWindowAttrA(i);
  updateWindowsForChangeToWindowAttr();
}

// -----------------------------------------------------------------------

void SDLTextSystem::setWindowAttrF(int i)
{
  TextSystem::setWindowAttrF(i);
  updateWindowsForChangeToWindowAttr();
}

// -----------------------------------------------------------------------

void SDLTextSystem::setMousePosition(RLMachine& machine, int x, int y)
{
  for(WindowMap::iterator it = m_textWindow.begin(); it != m_textWindow.end(); ++it)
  {
    it->second->setMousePosition(machine, x, y);
  }

//   for_each(m_textWindow.begin(), m_textWindow.end(),    
//            bind(&SDLTextWindow::setMousePosition, _1, 
//                 ref(machine), x, y));
}

// -----------------------------------------------------------------------

bool SDLTextSystem::handleMouseClick(RLMachine& machine, int x, int y, 
                                     bool pressed)
{
  if(systemVisible())
  {
    for(WindowMap::iterator it = m_textWindow.begin(); 
        it != m_textWindow.end(); ++it)
    {
      if(it->second->handleMouseClick(machine, x, y, pressed))
        return true;
    }

    return false;
  }
  else
  {
    return false;
  }
}

// -----------------------------------------------------------------------

boost::shared_ptr<Surface> SDLTextSystem::renderText(
  RLMachine& machine, const std::string& utf8str, int size, int xspace,
  int yspace, int colour)
{
  // Pick the correct font
  shared_ptr<TTF_Font> font = getFontOfSize(size);

  // Pick the correct font colour
  Gameexe& gexe = machine.system().gameexe();
  vector<int> colourVec = gexe("COLOR_TABLE", colour);
  SDL_Color color = {colourVec.at(0), colourVec.at(1), colourVec.at(2)};

  // Naively render. Ignore most of the arguments for now
  if(utf8str.size())
  {
	SDL_Surface* tmp =
	  TTF_RenderUTF8_Blended(font.get(), utf8str.c_str(), color);
	if(tmp == NULL)
	{
	  ostringstream oss;
	  oss << "Error printing \"" << utf8str << "\" in font size " << size;
	  throw rlvm::Exception(oss.str());
	}
	return shared_ptr<Surface>(new SDLSurface(tmp));
  }
  else
  {
	// Allocate a 1x1 SDL_Surface
	return shared_ptr<Surface>(new SDLSurface(buildNewSurface(1, 1)));
  }
}

// -----------------------------------------------------------------------

boost::shared_ptr<TTF_Font> SDLTextSystem::getFontOfSize(int size)
{
  FontSizeMap::iterator it = m_map.find(size);
  if(it == m_map.end())
  {
    string filename = findFontFile("msgothic.ttc");
    TTF_Font* f = TTF_OpenFont(filename.c_str(), size);
    if(f == NULL)
    {
      ostringstream oss;
      oss << "Error loading font: " << TTF_GetError();
      throw SystemError(oss.str());
    }

    TTF_SetFontStyle(f, TTF_STYLE_NORMAL);

    // Build a smart_ptr to own this font, and set a deleter function.
    shared_ptr<TTF_Font> font(f, TTF_CloseFont);

    m_map[size] = font;
    return font;
  }
  else
  {
    return it->second;
  }
}

// -----------------------------------------------------------------------

void SDLTextSystem::reset()
{
  m_textWindow.clear();
  TextSystem::reset();
}
