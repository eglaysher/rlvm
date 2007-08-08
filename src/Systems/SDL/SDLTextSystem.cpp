// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006 Elliot Glaysher
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
  if(it != m_textWindow.end() && it->isVisible() && 
     m_inPauseState && !isReadingBacklog())
  {
    if(!m_textKeyCursor)
      setKeyCursor(machine, 0);

    m_textKeyCursor->execute(machine);
  }

  // Let each window update any TextWindowButton s.
  for_each(m_textWindow.begin(), m_textWindow.end(),
           bind(&TextWindow::execute, _1, ref(machine)));
}

// -----------------------------------------------------------------------

void SDLTextSystem::render(RLMachine& machine)
{
  for_each(m_textWindow.begin(), m_textWindow.end(), 
           bind(&TextWindow::render, _1, ref(machine)));

  WindowMap::iterator it = m_textWindow.find(m_activeWindow);

  if(it != m_textWindow.end() && it->isVisible() && 
     m_inPauseState && !isReadingBacklog())
  {
    if(!m_textKeyCursor)
      setKeyCursor(machine, 0);

    m_textKeyCursor->render(machine, *it);
  }
}

// -----------------------------------------------------------------------

void SDLTextSystem::hideTextWindow(int winNumber)
{
  WindowMap::iterator it = m_textWindow.find(winNumber);
  if(it != m_textWindow.end())
  {
    it->setVisible(0);
  }
}

// -----------------------------------------------------------------------

void SDLTextSystem::hideAllTextWindows()
{
  for_each(m_textWindow.begin(), m_textWindow.end(), 
           bind(&TextWindow::setVisible, _1, 0));
}

// -----------------------------------------------------------------------

void SDLTextSystem::clearAllTextWindows()
{
  for_each(m_textWindow.begin(), m_textWindow.end(), 
           bind(&TextWindow::clearWin, _1));
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

  return *it;
}

// -----------------------------------------------------------------------

void SDLTextSystem::updateWindowsForChangeToWindowAttr()
{
  // Check each text window to see if it needs updating
  for(WindowMap::iterator it = m_textWindow.begin(); 
      it != m_textWindow.end(); ++it)
  {
    if(!it->windowAttrMod())
      it->setRGBAF(windowAttr());
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
  for_each(m_textWindow.begin(), m_textWindow.end(),    
           bind(&SDLTextWindow::setMousePosition, _1, 
                ref(machine), x, y));
}

// -----------------------------------------------------------------------

bool SDLTextSystem::handleMouseClick(RLMachine& machine, int x, int y, 
                                     bool pressed)
{
  return find_if(m_textWindow.begin(), m_textWindow.end(),    
           bind(&SDLTextWindow::handleMouseClick, _1, 
                ref(machine), x, y, pressed)) != m_textWindow.end();
}

// -----------------------------------------------------------------------

boost::shared_ptr<Surface> SDLTextSystem::renderText(
  RLMachine& machine, const std::string& utf8str, int size, int xspace,
  int yspace, int colour)
{
  // Pick the correct font
  shared_ptr<TTF_Font> font = machine.system().text().getFontOfSize(size);

  // Pick the correct font colour
  Gameexe& gexe = machine.system().gameexe();
  vector<int> colourVec = gexe("COLOR_TABLE", colour);
  SDL_Color color = {colourVec.at(0), colourVec.at(1), colourVec.at(2)};

  // Naively render. Ignore most of the arguments for now
  if(utf8str.size())
  {
	cerr << "Rendering \"" << utf8str << "\" with size " << size << endl;
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
