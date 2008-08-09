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

#include "NullSystem/NullSurface.hpp"
#include "Systems/Base/Colour.hpp"

// -----------------------------------------------------------------------
// NullSurface
// -----------------------------------------------------------------------
NullSurface::NullSurface(const std::string& surface_name)
  : surface_name_(surface_name), allocated_(false), size_(-1, -1),
    surface_log_(surface_name)
{
}

// -----------------------------------------------------------------------

NullSurface::NullSurface(const std::string& surface_name, const Size& size)
  : surface_name_(surface_name), allocated_(true), size_(size),
    surface_log_(surface_name)
{
}

// -----------------------------------------------------------------------

void NullSurface::allocate(const Size& size) {
  surface_log_.recordFunction("allocate", size);
  allocated_ = true;
  size_ = size;
}

// -----------------------------------------------------------------------

void NullSurface::deallocate() {
  surface_log_.recordFunction("deallocate");
  allocated_ = false;
}

// -----------------------------------------------------------------------

Size NullSurface::size() const { return size_; }

// -----------------------------------------------------------------------

void NullSurface::blitToSurface(
  Surface& surface,
  const Rect& src, const Rect& dst,
  int alpha, bool useSrcAlpha)
{
  surface_log_.recordFunction(
    "blitToSurface", src, dst, alpha, useSrcAlpha);
}

// -----------------------------------------------------------------------

void NullSurface::renderToScreen(
  const Rect& src, const Rect& dst, int alpha)
{
  surface_log_.recordFunction(
    "renderToScreen", src, dst, alpha);
}

// -----------------------------------------------------------------------

void NullSurface::renderToScreenAsColorMask(
  const Rect& src, const Rect& dst, const RGBAColour& rgba, int filter)
{
  surface_log_.recordFunction(
    "renderToScreenAsColorMask", src, dst, rgba, filter);
}

// -----------------------------------------------------------------------

void NullSurface::renderToScreen(
  const Rect& src, const Rect& dst, const int opacity[4])
{
  surface_log_.recordFunction(
    "renderToScreen", src, dst);
}

// -----------------------------------------------------------------------

void NullSurface::renderToScreenAsObject(const GraphicsObject& rp) {
  surface_log_.recordFunction("renderToScreenAsObject");
}

// -----------------------------------------------------------------------

void NullSurface::renderToScreenAsObject(const GraphicsObject& rp,
                                         const GraphicsObjectOverride& override) {
  surface_log_.recordFunction("renderToScreenAsObject");
}

// -----------------------------------------------------------------------

int NullSurface::numPatterns() const {
  return region_table_.size();
}

// -----------------------------------------------------------------------

const Surface::GrpRect& NullSurface::getPattern(int pattNo) const {
  static Surface::GrpRect r;
  return r;
}

// -----------------------------------------------------------------------

void NullSurface::rawRenderQuad(const int srcCoords[8],
                                const int destCoords[8],
                                const int opacity[4]) {
  surface_log_.recordFunction("rawRenderQuad");
}

// -----------------------------------------------------------------------

void NullSurface::fill(const RGBAColour& colour) {
  surface_log_.recordFunction("fill", colour);
}

// -----------------------------------------------------------------------

void NullSurface::fill(const RGBAColour& colour, const Rect& rect) {
  surface_log_.recordFunction("fill", colour, rect);
}

// -----------------------------------------------------------------------

void NullSurface::getDCPixel(const Point& p, int& r, int& g, int& b) {
  surface_log_.recordFunction("getDCPixel", p);
  // This really doesn't work...
}

// -----------------------------------------------------------------------

boost::shared_ptr<Surface> NullSurface::clipAsColorMask(
  const Rect& rect, int r, int g, int b) {
  surface_log_.recordFunction("clipAsColorMask", rect, r, g, b);

  return boost::shared_ptr<Surface>(
    new NullSurface("Clip of " + surface_name_, rect.size()));
}

// -----------------------------------------------------------------------

Surface* NullSurface::clone() const {
  surface_log_.recordFunction("clone");

  return new NullSurface("Copy of " + surface_name_, size_);
}


