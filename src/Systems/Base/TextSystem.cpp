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

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

#include "Systems/Base/System.hpp"
#include "Systems/Base/TextSystem.hpp"
#include "Systems/Base/TextPage.hpp"
#include "Systems/Base/TextKeyCursor.hpp"

#include "libReallive/gameexe.h"

#include <iostream>

using namespace std;

TextSystem::TextSystem(Gameexe& gexe)
  : m_autoMode(false), m_ctrlKeySkip(true), m_fastTextMode(false),
    m_messageNoWait(false),
    m_messageSpeed(0), m_defaultTextWindow(0), m_inPauseState(false)
{
  GameexeInterpretObject ctrlUse(gexe("CTRL_USE"));
  if(ctrlUse.exists())
    m_ctrlKeySkip = ctrlUse;

  GameexeInterpretObject windowAttr(gexe("WINDOW_ATTR"));
  if(windowAttr.exists())
    setDefaultWindowAttr(windowAttr);
}

// -----------------------------------------------------------------------

TextSystem::~TextSystem()
{
  
}

// -----------------------------------------------------------------------

TextPage& TextSystem::currentPage(RLMachine& machine)
{
  if(!m_activePage.get())
  {
    newPage(machine);
  }

  return *m_activePage;
}

// -----------------------------------------------------------------------

void TextSystem::newPage(RLMachine& machine)
{
  // Add the current page to the backlog
  if(m_activePage.get())
    m_previousPages.push_back(m_activePage.release());

  m_previousPageIt = m_previousPages.end();

  // Clear all windows
  clearAllTextWindows();
  hideAllTextWindows();

  m_activePage.reset(new TextPage(machine));
  m_activePage->setWindow(m_defaultTextWindow);
  m_activePage->addSetToRightStartingColorElement();
}

// -----------------------------------------------------------------------

void TextSystem::backPage(RLMachine& machine)
{
  if(m_previousPageIt != m_previousPages.begin())
  {
    m_previousPageIt = boost::prior(m_previousPageIt);

    // Clear all windows
    clearAllTextWindows();
    hideAllTextWindows();

    m_previousPageIt->replay(false);
  }
}

// -----------------------------------------------------------------------

void TextSystem::forwardPage(RLMachine& machine)
{
  if(m_previousPageIt != m_previousPages.end())
  {
    m_previousPageIt = boost::next(m_previousPageIt);

    // Clear all windows
    clearAllTextWindows();
    hideAllTextWindows();

    if(m_previousPageIt != m_previousPages.end())
      m_previousPageIt->replay(false);
    else
      m_activePage->replay(true);
  }
}

// -----------------------------------------------------------------------

bool TextSystem::isReadingBacklog() const
{
  return m_previousPageIt != m_previousPages.end();
}

// -----------------------------------------------------------------------

void TextSystem::setKeyCursor(RLMachine& machine, int newCursor)
{
  if(!m_textKeyCursor || 
     m_textKeyCursor->cursorNumber() != newCursor)
  {
    m_textKeyCursor.reset(new TextKeyCursor(machine, newCursor));
  }
}

// -----------------------------------------------------------------------

void TextSystem::setDefaultWindowAttr(const std::vector<int>& attr)
{
  m_windowAttr = attr;
}
