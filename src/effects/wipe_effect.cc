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

#include "effects/wipe_effect.h"

#include <cmath>

#include "machine/rlmachine.h"
#include "systems/base/graphics_system.h"
#include "systems/base/surface.h"
#include "systems/base/system.h"

// -----------------------------------------------------------------------
// WipeEffect base class
// -----------------------------------------------------------------------

WipeEffect::WipeEffect(RLMachine& machine,
                       std::shared_ptr<Surface> src,
                       std::shared_ptr<Surface> dst,
                       const Size& screen_size,
                       int time,
                       int interpolation)
    : Effect(machine, src, dst, screen_size, time),
      interpolation_(interpolation),
      interpolation_in_pixels_(0) {
  if (interpolation_)
    interpolation_in_pixels_ = int(pow(float(2), interpolation) * 2.5);
}

WipeEffect::~WipeEffect() {}

// Calculates the size of the interpolation and main polygons.
//
// There are 3 possible stages:
// - [0, interpolation_in_pixels_) - Draw only the
//   transition. (sizeOfMainPolygon == 0, sizeOfInterpolation ==
//   amountVisible)
// - [interpolation_in_pixels_, sizeOfScreen) - Draw both
//   polygons. (sizeOfMainPolygon == amountVisible -
//   interpolation_in_pixels_, sizeOfInterpolation == amountVisible)
// - [height, height + interpolation_in_pixels_) - Draw both
//   polygons, flooring the height of the transition to
void WipeEffect::CalculateSizes(int currentTime,
                                int& sizeOfInterpolation,
                                int& sizeOfMainPolygon,
                                int sizeOfScreen) {
  int amountVisible = int((float(currentTime) / duration()) *
                          (sizeOfScreen + interpolation_in_pixels_));
  if (amountVisible < interpolation_in_pixels_) {
    sizeOfInterpolation = amountVisible;
    sizeOfMainPolygon = 0;
  } else if (amountVisible < sizeOfScreen) {
    sizeOfInterpolation = interpolation_in_pixels_;
    sizeOfMainPolygon = amountVisible - interpolation_in_pixels_;
  } else if (amountVisible < sizeOfScreen + interpolation_in_pixels_) {
    sizeOfMainPolygon = amountVisible - interpolation_in_pixels_;
    sizeOfInterpolation = sizeOfScreen - sizeOfMainPolygon;
  }
}

bool WipeEffect::BlitOriginalImage() const { return true; }

// -----------------------------------------------------------------------
// WipeTopToBottomEffect
// -----------------------------------------------------------------------

WipeTopToBottomEffect::WipeTopToBottomEffect(RLMachine& machine,
                                             std::shared_ptr<Surface> src,
                                             std::shared_ptr<Surface> dst,
                                             const Size& screen_size,
                                             int time,
                                             int interpolation)
    : WipeEffect(machine, src, dst, screen_size, time, interpolation) {}

WipeTopToBottomEffect::~WipeTopToBottomEffect() {}

void WipeTopToBottomEffect::PerformEffectForTime(RLMachine& machine,
                                                 int currentTime) {
  int sizeOfInterpolation, sizeOfMainPolygon;
  CalculateSizes(currentTime, sizeOfInterpolation, sizeOfMainPolygon, height());

  if (sizeOfMainPolygon) {
    src_surface().RenderToScreen(Rect::REC(0, 0, width(), sizeOfMainPolygon),
                                 Rect::REC(0, 0, width(), sizeOfMainPolygon),
                                 255);
  }

  if (sizeOfInterpolation) {
    int opacity[4] = {255, 255, 0, 0};

    src_surface().RenderToScreen(
        Rect::GRP(0,
                  sizeOfMainPolygon,
                  width(),
                  sizeOfMainPolygon + sizeOfInterpolation),
        Rect::GRP(0,
                  sizeOfMainPolygon,
                  width(),
                  sizeOfMainPolygon + sizeOfInterpolation),
        opacity);
  }
}

// -----------------------------------------------------------------------
// WipeBottomToTopEffect
// -----------------------------------------------------------------------

WipeBottomToTopEffect::WipeBottomToTopEffect(RLMachine& machine,
                                             std::shared_ptr<Surface> src,
                                             std::shared_ptr<Surface> dst,
                                             const Size& screen_size,
                                             int time,
                                             int interpolation)
    : WipeEffect(machine, src, dst, screen_size, time, interpolation) {}

