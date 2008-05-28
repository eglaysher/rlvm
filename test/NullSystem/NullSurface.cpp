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

// -----------------------------------------------------------------------
// NullSurface
// -----------------------------------------------------------------------
NullSurface::NullSurface(const std::string& surface_name) 
  : surface_name_(surface_name), allocated_(false), width_(-1), height_(-1), 
    surface_log_(surface_name)
{
}

// -----------------------------------------------------------------------

NullSurface::NullSurface(const std::string& surface_name, int width, int height) 
  : surface_name_(surface_name), allocated_(true), width_(width), height_(height), 
    surface_log_(surface_name)
{
}

// -----------------------------------------------------------------------

void NullSurface::allocate(int width, int height) {
  surface_log_.recordFunction("allocate", width, height);
  allocated_ = true;
  width_ = width;
  height_ = height;
}
 
// -----------------------------------------------------------------------

void NullSurface::deallocate() {
  surface_log_.recordFunction("deallocate");
  allocated_ = false;
}

// -----------------------------------------------------------------------

int NullSurface::width() const { return width_; }
int NullSurface::height() const { return height_; }

// -----------------------------------------------------------------------

void NullSurface::blitToSurface(
  Surface& surface, 
  int srcX, int srcY, int srcWidth, int srcHeight,
  int destX, int destY, int destWidth, int destHeight,
  int alpha, bool useSrcAlpha)
{
  surface_log_.recordFunction(
    "blitToSurface", srcX, srcY, srcWidth, srcHeight,
    destX, destY, destWidth, destHeight, alpha, useSrcAlpha);
}

// -----------------------------------------------------------------------

void NullSurface::renderToScreen(
  int srcX, int srcY, int srcWidth, int srcHeight,
  int destX, int destY, int destWidth, int destHeight,
  int alpha)
{
  surface_log_.recordFunction(
    "renderToScreen", srcX, srcY, srcWidth, srcHeight,
    destX, destY, destWidth, destHeight, alpha);
}

// -----------------------------------------------------------------------

void NullSurface::renderToScreenAsColorMask(
  int srcX1, int srcY1, int srcX2, int srcY2,
  int destX1, int destY1, int destX2, int destY2,
  int r, int g, int b, int alpha, int filter)
{
  surface_log_.recordFunction(
    "renderToScreenAsColorMask", srcX1, srcY1, srcX2, srcY2,
    destX1, destY1, destX2, destY2, r, g, b, alpha, filter);
}

// -----------------------------------------------------------------------

void NullSurface::renderToScreen(
    int srcX1, int srcY1, int srcX2, int srcY2,
    int destX1, int destY1, int destX2, int destY2,
    const int opacity[4])
{
  surface_log_.recordFunction(
    "renderToScreen",  srcX1, srcY1, srcX2, srcY2,
    destX1, destY1, destX2, destY2);
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
  return m_regionTable.size();
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

void NullSurface::fill(int r, int g, int b, int alpha) {
  surface_log_.recordFunction("fill", r, g, b, alpha);
}

// -----------------------------------------------------------------------

void NullSurface::fill(int r, int g, int b, int alpha, int x, int y, 
                       int width, int height) {
  surface_log_.recordFunction("fill", r, g, b, alpha, x, y, width, height);
}

// -----------------------------------------------------------------------

void NullSurface::getDCPixel(int x, int y, int& r, int& g, int& b) {
  surface_log_.recordFunction("getDCPixel", x, y);
  // This really doesn't work...
}

// -----------------------------------------------------------------------

boost::shared_ptr<Surface> NullSurface::clipAsColorMask(
    int x, int y, int width, int height, 
    int r, int g, int b) {
  surface_log_.recordFunction("clipAsColorMask", x, y, width, height, r, g, b);

  return boost::shared_ptr<Surface>(
    new NullSurface("Clip of " + surface_name_, width, height));
}

// -----------------------------------------------------------------------

Surface* NullSurface::clone() const {
  surface_log_.recordFunction("clone");

  return new NullSurface("Copy of " + surface_name_, width_, height_);
}


