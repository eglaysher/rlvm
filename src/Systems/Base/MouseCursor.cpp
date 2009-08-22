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

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

#include "Systems/Base/MouseCursor.hpp"
#include "Systems/Base/Surface.hpp"
#include <iostream>

// -----------------------------------------------------------------------

const int CURSOR_SIZE_INT = 32;
const Size CURSOR_SIZE = Size(CURSOR_SIZE_INT, CURSOR_SIZE_INT);
const Rect CURSOR_RECT = Rect(8, 8, CURSOR_SIZE);

const int HOTSPOTMASK_X_OFFSET = 8;
const int HOTSPOTMASK_Y_OFFSET = 48;

using namespace std;

// -----------------------------------------------------------------------
// MouseCursor (public)
// -----------------------------------------------------------------------
MouseCursor::MouseCursor(const boost::shared_ptr<Surface>& cursor_surface)
  : cursor_surface_(cursor_surface) {
  findHotspot();

  int alphaR, alphaG, alphaB;
  cursor_surface->getDCPixel(Point(0, 0), alphaR, alphaG, alphaB);

  cursor_surface_ =
    cursor_surface->clipAsColorMask(CURSOR_RECT, alphaR, alphaG, alphaB);
}

// -----------------------------------------------------------------------

MouseCursor::~MouseCursor() {}

// -----------------------------------------------------------------------

void MouseCursor::renderHotspotAt(const Point& mouse_location) {
  Point render_point = getTopLeftForHotspotAt(mouse_location);
  cursor_surface_->renderToScreen(
    Rect(0, 0, CURSOR_SIZE),
    Rect(render_point, CURSOR_SIZE));
}

// -----------------------------------------------------------------------

Point MouseCursor::getTopLeftForHotspotAt(const Point& mouse_location) {
  return mouse_location - hotspot_offset_;
}

// -----------------------------------------------------------------------
// MouseCursor (private)
// -----------------------------------------------------------------------

void MouseCursor::findHotspot() {
  int r, g, b;

  for (int x = HOTSPOTMASK_X_OFFSET; x < HOTSPOTMASK_X_OFFSET + CURSOR_SIZE_INT;
      ++x) {
    for (int y = HOTSPOTMASK_Y_OFFSET; y < HOTSPOTMASK_Y_OFFSET + CURSOR_SIZE_INT;
        ++y) {
      cursor_surface_->getDCPixel(Point(x, y), r, g, b);

      if (r == 255 && g == 255 && b == 255) {
        hotspot_offset_ =
          Size(x - HOTSPOTMASK_X_OFFSET, y - HOTSPOTMASK_Y_OFFSET);
        break;
      }
    }
  }
}
