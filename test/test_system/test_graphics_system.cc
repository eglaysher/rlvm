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

#include "test_system/test_graphics_system.h"

#include <map>
#include <string>
#include <sstream>

#include "systems/base/colour.h"
#include "systems/base/graphics_object.h"
#include "systems/base/graphics_system.h"
#include "test_system/mock_colour_filter.h"
#include "test_system/mock_surface.h"
#include "utilities/exception.h"

using namespace std;

TestGraphicsSystem::TestGraphicsSystem(System& system, Gameexe& gexe)
    : GraphicsSystem(system, gexe) {
  SetScreenSize(Size(640, 480));

  for (int i = 0; i < 16; ++i) {
    ostringstream oss;
    oss << "DC #" << i;
    display_contexts_[i].reset(MockSurface::Create(oss.str()));
  }

  display_contexts_[0]->Allocate(screen_size());
  display_contexts_[1]->Allocate(screen_size());

  haikei_.reset(MockSurface::Create("Haikei"));
  haikei_->Allocate(screen_size());
}

TestGraphicsSystem::~TestGraphicsSystem() {}

void TestGraphicsSystem::AllocateDC(int dc, Size size) {
  if (dc >= 16)
    throw rlvm::Exception(
        "Invalid DC number in "
        "TestGraphicsSystem::allocate_dc");

  // We can't reallocate the screen!
  if (dc == 0)
    throw rlvm::Exception("Attempting to reallocate DC 0!");

  // DC 1 is a special case and must always be at least the size of
  // the screen.
  if (dc == 1) {
    std::shared_ptr<MockSurface> dc0 = display_contexts_[0];
    Size dc0_size = dc0->GetSize();
    if (size.width() < dc0_size.width())
      size.set_width(dc0_size.width());
    if (size.height() < dc0_size.height())
      size.set_height(dc0_size.height());
  }

  // Allocate a new obj.
  display_contexts_[dc]->Allocate(size);
}

void TestGraphicsSystem::SetMinimumSizeForDC(int, Size) {
  // noop for now.
}

void TestGraphicsSystem::FreeDC(int dc) {
  if (dc == 0) {
    throw rlvm::Exception("Attempt to deallocate DC[0]");
  } else if (dc == 1) {
    // DC[1] never gets freed; it only gets blanked
    display_contexts_[1]->Fill(RGBAColour::Black());
  } else {
    display_contexts_[dc]->Deallocate();
  }
}

void TestGraphicsSystem::InjectSurface(
    const std::string& short_filename,
    const std::shared_ptr<Surface>& surface) {
  named_surfaces_[short_filename] = surface;
}

std::shared_ptr<const Surface> TestGraphicsSystem::LoadSurfaceFromFile(
    const std::string& short_filename) {
  // If we have an injected surface, return it instead of a fresh surface.
  std::map<std::string, std::shared_ptr<const Surface>>::iterator it =
      named_surfaces_.find(short_filename);
  if (it != named_surfaces_.end()) {
    return it->second;
  }

  // We don't have an injected surface so make a surface.
  return std::shared_ptr<const Surface>(
      std::const_pointer_cast<const MockSurface>(
          std::shared_ptr<MockSurface>(
              MockSurface::Create(short_filename, Size(50, 50)))));
}

std::shared_ptr<Surface> TestGraphicsSystem::GetHaikei() { return haikei_; }

std::shared_ptr<Surface> TestGraphicsSystem::GetDC(int dc) {
  return display_contexts_[dc];
}

std::shared_ptr<Surface> TestGraphicsSystem::BuildSurface(const Size& s) {
  static int surface_num = 0;
  ostringstream oss;
  oss << "Built Surface #" << surface_num++;

  return std::shared_ptr<Surface>(MockSurface::Create(oss.str(), s));
}

ColourFilter* TestGraphicsSystem::BuildColourFiller() {
  return new MockColourFilter;
}

void TestGraphicsSystem::BeginFrame() {}

void TestGraphicsSystem::EndFrame() {}

std::shared_ptr<Surface> TestGraphicsSystem::EndFrameToSurface() {
  return std::shared_ptr<Surface>();
}

MockSurface& TestGraphicsSystem::GetMockDC(int dc) {
  return *display_contexts_[dc];
}
