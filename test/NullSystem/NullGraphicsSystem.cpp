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

#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/GraphicsObject.hpp"
#include "NullGraphicsSystem.hpp"

// -----------------------------------------------------------------------

NullGraphicsSystem::NullGraphicsSystem(Gameexe& gexe)
  : GraphicsSystem(gexe), graphics_system_log_("NullGraphicsSystem")
{

}

// -----------------------------------------------------------------------

void NullGraphicsSystem::allocateDC(int dc, int width, int height) { 
  graphics_system_log_.recordFunction("allocateDC", dc, width, height);
}

// -----------------------------------------------------------------------

void NullGraphicsSystem::freeDC(int dc) { 
  graphics_system_log_.recordFunction("freeDC", dc);
}

// -----------------------------------------------------------------------

void NullGraphicsSystem::promoteObjects() { 
  graphics_system_log_.recordFunction("promoteObjects");
}

// -----------------------------------------------------------------------

void NullGraphicsSystem::clearAndPromoteObjects() { 
  graphics_system_log_.recordFunction("clearAndPromoteObjects");
}

// -----------------------------------------------------------------------

GraphicsObject& NullGraphicsSystem::getObject(int layer, int objNumber) 
{ 
  static GraphicsObject x; 
  graphics_system_log_.recordFunction("getObject", layer, objNumber);
  return x;
}

// -----------------------------------------------------------------------

boost::shared_ptr<Surface> NullGraphicsSystem::loadSurfaceFromFile(
  const boost::filesystem::path& filename)
{ 
  graphics_system_log_.recordFunction("loadSurfaceFromFile", filename);
  
  // Make this a real surface so we can track what's done with it
  return boost::shared_ptr<Surface>();
}

// -----------------------------------------------------------------------

boost::shared_ptr<Surface> NullGraphicsSystem::getDC(int dc) 
{ 
  graphics_system_log_.recordFunction("getDC", dc);
  return boost::shared_ptr<Surface>(); 
}

// -----------------------------------------------------------------------

boost::shared_ptr<Surface> NullGraphicsSystem::buildSurface(int w, int h)
{
  graphics_system_log_.recordFunction("getDC", w, h);
  return boost::shared_ptr<Surface>(); 
}

// -----------------------------------------------------------------------

void NullGraphicsSystem::blitSurfaceToDC(
  Surface& sourceObj, int targetDC, 
  int srcX, int srcY, int srcWidth, int srcHeight,
  int destX, int destY, int destWidth, int destHeight,
  int alpha) {
  graphics_system_log_.recordFunction(
    "blitSurfaceToDC", targetDC, srcX, srcY, srcWidth, srcHeight, 
    destX, destY, destWidth, destHeight, alpha);
  // TODO
}
