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

#ifndef TEST_TESTSYSTEM_TESTGRAPHICSSYSTEM_HPP_
#define TEST_TESTSYSTEM_TESTGRAPHICSSYSTEM_HPP_

#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/Surface.hpp"

#include <map>
#include <string>

#include <boost/shared_ptr.hpp>

// -----------------------------------------------------------------------

class System;
class MockSurface;

// -----------------------------------------------------------------------

class TestGraphicsSystem : public GraphicsSystem {
 public:
  TestGraphicsSystem(System& system, Gameexe& gexe);

  virtual void executeGraphicsSystem(RLMachine& machine) {
    GraphicsSystem::executeGraphicsSystem(machine);
  }

  int screenWidth() const { return 640; }
  int screenHeight() const { return 480; }

  void injectSurface(const std::string& short_filename,
                     const boost::shared_ptr<Surface>& surface);

  virtual Size screenSize() const { return Size(640, 480); }
  virtual void allocateDC(int dc, Size s);
  virtual void setMinimumSizeForDC(int, Size) { /* noop for now. */ }
  virtual void freeDC(int dc);

  virtual void clearAndPromoteObjects();

  virtual GraphicsObject& getObject(int layer, int obj_number);

  // Make a null Surface object?
  virtual boost::shared_ptr<Surface> loadNonCGSurfaceFromFile(
      const std::string& short_filename);
  virtual boost::shared_ptr<Surface> getHaikei();
  virtual boost::shared_ptr<Surface> getDC(int dc);
  virtual boost::shared_ptr<Surface> buildSurface(const Size& s);

  virtual void blitSurfaceToDC(
      Surface& source_obj, int target_dc,
      int srcX, int srcY, int src_width, int src_height,
      int destX, int destY, int dest_width, int dest_height,
      int alpha = 255);

  // Needed because of covariant issues.
  MockSurface& getMockDC(int dc);

 private:
  boost::shared_ptr<MockSurface> haikei_;

  /// Map between device contexts number and their surface.
  boost::shared_ptr<MockSurface> display_contexts_[16];

  // A list of user injected surfaces to hand back for named files.
  std::map<std::string, boost::shared_ptr<Surface> > named_surfaces_;
};

#endif  // TEST_TESTSYSTEM_TESTGRAPHICSSYSTEM_HPP_
