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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
//
// -----------------------------------------------------------------------

#include "systems/base/mouse_cursor.h"

#include "systems/base/event_system.h"
#include "systems/base/graphics_system.h"
#include "systems/base/surface.h"
#include "systems/base/system.h"

const int CURSOR_SIZE_INT = 32;
const Size CURSOR_SIZE = Size(CURSOR_SIZE_INT, CURSOR_SIZE_INT);

const int HOTSPOTMASK_X_OFFSET = 8;
const int HOTSPOTMASK_Y_OFFSET = 48;

// -----------------------------------------------------------------------
// MouseCursor (public)
// -----------------------------------------------------------------------
MouseCursor::MouseCursor(System& system,
                         const std::shared_ptr<const Surface>& cursor_surface,
                         int count,
                         int speed)
    : cursor_surface_(cursor_surface),
      count_(count),
      frame_speed_(speed / count_),
      current_frame_(0),
      last_time_frame_incremented_(system.event().GetTicks()) {
  // TODO(erg): Technically, each frame might have a hotspot. In practice, the
  // hotspot is in the same place every frame.
  FindHotspot();

  int alphaR, alphaG, alphaB;
  cursor_surface->GetDCPixel(Point(0, 0), alphaR, alphaG, alphaB);

  cursor_surface_ = cursor_surface->ClipAsColorMask(
      Rect(8, 8, Size(CURSOR_SIZE_INT * count_, CURSOR_SIZE_INT)),
      alphaR,
      alphaG,
      alphaB);
}

MouseCursor::~MouseCursor() {}

void MouseCursor::Execute(System& system) {
  unsigned int cur_time = system.event().GetTicks();

  if (last_time_frame_incremented_ + frame_speed_ < cur_time) {
    last_time_frame_incremented_ = cur_time;

    system.graphics().MarkScreenAsDirty(GUT_MOUSE_MOTION);

    current_frame_++;
    if (current_frame_ >= count_)
      current_frame_ = 0;
  }
}

void MouseCursor::RenderHotspotAt(const Point& mouse_location) {
  Point render_point = GetTopLeftForHotspotAt(mouse_location);
  cursor_surface_->RenderToScreen(
      Rect(current_frame_ * CURSOR_SIZE_INT, 0, CURSOR_SIZE),
      Rect(render_point, CURSOR_SIZE));
}

// -----------------------------------------------------------------------
// MouseCursor (private)
// -----------------------------------------------------------------------

Point MouseCursor::GetTopLeftForHotspotAt(const Point& mouse_location) {
  return mouse_location - hotspot_offset_;
}

void MouseCursor::FindHotspot() {
  int r, g, b;

  for (int x = HOTSPOTMASK_X_OFFSET; x < HOTSPOTMASK_X_OFFSET + CURSOR_SIZE_INT;
       ++x) {
    for (int y = HOTSPOTMASK_Y_OFFSET;
         y < HOTSPOTMASK_Y_OFFSET + CURSOR_SIZE_INT;
         ++y) {
      cursor_surface_->GetDCPixel(Point(x, y), r, g, b);

      if (r == 255 && g == 255 && b == 255) {
        hotspot_offset_ =
            Size(x - HOTSPOTMASK_X_OFFSET, y - HOTSPOTMASK_Y_OFFSET);
        return;
      }
    }
  }
}
