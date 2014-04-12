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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
//
// -----------------------------------------------------------------------

#ifndef TEST_TEST_SYSTEM_TEST_GRAPHICS_SYSTEM_H_
#define TEST_TEST_SYSTEM_TEST_GRAPHICS_SYSTEM_H_

#include "systems/base/graphics_system.h"
#include "systems/base/surface.h"

#include <map>
#include <string>

#include <boost/shared_ptr.hpp>

class System;
class MockSurface;
class MockColourFilter;

class TestGraphicsSystem : public GraphicsSystem {
 public:
  TestGraphicsSystem(System& system, Gameexe& gexe);
  virtual ~TestGraphicsSystem();

  virtual void ExecuteGraphicsSystem(RLMachine& machine) {
    GraphicsSystem::ExecuteGraphicsSystem(machine);
  }

  int screenWidth() const { return 640; }
  int screenHeight() const { return 480; }

  void injectSurface(const std::string& short_filename,
                     const boost::shared_ptr<Surface>& surface);

  virtual Size screen_size() const { return Size(640, 480); }
  virtual void AllocateDC(int dc, Size s);
  virtual void SetMinimumSizeForDC(int, Size) { /* noop for now. */
  }
  virtual void FreeDC(int dc);

  virtual void ClearAndPromoteObjects();

  virtual GraphicsObject& GetObject(int layer, int obj_number);

  // Make a null Surface object?
  virtual boost::shared_ptr<const Surface> LoadSurfaceFromFile(
      const std::string& short_filename) override;
  virtual boost::shared_ptr<Surface> GetHaikei() override;
  virtual boost::shared_ptr<Surface> GetDC(int dc) override;
  virtual boost::shared_ptr<Surface> BuildSurface(const Size& s) override;
  virtual ColourFilter* BuildColourFiller() override;

  virtual void BeginFrame() {}
  virtual void EndFrame() {}
  virtual boost::shared_ptr<Surface> EndFrameToSurface() {
    return boost::shared_ptr<Surface>();
  }

  // Needed because of covariant issues.
  MockSurface& getMockDC(int dc);

 private:
  boost::shared_ptr<MockSurface> haikei_;

  // Map between device contexts number and their surface.
  boost::shared_ptr<MockSurface> display_contexts_[16];

  // A list of user injected surfaces to hand back for named files.
  std::map<std::string, boost::shared_ptr<const Surface>> named_surfaces_;
};

#endif  // TEST_TEST_SYSTEM_TEST_GRAPHICS_SYSTEM_H_
