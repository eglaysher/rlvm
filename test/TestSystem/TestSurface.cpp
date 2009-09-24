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

#include "TestSystem/TestSurface.hpp"
#include "Systems/Base/Colour.hpp"

#include <string>

// -----------------------------------------------------------------------
// TestSurface
// -----------------------------------------------------------------------
TestSurface::TestSurface(const std::string& surface_name)
    : surface_name_(surface_name), allocated_(false), size_(-1, -1) {
}

// -----------------------------------------------------------------------

TestSurface::TestSurface(const std::string& surface_name, const Size& size)
    : surface_name_(surface_name), allocated_(true), size_(size) {
}

// -----------------------------------------------------------------------

void TestSurface::allocate(const Size& size) {
  allocated_ = true;
  size_ = size;
}

// -----------------------------------------------------------------------

void TestSurface::deallocate() {
  allocated_ = false;
}

// -----------------------------------------------------------------------

Size TestSurface::size() const { return size_; }

// -----------------------------------------------------------------------

void TestSurface::blitToSurface(
  Surface& surface,
  const Rect& src, const Rect& dst,
  int alpha, bool use_src_alpha) {
}

// -----------------------------------------------------------------------

void TestSurface::renderToScreen(
  const Rect& src, const Rect& dst, int alpha) {
}

// -----------------------------------------------------------------------

void TestSurface::renderToScreenAsColorMask(
  const Rect& src, const Rect& dst, const RGBAColour& rgba, int filter) {
}

// -----------------------------------------------------------------------

void TestSurface::renderToScreen(
  const Rect& src, const Rect& dst, const int opacity[4]) {
}

// -----------------------------------------------------------------------

void TestSurface::renderToScreenAsObject(const GraphicsObject& rp,
                                         const Rect& src,
                                         const Rect& dst,
                                         int alpha) {
}

// -----------------------------------------------------------------------

int TestSurface::numPatterns() const {
  return region_table_.size();
}

// -----------------------------------------------------------------------

const Surface::GrpRect& TestSurface::getPattern(int patt_no) const {
  static Surface::GrpRect r;
  return r;
}

// -----------------------------------------------------------------------

void TestSurface::rawRenderQuad(const int src_coords[8],
                                const int dest_coords[8],
                                const int opacity[4]) {
}

// -----------------------------------------------------------------------

void TestSurface::fill(const RGBAColour& colour) {
}

// -----------------------------------------------------------------------

void TestSurface::fill(const RGBAColour& colour, const Rect& rect) {
}

// -----------------------------------------------------------------------

void TestSurface::getDCPixel(const Point& p, int& r, int& g, int& b) {
  // This really doesn't work...
}

// -----------------------------------------------------------------------

boost::shared_ptr<Surface> TestSurface::clipAsColorMask(
  const Rect& rect, int r, int g, int b) {
  return boost::shared_ptr<Surface>(
    new TestSurface("Clip of " + surface_name_, rect.size()));
}

// -----------------------------------------------------------------------

Surface* TestSurface::clone() const {
  return new TestSurface("Copy of " + surface_name_, size_);
}


