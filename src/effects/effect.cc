// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006 Elliot Glaysher
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

#include "effects/effect.h"

#include "machine/rlmachine.h"
#include "systems/base/event_system.h"
#include "systems/base/graphics_system.h"
#include "systems/base/surface.h"
#include "systems/base/system.h"

// -----------------------------------------------------------------------
// Effect
// -----------------------------------------------------------------------

Effect::Effect(RLMachine& machine,
               boost::shared_ptr<Surface> src,
               boost::shared_ptr<Surface> dst,
               Size size,
               int time)
    : screen_size_(size),
      duration_(time),
      start_time_(machine.system().event().getTicks()),
      machine_(machine),
      src_surface_(src),
      dst_surface_(dst) {
  machine.system().graphics().setIsResponsibleForUpdate(false);
}

Effect::~Effect() {
  machine_.system().graphics().setIsResponsibleForUpdate(true);
}

bool Effect::operator()(RLMachine& machine) {
  unsigned int time = machine.system().event().getTicks();
  unsigned int currentFrame = time - start_time_;

  bool fastForward = machine.system().fastForward();

  if (currentFrame >= duration_ || fastForward) {
    return true;
  } else {
    GraphicsSystem& graphics = machine.system().graphics();
    graphics.beginFrame();

    if (blitOriginalImage()) {
      dstSurface().renderToScreen(
          Rect(Point(0, 0), size()), Rect(Point(0, 0), size()), 255);
    }

    performEffectForTime(machine, currentFrame);

    graphics.endFrame();
    return false;
  }
}

// -----------------------------------------------------------------------
// BlitAfterEffectFinishes
// -----------------------------------------------------------------------

BlitAfterEffectFinishes::BlitAfterEffectFinishes(LongOperation* in,
                                                 boost::shared_ptr<Surface> src,
                                                 boost::shared_ptr<Surface> dst,
                                                 const Rect& srcRect,
                                                 const Rect& destRect)
    : PerformAfterLongOperationDecorator(in),
      src_surface_(src),
      dst_surface_(dst),
      src_rect_(srcRect),
      dest_rect_(destRect) {}

BlitAfterEffectFinishes::~BlitAfterEffectFinishes() {}

void BlitAfterEffectFinishes::performAfterLongOperation(RLMachine& machine) {
  // Blit DC1 onto DC0, with full opacity, and end the operation
  src_surface_->blitToSurface(*dst_surface_, src_rect_, dest_rect_, 255);

  // Now force a screen refresh
  machine.system().graphics().forceRefresh();
}
