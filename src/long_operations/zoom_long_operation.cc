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

#include "long_operations/zoom_long_operation.h"

#include "machine/rlmachine.h"
#include "systems/base/event_system.h"
#include "systems/base/graphics_system.h"
#include "systems/base/surface.h"
#include "systems/base/system.h"

// -----------------------------------------------------------------------
// ZoomLongOperation
// -----------------------------------------------------------------------

ZoomLongOperation::ZoomLongOperation(
    RLMachine& machine,
    const std::shared_ptr<Surface>& origSurface,
    const std::shared_ptr<Surface>& srcSurface,
    const Rect& frect,
    const Rect& trect,
    const Rect& drect,
    const int time)
    : orig_surface_(origSurface),
      src_surface_(srcSurface),
      frect_(frect),
      trect_(trect),
      drect_(drect),
      duration_(time),
      start_time_(machine.system().event().GetTicks()) {}

ZoomLongOperation::~ZoomLongOperation() {}

bool ZoomLongOperation::operator()(RLMachine& machine) {
  unsigned int time = machine.system().event().GetTicks();
  unsigned int currentFrame = time - start_time_;

  bool fastForward = machine.system().ShouldFastForward();

  if (currentFrame >= duration_ || fastForward) {
    return true;
  } else {
    // Render to the screen
    GraphicsSystem& graphics = machine.system().graphics();
    graphics.BeginFrame();

    // First blit the original dc0 to the screen
    orig_surface_->RenderToScreen(
        orig_surface_->GetRect(), orig_surface_->GetRect(), 255);

    // figure out the new coordinates for the zoom.
    float ratio = currentFrame / float(duration_);
    Point zPt = frect_.origin() + ((trect_.origin() - frect_.origin()) * ratio);
    Size zSize = frect_.size() + ((trect_.size() - frect_.size()) * ratio);

    src_surface_->RenderToScreen(Rect(zPt, zSize), drect_, 255);

    graphics.EndFrame();
    return false;
  }
}
