// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2008 Elliot Glaysher
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

#include "TestSystem/MockSurface.hpp"
#include "Systems/Base/Colour.hpp"

#include <string>

#include "gmock/gmock.h"

// -----------------------------------------------------------------------
// MockSurface
// -----------------------------------------------------------------------
// static
MockSurface* MockSurface::Create(const std::string& surface_name) {
  return new ::testing::NiceMock<MockSurface>(surface_name);
}

// static
MockSurface* MockSurface::Create(const std::string& surface_name,
                                 const Size& size) {
  return new ::testing::NiceMock<MockSurface>(surface_name, size);
}

void MockSurface::allocate(const Size& size) {
  allocated_ = true;
  size_ = size;
}

void MockSurface::deallocate() {
  allocated_ = false;
}

Size MockSurface::size() const { return size_; }

boost::shared_ptr<Surface> MockSurface::clipAsColorMask(
  const Rect& rect, int r, int g, int b) {
  return boost::shared_ptr<Surface>(
      new ::testing::NiceMock<MockSurface>(
          "Clip of " + surface_name_, rect.size()));
}

Surface* MockSurface::clone() const {
  return new ::testing::NiceMock<MockSurface>(
      "Copy of " + surface_name_, size_);
}

MockSurface::MockSurface(const std::string& surface_name)
    : surface_name_(surface_name), allocated_(false), size_(-1, -1) {
}

MockSurface::MockSurface(const std::string& surface_name, const Size& size)
    : surface_name_(surface_name), allocated_(true), size_(size) {
}


