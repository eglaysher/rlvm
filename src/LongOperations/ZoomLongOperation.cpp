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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// -----------------------------------------------------------------------

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

#include "LongOperations/ZoomLongOperation.hpp"

#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/EventSystem.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/Surface.hpp"

#include <iostream>

using std::cerr;
using std::endl;

// -----------------------------------------------------------------------
// ZoomLongOperation
// -----------------------------------------------------------------------

ZoomLongOperation::ZoomLongOperation(
  RLMachine& machine,
  const boost::shared_ptr<Surface>& origSurface,
  const boost::shared_ptr<Surface>& srcSurface,
  const Rect& frect, const Rect& trect, const Rect& drect,
  const int time)
  : machine_(machine), orig_surface_(origSurface),
    src_surface_(srcSurface),
    frect_(frect), trect_(trect), drect_(drect), duration_(time),
    start_time_(machine.system().event().getTicks()) {
}

// -----------------------------------------------------------------------

ZoomLongOperation::~ZoomLongOperation() {
}

// -----------------------------------------------------------------------

bool ZoomLongOperation::operator()(RLMachine& machine) {
  unsigned int time = machine.system().event().getTicks();
  unsigned int currentFrame = time - start_time_;

  bool fastForward = machine.system().fastForward();

  if (currentFrame >= duration_ || fastForward) {
    return true;
  } else {
    // Render to the screen
    GraphicsSystem& graphics = machine.system().graphics();
    graphics.beginFrame();

    // First blit the original dc0 to the screen
    orig_surface_->
      renderToScreen(orig_surface_->rect(), orig_surface_->rect(), 255);

    // figure out the new coordinates for the zoom.
    float ratio = currentFrame / float(duration_);
    Point zPt = frect_.origin() + ((trect_.origin() - frect_.origin()) * ratio);
    Size zSize = frect_.size() + ((trect_.size() - frect_.size()) * ratio);

    src_surface_->
      renderToScreen(Rect(zPt, zSize), drect_, 255);

    graphics.endFrame();
    return false;
  }
}



