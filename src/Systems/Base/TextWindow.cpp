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

#include "libReallive/defs.h"

#include "Systems/Base/TextWindow.hpp"
#include "Systems/Base/TextWindowButton.hpp"
#include "Systems/Base/TextSystem.hpp"
#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/SystemError.hpp"
#include "Systems/Base/GraphicsSystem.hpp"

#include "libReallive/gameexe.h"
#include "Utilities.h"

#include <vector>
#include <boost/bind.hpp>
#include <iomanip>

using std::cerr;
using std::endl;
using std::ostringstream;
using std::setfill;
using std::setw;
using std::vector;

// -----------------------------------------------------------------------
// TextWindow
// -----------------------------------------------------------------------

TextWindow::TextWindow(RLMachine& machine, int windowNum)
  : m_currentLineNumber(0), m_currentIndentationInPixels(0), 
    m_useIndentation(0), 
    m_r(0), m_g(0), m_b(0), m_alpha(0), m_filter(0), m_isVisible(0),
    m_inSelectionMode(0), m_nextId(0)
{
  Gameexe& gexe = machine.system().gameexe();

  getScreenSize(gexe, m_screenWidth, m_screenHeight);

  // Base form for everything to follow.
  GameexeInterpretObject window(gexe("WINDOW", windowNum));

  // Handle: #WINDOW.index.ATTR_MOD, #WINDOW_ATTR, #WINDOW.index.ATTR
  m_windowAttrMod = window("ATTR_MOD");
  if(m_windowAttrMod == 0)
  {
    setRGBAF(machine.system().text().windowAttr());
  }
  else
    setRGBAF(window("ATTR"));

  setFontSizeInPixels(window("MOJI_SIZE"));
  setWindowSizeInCharacters(window("MOJI_CNT"));
  setSpacingBetweenCharacters(window("MOJI_REP"));
  setRubyTextSize(window("LUBY_SIZE").to_int(0));
  setTextboxPadding(window("MOJI_POS"));

  setWindowPosition(window("POS"));

  setDefaultTextColor(gexe("COLOR_TABLE", 0));

  // INDENT_USE appears to default to on. See the first scene in the
  // game with Nagisa, paying attention to indentation; then check the
  // Gameexe.ini.
  setUseIndentation(window("INDENT_USE").to_int(1));

  setNameMod(window("NAME_MOD").to_int(0));

  setKeycurMod(window("KEYCUR_MOD"));
  setActionOnPause(window("R_COMMAND_MOD"));
}

// -----------------------------------------------------------------------

TextWindow::~TextWindow() {}

// -----------------------------------------------------------------------

void TextWindow::execute(RLMachine& machine)
{
  using namespace boost;

  if(isVisible() && ! machine.system().graphics().interfaceHidden())
  {
    for(ButtonMap::iterator it = m_buttonMap.begin(); it != m_buttonMap.end();
        ++it)
    {
      it->second->execute();
    }
  }
}

// -----------------------------------------------------------------------

void TextWindow::setTextboxPadding(const vector<int>& posData)
{
  m_upperBoxPadding = posData.at(0);
  m_lowerBoxPadding = posData.at(1);
  m_leftBoxPadding = posData.at(2);
  m_rightBoxPadding = posData.at(3);  
}

// -----------------------------------------------------------------------

void TextWindow::setDefaultTextColor(const vector<int>& colorData)
{
  m_defaultRed = colorData.at(0);
  m_defaultGreen = colorData.at(1);
  m_defaultBlue = colorData.at(2);
}

// -----------------------------------------------------------------------

void TextWindow::setFontColor(const vector<int>& colorData)
{
  m_fontRed = colorData.at(0);
  m_fontGreen = colorData.at(1);
  m_fontBlue = colorData.at(2);
}

// -----------------------------------------------------------------------

void TextWindow::setWindowSizeInCharacters(const vector<int>& posData)
{
  m_xWindowSizeInChars = posData.at(0);
  m_yWindowSizeInChars = posData.at(1);
}

// -----------------------------------------------------------------------

void TextWindow::setSpacingBetweenCharacters(const vector<int>& posData)
{
  m_xSpacing = posData.at(0);
  m_ySpacing = posData.at(1);
}

// -----------------------------------------------------------------------

void TextWindow::setWindowPosition(const vector<int>& posData)
{
  m_origin = posData.at(0);
  m_xDistanceFromOrigin = posData.at(1);
  m_yDistanceFromOrigin = posData.at(2);
}