WipeBottomToTopEffect::~WipeBottomToTopEffect() {}

void WipeBottomToTopEffect::PerformEffectForTime(RLMachine& machine,
                                                 int currentTime) {
  int sizeOfInterpolation, sizeOfMainPolygon;
  CalculateSizes(currentTime, sizeOfInterpolation, sizeOfMainPolygon, height());

  // Render the sliding on frame
  if (sizeOfMainPolygon) {
    src_surface().RenderToScreen(
        Rect::GRP(0, height() - sizeOfMainPolygon, width(), height()),
        Rect::GRP(0, height() - sizeOfMainPolygon, width(), height()),
        255);
  }

  if (sizeOfInterpolation) {
    int opacity[4] = {0, 0, 255, 255};
    src_surface().RenderToScreen(
        Rect::GRP(0,
                  height() - sizeOfMainPolygon - sizeOfInterpolation,
                  width(),
                  height() - sizeOfMainPolygon),
        Rect::GRP(0,
                  height() - sizeOfMainPolygon - sizeOfInterpolation,
                  width(),
                  height() - sizeOfMainPolygon),
        opacity);
  }
}

// -----------------------------------------------------------------------
// WipeFromLeftToRightEffect
// -----------------------------------------------------------------------

WipeLeftToRightEffect::WipeLeftToRightEffect(RLMachine& machine,
                                             std::shared_ptr<Surface> src,
                                             std::shared_ptr<Surface> dst,
                                             const Size& screen_size,
                                             int time,
                                             int interpolation)
    : WipeEffect(machine, src, dst, screen_size, time, interpolation) {}

WipeLeftToRightEffect::~WipeLeftToRightEffect() {}

void WipeLeftToRightEffect::PerformEffectForTime(RLMachine& machine,
                                                 int currentTime) {
  int sizeOfInterpolation, sizeOfMainPolygon;
  CalculateSizes(currentTime, sizeOfInterpolation, sizeOfMainPolygon, width());

  // CONTINUE FIXING THE WIPES HERE!

  if (sizeOfMainPolygon) {
    src_surface().RenderToScreen(Rect::GRP(0, 0, sizeOfMainPolygon, height()),
                                 Rect::GRP(0, 0, sizeOfMainPolygon, height()),
                                 255);
  }

  if (sizeOfInterpolation) {
    int opacity[4] = {255, 0, 0, 255};
    src_surface().RenderToScreen(
        Rect::GRP(sizeOfMainPolygon,
                  0,
                  sizeOfMainPolygon + sizeOfInterpolation,
                  height()),
        Rect::GRP(sizeOfMainPolygon,
                  0,
                  sizeOfMainPolygon + sizeOfInterpolation,
                  height()),
        opacity);
  }
}

// -----------------------------------------------------------------------
// WipeFromRightToLeftEffect
// -----------------------------------------------------------------------

WipeRightToLeftEffect::WipeRightToLeftEffect(RLMachine& machine,
                                             std::shared_ptr<Surface> src,
                                             std::shared_ptr<Surface> dst,
                                             const Size& screen_size,
                                             int time,
                                             int interpolation)
    : WipeEffect(machine, src, dst, screen_size, time, interpolation) {}

WipeRightToLeftEffect::~WipeRightToLeftEffect() {}

void WipeRightToLeftEffect::PerformEffectForTime(RLMachine& machine,
                                                 int currentTime) {
  int sizeOfInterpolation, sizeOfMainPolygon;
  CalculateSizes(currentTime, sizeOfInterpolation, sizeOfMainPolygon, width());

  if (sizeOfMainPolygon) {
    src_surface().RenderToScreen(
        Rect::GRP(width() - sizeOfMainPolygon, 0, width(), height()),
        Rect::GRP(width() - sizeOfMainPolygon, 0, width(), height()),
        255);
  }

  if (sizeOfInterpolation) {
    int opacity[4] = {0, 255, 255, 0};
    src_surface().RenderToScreen(
        Rect::GRP(width() - sizeOfInterpolation - sizeOfMainPolygon,
                  0,
                  width() - sizeOfMainPolygon,
                  height()),
        Rect::GRP(width() - sizeOfInterpolation - sizeOfMainPolygon,
                  0,
                  width() - sizeOfMainPolygon,
                  height()),
        opacity);
  }
}
