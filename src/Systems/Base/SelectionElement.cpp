// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2007 Elliot Glaysher
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

#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/Surface.hpp"
#include "Systems/Base/SelectionElement.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/Rect.hpp"

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
  int id, const Point& pos)
  : m_isHighlighted(false), m_id(id), m_pos(pos),
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

bool SelectionElement::isHighlighted(const Point& p)
{
  return Rect(m_pos, m_normalImage->size()).contains(p);
}

// -----------------------------------------------------------------------

void SelectionElement::setMousePosition(RLMachine& machine, const Point& pos)
{
  bool startValue = m_isHighlighted;
  m_isHighlighted = isHighlighted(pos);

  if(startValue != m_isHighlighted)
    machine.system().graphics().markScreenAsDirty(GUT_TEXTSYS);
}

// -----------------------------------------------------------------------

bool SelectionElement::handleMouseClick(
  RLMachine& machine, const Point& pos, bool pressed)
{
  if(pressed == false && isHighlighted(pos))
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

  Size s = target->size();

  // POINT
  target->renderToScreen(Rect(Point(0, 0), s), Rect(m_pos, s), 255);
}
