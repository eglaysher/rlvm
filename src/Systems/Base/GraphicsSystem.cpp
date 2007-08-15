// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006, 2007 Elliot Glaysher
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

#include "GraphicsSystem.hpp"
#include "libReallive/gameexe.h"

// -----------------------------------------------------------------------

GraphicsSystem::GraphicsSystem(Gameexe& gameexe) 
  : m_screenUpdateMode(SCREENUPDATEMODE_AUTOMATIC),
	m_displaySubtitle(gameexe("SUBTITLE").to_int(0))
{

}

// -----------------------------------------------------------------------

GraphicsSystem::~GraphicsSystem()
{}

// -----------------------------------------------------------------------

void GraphicsSystem::setWindowSubtitle(const std::string& utf8encoded)
{
  m_subtitle = utf8encoded;
}

// -----------------------------------------------------------------------

const std::string& GraphicsSystem::windowSubtitle() const
{
  return m_subtitle;
}

// -----------------------------------------------------------------------

// Default implementations for some functions (which probably have
// default implementations because I'm lazy, and these really should
// be pure virtual)
void GraphicsSystem::markScreenAsDirty() { }
void GraphicsSystem::markScreenForRefresh() { }
void GraphicsSystem::beginFrame() { }
void GraphicsSystem::endFrame() { }

// -----------------------------------------------------------------------

boost::shared_ptr<Surface> GraphicsSystem::renderToSurfaceWithBg(
  RLMachine& machine, boost::shared_ptr<Surface> bg) 
{ return boost::shared_ptr<Surface>(); }


// -----------------------------------------------------------------------

void GraphicsSystem::saveGlobals(Json::Value& system)
{
  // Right now, does no work
}

// -----------------------------------------------------------------------

void GraphicsSystem::loadGlobals(const Json::Value& system)
{
  // Right now, noop
}
