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

#include "Systems/Base/SystemError.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/TextSystem.hpp"
#include "Systems/Base/TextPage.hpp"
#include "Systems/Base/TextKeyCursor.hpp"

#include "libReallive/gameexe.h"
#include "Utilities.h"

#include <boost/bind.hpp>

#include <iostream>
#include <sstream>

using std::vector;
using std::ostringstream;
using boost::bind;
using boost::shared_ptr;

// -----------------------------------------------------------------------
// TextSystem
// -----------------------------------------------------------------------
TextSystem::TextSystem(Gameexe& gexe)
  : m_autoMode(false),  m_autoModeBaseTime(1000), m_autoModeCharTime(100),

    m_ctrlKeySkip(true), m_fastTextMode(false),
    m_messageNoWait(false),
    m_messageSpeed(0), m_activeWindow(0), m_isReadingBacklog(false),

    m_currentPageset(new PageSet),

    m_inPauseState(false),

    // #WINDOW_*_USE
    m_moveUse(false), m_clearUse(false), m_readJumpUse(false),
    m_automodeUse(false), m_msgbkUse(false), m_msgbkleftUse(false),
    m_msgbkrightUse(false), m_exbtnUse(false)
{
  GameexeInterpretObject ctrlUse(gexe("CTRL_USE"));
  if(ctrlUse.exists())
    m_ctrlKeySkip = ctrlUse;

  GameexeInterpretObject windowAttr(gexe("WINDOW_ATTR"));
  if(windowAttr.exists())
    setDefaultWindowAttr(windowAttr);

  checkAndSetBool(gexe, "WINDOW_MOVE_USE", m_moveUse);
  checkAndSetBool(gexe, "WINDOW_CLEAR_USE", m_clearUse);
  checkAndSetBool(gexe, "WINDOW_READJUMP_USE", m_readJumpUse);
  checkAndSetBool(gexe, "WINDOW_AUTOMODE_USE", m_automodeUse);
  checkAndSetBool(gexe, "WINDOW_MSGBK_USE", m_msgbkUse);
  checkAndSetBool(gexe, "WINDOW_MSGBKLEFT_USE", m_msgbkleftUse);
  checkAndSetBool(gexe, "WINDOW_MSGBKRIGHT_USE", m_msgbkrightUse);
  checkAndSetBool(gexe, "WINDOW_EXBTN_USE", m_exbtnUse);

  m_previousPageIt = m_previousPageSets.end();
}

// -----------------------------------------------------------------------

TextSystem::~TextSystem()
{
  
}

// -----------------------------------------------------------------------

boost::shared_ptr<TTF_Font> TextSystem::getFontOfSize(int size)
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

TextWindow& TextSystem::currentWindow(RLMachine& machine)
{
  return textWindow(machine, m_activeWindow);
}

// -----------------------------------------------------------------------

void TextSystem::checkAndSetBool(Gameexe& gexe, const std::string& key,
                                 bool& out)
{
  GameexeInterpretObject keyObj(gexe(key));
  if(keyObj.exists())
    out = keyObj.to_int();
}

// -----------------------------------------------------------------------

vector<int> TextSystem::activeWindows()
{
  vector<int> tmp;
  for(PageSet::iterator it = m_currentPageset->begin(); 
      it != m_currentPageset->end(); ++it)
  {
    tmp.push_back(it.key());
  }
  return tmp;
}

// -----------------------------------------------------------------------

void TextSystem::snapshot(RLMachine& machine)
{
  m_previousPageSets.push_back(m_currentPageset->clone().release());
}

// -----------------------------------------------------------------------

void TextSystem::newPageOnWindow(RLMachine& machine, int window)
{
  // Erase the current instance of this window if it exists
  PageSet::iterator it = m_currentPageset->find(window);
  if(it != m_currentPageset->end())
  {
    m_currentPageset->erase(it);
  }

  m_previousPageIt = m_previousPageSets.end();
  m_currentPageset->insert(window, new TextPage(machine, window));
} 

// -----------------------------------------------------------------------

TextPage& TextSystem::currentPage(RLMachine& machine)
{
  // Check to see if the active window has a current page.
  PageSet::iterator it = m_currentPageset->find(m_activeWindow);
  if(it == m_currentPageset->end())
    it = m_currentPageset->insert(
      m_activeWindow, new TextPage(machine, m_activeWindow)).first;

  return *it;
}

// -----------------------------------------------------------------------

void TextSystem::backPage(RLMachine& machine)
{
  m_isReadingBacklog = true;

  if(m_previousPageIt != m_previousPageSets.begin())
  {
    m_previousPageIt = boost::prior(m_previousPageIt);

    // Clear all windows
    clearAllTextWindows();      
    hideAllTextWindows();       

    replayPageSet(*m_previousPageIt, false);
  }
}

// -----------------------------------------------------------------------

void TextSystem::forwardPage(RLMachine& machine)
{
  m_isReadingBacklog = true;

  if(m_previousPageIt != m_previousPageSets.end())
  {
    m_previousPageIt = boost::next(m_previousPageIt);

    // Clear all windows
    clearAllTextWindows();
    hideAllTextWindows();

    if(m_previousPageIt != m_previousPageSets.end())
      replayPageSet(*m_previousPageIt, false);
    else
      replayPageSet(*m_currentPageset, false);
  }
}

// -----------------------------------------------------------------------

void TextSystem::replayPageSet(PageSet& set, bool isCurrentPage)
{
  for_each(set.begin(), set.end(),
           bind(&TextPage::replay, _1, isCurrentPage));           
}

// -----------------------------------------------------------------------

bool TextSystem::isReadingBacklog() const
{
  return m_isReadingBacklog;
}

// -----------------------------------------------------------------------

void TextSystem::stopReadingBacklog()
{
  m_isReadingBacklog = false;

  // Clear all windows
  clearAllTextWindows();
  hideAllTextWindows();  
  replayPageSet(*m_currentPageset, true);
}

// -----------------------------------------------------------------------

int TextSystem::getAutoTime(int numChars)
{
  return m_autoModeBaseTime + m_autoModeCharTime * numChars;
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

