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

#ifndef __NullGraphicsSystem_hpp__
#define __NullGraphicsSystem_hpp__

#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/Surface.hpp"

#include "NullSystem/MockLog.hpp"

#include <boost/shared_ptr.hpp>

// -----------------------------------------------------------------------

class NullSurface;

// -----------------------------------------------------------------------

class NullGraphicsSystem : public GraphicsSystem
{
public:
  NullGraphicsSystem(Gameexe& gexe);

  virtual void refresh(RLMachine&) { }

  virtual void executeGraphicsSystem(RLMachine&) { }

  int screenWidth() const { return 640; }
  int screenHeight() const { return 480; }
  virtual Size screenSize() const { return Size(640, 480); }
  virtual void allocateDC(int dc, Size s);
  virtual void freeDC(int dc);

  virtual void promoteObjects();
  virtual void clearAndPromoteObjects();

  virtual GraphicsObject& getObject(int layer, int objNumber);

  // Make a null Surface object?
  virtual boost::shared_ptr<Surface> loadSurfaceFromFile(const boost::filesystem::path& filename);
  virtual boost::shared_ptr<Surface> getDC(int dc);
  virtual boost::shared_ptr<Surface> buildSurface(const Size& s);

  virtual void blitSurfaceToDC(Surface& sourceObj, int targetDC, 
                               int srcX, int srcY, int srcWidth, int srcHeight,
                               int destX, int destY, int destWidth, int destHeight,
                               int alpha = 255);

private:
  /// Record all method calls here
  MockLog graphics_system_log_;

  /// Map between device contexts number and their surface.
  boost::shared_ptr<NullSurface> m_displayContexts[16];
};

#endif
