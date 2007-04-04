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


#include "Systems/Base/TextSystem.hpp"
#include "Systems/Base/TextPage.hpp"

#include <iostream>

using namespace std;

TextSystem::TextSystem()
  : m_fastTextMode(false), m_messageNoWait(false), m_messageSpeed(0),
    m_defaultTextWindow(0), m_inPauseState(false)
{
  
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
}

// -----------------------------------------------------------------------

void TextSystem::backPage(RLMachine& machine)
{
  if(m_previousPageIt != m_previousPages.begin())
  {
    cerr << "Moving back a page." << endl;
    m_previousPageIt = boost::prior(m_previousPageIt);

    // Clear all windows
    clearAllTextWindows();
    hideAllTextWindows();

    m_previousPageIt->replay();
  }
}

// -----------------------------------------------------------------------

void TextSystem::forwardPage(RLMachine& machine)
{
  if(m_previousPageIt != m_previousPages.end())
  {
    cerr << "Moving forward a page." << endl;
    m_previousPageIt = boost::next(m_previousPageIt);

    // Clear all windows
    clearAllTextWindows();
    hideAllTextWindows();

    if(m_previousPageIt != m_previousPages.end())
      m_previousPageIt->replay();
    else
      m_activePage->replay();
  }
}

// -----------------------------------------------------------------------

bool TextSystem::isReadingBacklog() const
{
  return m_previousPageIt != m_previousPages.end();
}
