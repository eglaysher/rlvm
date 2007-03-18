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

#include "libReallive/defs.h"

#include "Systems/Base/TextWindow.hpp"

#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/GraphicsSystem.hpp"

#include "libReallive/gameexe.h"

#include <vector>

using namespace std;

TextWindow::TextWindow()
  : m_r(0), m_g(0), m_b(0), m_alpha(0), m_filter(0), m_isVisible(0), 
    m_useIndentation(0), m_currentIndentationInPixels(0)
{}

// -----------------------------------------------------------------------

void TextWindow::setTextboxPadding(const std::vector<int>& posData)
{
  m_upperBoxPadding = posData.at(0);
  m_lowerBoxPadding = posData.at(1);
  m_leftBoxPadding = posData.at(2);
  m_rightBoxPadding = posData.at(3);  
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

void TextWindow::setWindowPosition(const std::vector<int>& posData)
{
  m_origin = posData.at(0);
  m_xDistanceFromOrigin = posData.at(1);
  m_yDistanceFromOrigin = posData.at(2);
}

// -----------------------------------------------------------------------

int TextWindow::windowWidth() const
{
  return (m_xWindowSizeInChars * 
          (m_fontSizeInPixels + m_xSpacing)) + m_rightBoxPadding;
}

// -----------------------------------------------------------------------

int TextWindow::windowHeight() const
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
  };

  throw libReallive::Error("Invalid origin");
}   

// -----------------------------------------------------------------------

int TextWindow::boxY1() const
{
  switch(m_origin)
  {
  case 0: // Top and left
  case 1: // Top and right
    return m_yDistanceFromOrigin;
  }

  throw libReallive::Error("Invalid origin");  
}

// -----------------------------------------------------------------------

int TextWindow::textX1(RLMachine& machine) const
{
  switch(m_origin)
  {
  case 0: // Top and left
  case 2: // Bottom and left
    return m_xDistanceFromOrigin + m_leftBoxPadding;
//   case 1: // Top and right
//   case 3: // Bottom and right
//     return machine.system().graphics().screenWidth() - m_xDistanceFromOrigin;
  };

  throw libReallive::Error("Invalid origin");
}

// -----------------------------------------------------------------------

int TextWindow::textY1(RLMachine& machine) const
{
  switch(m_origin)
  {
  case 0: // Top and left
  case 1: // Top and right
    return m_yDistanceFromOrigin + m_upperBoxPadding;
//   case 2: // Bottom and left
//   case 3: // Bottom and right
//     return machine.system().graphics().screenHeight() - m_yDistanceFromOrigin;
  }

  throw libReallive::Error("Invalid origin");
}

// -----------------------------------------------------------------------

int TextWindow::textX2(RLMachine& machine) const
{
  return textX1(machine) + windowWidth();
}

// -----------------------------------------------------------------------

int TextWindow::textY2(RLMachine& machine) const
{
  return textY1(machine) + windowHeight();
}


// -----------------------------------------------------------------------

void TextWindow::setWindowWaku(RLMachine& machine, Gameexe& gexe,
                               const int wakuNo)
{
  GameexeInterpretObject waku(gexe("WAKU", wakuNo, 0));

  setWakuMain(machine, waku("NAME"));
  setWakuBacking(machine, waku("BACK"));
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