// -----------------------------------------------------------------------

int TextWindow::textWindowWidth() const
{
  return (m_xWindowSizeInChars * 
          (m_fontSizeInPixels + m_xSpacing)) + m_rightBoxPadding;
}

// -----------------------------------------------------------------------

int TextWindow::textWindowHeight() const
{
  return (m_yWindowSizeInChars * 
          (m_fontSizeInPixels + m_ySpacing + m_rubySize)) + m_lowerBoxPadding;
}

// -----------------------------------------------------------------------

int TextWindow::boxX1() const
{
  switch(m_origin)
  {
  case 0:
  case 2:
    return m_xDistanceFromOrigin;
  case 1:
  case 3:
    return m_screenWidth - m_xDistanceFromOrigin - textWindowWidth() -
      m_leftBoxPadding;
  default:
    throw SystemError("Invalid origin");
  };
}   

// -----------------------------------------------------------------------

int TextWindow::boxY1() const
{
  switch(m_origin)
  {
  case 0: // Top and left
  case 1: // Top and right
    return m_yDistanceFromOrigin;
  case 2: // Bottom and left
  case 3: // Bottom and right
    return m_screenHeight - m_yDistanceFromOrigin - textWindowHeight() - 
      m_upperBoxPadding;
  default:
    throw SystemError("Invalid origin");  
  }
}

// -----------------------------------------------------------------------

int TextWindow::textX1() const
{
  switch(m_origin)
  {
  case 0: // Top and left
  case 2: // Bottom and left
    return m_xDistanceFromOrigin + m_leftBoxPadding;
  case 1: // Top and right
  case 3: // Bottom and right
    return m_screenWidth - m_xDistanceFromOrigin - textWindowWidth();
  default:
    throw SystemError("Invalid origin");
  };
}

// -----------------------------------------------------------------------

int TextWindow::textY1() const
{
  switch(m_origin)
  {
  case 0: // Top and left
  case 1: // Top and right
    return m_yDistanceFromOrigin + m_upperBoxPadding;
  case 2: // Bottom and left
  case 3: // Bottom and right
    return m_screenHeight - m_yDistanceFromOrigin - textWindowHeight();
  default:
    throw SystemError("Invalid origin");
  }
}

// -----------------------------------------------------------------------

int TextWindow::textX2() const
{
  return textX1() + textWindowWidth();
}

// -----------------------------------------------------------------------

int TextWindow::textY2() const
{
  return textY1() + textWindowHeight();
}

// -----------------------------------------------------------------------

void TextWindow::setKeycurMod(const vector<int>& keycur)
{
  m_keycursorType = keycur.at(0);
  m_keycursorX = keycur.at(1);
  m_keycursorY = keycur.at(2);
}

// -----------------------------------------------------------------------

int TextWindow::keycursorX() const
{
  switch(m_keycursorType)
  {
  case 0:
    return textX2();
  case 1:
    return m_textInsertionPointX;
  case 2:
    return textX1() + m_keycursorX;
  default:
    throw SystemError("Invalid keycursor type");    
  }
}

// -----------------------------------------------------------------------

int TextWindow::keycursorY() const
{
  switch(m_keycursorType)
  {
  case 0:
    return textY2();
  case 1:
    return m_textInsertionPointY;
  case 2:
    return textY1() + m_keycursorY;
  default:
    throw SystemError("Invalid keycursor type");
  }
}

// -----------------------------------------------------------------------

void readjumpTurnsOn()
{
  cerr << "Readjump (falsly) turn on!" << endl;
}

void readjumpTurnsOff()
{
  cerr << "Readjump (falsly) turn off!" << endl;
}

void automodeTurnsOn()
{
  cerr << "Automode (falsly) turn on!" << endl;
}

void automodeTurnsOff()
{
  cerr << "Automode (falsly) turn off!" << endl;
}

// -----------------------------------------------------------------------


