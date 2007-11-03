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

#include "GraphicsTextObject.hpp"

#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/TextSystem.hpp"
#include "Systems/Base/Surface.hpp"
#include "Systems/Base/GraphicsObject.hpp"
#include "Utilities.h"

// -----------------------------------------------------------------------

GraphicsTextObject::GraphicsTextObject(RLMachine& machine)
{
}

// -----------------------------------------------------------------------

GraphicsTextObject::~GraphicsTextObject()
{}

// -----------------------------------------------------------------------

void GraphicsTextObject::updateSurface(RLMachine& machine,
                                       GraphicsObject& rp)
{
  m_cachedUtf8str = rp.textText();
  m_surface = machine.system().text().renderText(
    machine, m_cachedUtf8str, rp.textSize(), rp.textXSpace(), 
	rp.textYSpace(), rp.textColour());
}

// -----------------------------------------------------------------------

bool GraphicsTextObject::needsUpdate(GraphicsObject& rp)
{
  return !m_surface || rp.textText() != m_cachedUtf8str;
}

// -----------------------------------------------------------------------

void GraphicsTextObject::render(RLMachine& machine, 
                                GraphicsObject& rp)
{
  if(needsUpdate(rp))
	updateSurface(machine, rp);

  m_surface->renderToScreenAsObject(rp);
}

// -----------------------------------------------------------------------

int GraphicsTextObject::pixelWidth(RLMachine& machine, GraphicsObject& rp)
{
  if(needsUpdate(rp))
    updateSurface(machine, rp);

  return int((rp.width() / 100.0f) * m_surface->width());
}

// -----------------------------------------------------------------------

int GraphicsTextObject::pixelHeight(RLMachine& machine, GraphicsObject& rp)
{
  if(needsUpdate(rp))
    updateSurface(machine, rp);

  return int((rp.height() / 100.0f) * m_surface->height());  
}

// -----------------------------------------------------------------------

GraphicsObjectData* GraphicsTextObject::clone() const
{
  return new GraphicsTextObject(*this);
}
