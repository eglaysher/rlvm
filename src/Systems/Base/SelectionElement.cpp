// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2007 Elliot Glaysher
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

#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/Surface.hpp"
#include "Systems/Base/SelectionElement.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/GraphicsSystem.hpp"

#include <iostream>

using namespace std;
using boost::shared_ptr;

// -----------------------------------------------------------------------
// SelectionElement
// -----------------------------------------------------------------------
SelectionElement::SelectionElement(
  const boost::shared_ptr<Surface>& normalImage, 
  const boost::shared_ptr<Surface>& highlightedImage,
  const boost::function<void(int)>& selectionCallback,
  int id, int xpos, int ypos)
  : m_isHighlighted(false), m_id(id), m_x(xpos), m_y(ypos),
    m_normalImage(normalImage),
    m_highlightedImage(highlightedImage),
    m_selectionCallback(selectionCallback)
{
  
}

// -----------------------------------------------------------------------

SelectionElement::~SelectionElement()
{
}

// -----------------------------------------------------------------------

void SelectionElement::setSelectionCallback(
  const boost::function<void(int)>& func)
{
  m_selectionCallback = func;
}

// -----------------------------------------------------------------------

bool SelectionElement::isHighlighted(int x, int y)
{
  return x >= m_x && x <= m_x + m_normalImage->width() &&
    y >= m_y && y <= m_y + m_normalImage->height();
}

// -----------------------------------------------------------------------

void SelectionElement::setMousePosition(RLMachine& machine, int x, int y)
{
  bool startValue = m_isHighlighted;
  m_isHighlighted = isHighlighted(x, y);

  if(startValue != m_isHighlighted)
    machine.system().graphics().markScreenForRefresh();
}

// -----------------------------------------------------------------------

bool SelectionElement::handleMouseClick(
  RLMachine& machine, int x, int y, bool pressed)
{
  if(pressed == false && isHighlighted(x, y))
  {
    // Released within the button
    if(m_selectionCallback)
      m_selectionCallback(m_id);

    return true;
  }
  else
    return false;
}

// -----------------------------------------------------------------------

void SelectionElement::render()
{
  shared_ptr<Surface> target;

  if(m_isHighlighted)
    target = m_highlightedImage;
  else
    target = m_normalImage;

  int width = target->width();
  int height = target->height();

  target->renderToScreen(
    0, 0, width, height,
    m_x, m_y, m_x + width, m_y + height, 255);
}