void TextWindow::setWindowWaku(RLMachine& machine, Gameexe& gexe,
                               const int wakuNo)
{
  using namespace boost;

  GameexeInterpretObject waku(gexe("WAKU", wakuNo, 0));

  setWakuMain(machine, waku("NAME"));
  setWakuBacking(machine, waku("BACK"));
  setWakuButton(machine, waku("BTN"));

  TextSystem& ts = machine.system().text();
  GraphicsSystem& gs = machine.system().graphics();
  m_buttonMap.clear();
  // Translation: Boost ptr_map is retarded.
  string key = "MOVE_BOX";
  m_buttonMap.insert(key,
                     new TextWindowButton(ts.windowMoveUse(), waku("MOVE_BOX")));
  key = string("CLEAR_BOX");
  m_buttonMap.insert(key,
                     new ActionTextWindowButton(
                       ts.windowClearUse(), waku("CLEAR_BOX"),
                       bind(&GraphicsSystem::toggleInterfaceHidden, ref(gs))));
  key = string("READJUMP_BOX");
  m_buttonMap.insert(key,
                     new ActivationTextWindowButton(
                       ts.windowReadJumpUse(), waku("READJUMP_BOX"),
                       &readjumpTurnsOn, &readjumpTurnsOff));
  key = string("AUTOMODE_BOX");
  m_buttonMap.insert(key,
                     new ActivationTextWindowButton(
                       ts.windowAutomodeUse(), waku("AUTOMODE_BOX"),
                       bind(&TextSystem::setAutoMode, ref(ts), true),
                       bind(&TextSystem::setAutoMode, ref(ts), false)));

  key = string("MSGBK_BOX");
  m_buttonMap.insert(key,
                     new TextWindowButton(ts.windowMsgbkUse(), waku("MSGBK_BOX")));
  key = string("MSGBKLEFT_BOX");
  m_buttonMap.insert(key,
                     new RepeatActionWhileHoldingWindowButton(
                       ts.windowMsgbkleftUse(), 
                       waku("MSGBKLEFT_BOX"),
                       machine, 
                       bind(&TextSystem::backPage, ref(ts), ref(machine)),
                       250));
  key = string("MSGBKRIGHT_BOX");
  m_buttonMap.insert(key,
                     new RepeatActionWhileHoldingWindowButton(
                       ts.windowMsgbkrightUse(), 
                       waku("MSGBKRIGHT_BOX"),
                       machine,
                       bind(&TextSystem::forwardPage, ref(ts), ref(machine)),
                       250));

  // Read the seven EXBTNs.
  for(int i = 0; i < 7; ++i)
  {
    GameexeInterpretObject wbcall(gexe("WBCALL", i));
    ostringstream oss;
    oss << "EXBTN_" << setw(3) << setfill('0') << i << "_BOX";
    key = oss.str();
    m_buttonMap.insert(key, 
                       new ExbtnWindowButton(machine,
                         ts.windowExbtnUse(),
                         waku(oss.str()), 
                         wbcall));
  }
}

// -----------------------------------------------------------------------

void TextWindow::setRGBAF(const vector<int>& attr)
{
  setR(attr.at(0)); 
  setG(attr.at(1)); 
  setB(attr.at(2));
  setAlpha(attr.at(3));
  setFilter(attr.at(4));
}

// -----------------------------------------------------------------------

void TextWindow::setMousePosition(RLMachine& machine, int x, int y)
{
  using namespace boost;

  for(ButtonMap::iterator it = m_buttonMap.begin(); it != m_buttonMap.end();
      ++it)
  {
    it->second->setMousePosition(machine, *this, x, y);
  }
}

// -----------------------------------------------------------------------

bool TextWindow::handleMouseClick(RLMachine& machine, int x, int y, 
                                  bool pressed)
{
  using namespace boost;

  if(isVisible() && ! machine.system().graphics().interfaceHidden())
  {
    for(ButtonMap::iterator it = m_buttonMap.begin(); it != m_buttonMap.end();
        ++it)
    {
      if(it->second->handleMouseClick(machine, *this, x, y, pressed))
        return true;
    }
  }
  
  return false;
}

// -----------------------------------------------------------------------

void TextWindow::startSelectionMode()
{
  m_inSelectionMode = true;
  m_nextId = 0;
}

// -----------------------------------------------------------------------

void TextWindow::setSelectionCallback(const boost::function<void(int)>& in)
{
  m_selectionCallback = in;
}

// -----------------------------------------------------------------------

void TextWindow::endSelectionMode()
{
  m_inSelectionMode = false;
  m_selectionCallback.clear();
}

// -----------------------------------------------------------------------

const boost::function<void(int)>& TextWindow::selectionCallback()
{
  return m_selectionCallback;
}
