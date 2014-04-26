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
#include <memory>
#include <string>

class System;
class MockSurface;
class MockColourFilter;

class TestGraphicsSystem : public GraphicsSystem {
 public:
  TestGraphicsSystem(System& system, Gameexe& gexe);
  virtual ~TestGraphicsSystem();

  void InjectSurface(const std::string& short_filename,
                     const std::shared_ptr<Surface>& surface);

  virtual void AllocateDC(int dc, Size s) override;
  virtual void SetMinimumSizeForDC(int, Size) override;
  virtual void FreeDC(int dc) override;

  // Make a null Surface object?
  virtual std::shared_ptr<const Surface> LoadSurfaceFromFile(
      const std::string& short_filename) override;
  virtual std::shared_ptr<Surface> GetHaikei() override;
  virtual std::shared_ptr<Surface> GetDC(int dc) override;
  virtual std::shared_ptr<Surface> BuildSurface(const Size& s) override;
  virtual ColourFilter* BuildColourFiller() override;

  virtual void BeginFrame() override;
  virtual void EndFrame() override;
  virtual std::shared_ptr<Surface> EndFrameToSurface() override;

  // Needed because of covariant issues.
  MockSurface& GetMockDC(int dc);

 private:
  std::shared_ptr<MockSurface> haikei_;

  // Map between device contexts number and their surface.
  std::shared_ptr<MockSurface> display_contexts_[16];

  // A list of user injected surfaces to hand back for named files.
  std::map<std::string, std::shared_ptr<const Surface>> named_surfaces_;
};

#endif  // TEST_TEST_SYSTEM_TEST_GRAPHICS_SYSTEM_H_
