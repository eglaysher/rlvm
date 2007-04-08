// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2007 El Riot
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

#include "Systems/Base/TextWindowButton.hpp"
#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/Surface.hpp"
#include "Systems/Base/TextWindow.hpp"

#include <stdexcept>

// -----------------------------------------------------------------------

TextWindowButton::TextWindowButton()
  : m_state(BUTTONSTATE_BUTTON_NOT_USED)  
{
}

// -----------------------------------------------------------------------

TextWindowButton::TextWindowButton(bool useThisButton,
                                   GameexeInterpretObject locationBox)
  : m_state(BUTTONSTATE_BUTTON_NOT_USED)
{
  if(useThisButton && locationBox.exists())
  {
    m_location = locationBox;
    m_state = BUTTONSTATE_NORMAL;
  }
}

// -----------------------------------------------------------------------

TextWindowButton::~TextWindowButton()
{
}

// -----------------------------------------------------------------------

int TextWindowButton::xLocation(TextWindow& window)
{
  int type = m_location.at(0);
  switch(type)
  {
  case 0:
    return window.boxX1() + m_location.at(1);
  default:
    throw std::runtime_error("Unsupported coordinate system"); 
  }
}

// -----------------------------------------------------------------------

int TextWindowButton::yLocation(TextWindow& window)
{
  int type = m_location.at(0);
  switch(type)
  {
  case 0:
    return window.boxY1() + m_location.at(2);
  default:
    throw std::runtime_error("Unsupported coordinate system"); 
  }
}

// -----------------------------------------------------------------------

void TextWindowButton::render(RLMachine& machine, 
                              TextWindow& window,
                              const boost::shared_ptr<Surface>& buttons,
                              int basePattern)
{
  if(m_state != BUTTONSTATE_BUTTON_NOT_USED && m_location.size() == 5 &&
     !(m_location[0] == 0 && m_location[1] == 0 && m_location[2] == 0 &&
       m_location[3] == 0 && m_location[4] == 0))
  {
    Surface::GrpRect rect = buttons->getPattern(basePattern + m_state);
    if(!(rect.x1 == 0 && rect.y1 == 0 && rect.x2 == 0 && rect.y2 == 0))
    {
      int destX = xLocation(window);
      int destY = yLocation(window);
      int width = rect.x2 - rect.x1;
      int height = rect.y2 - rect.y1;

      buttons->renderToScreen(
        rect.x1, rect.y1, rect.x2, rect.y2,
        destX, destY, destX + width, destY + height,
        255);
    }
  }
}
