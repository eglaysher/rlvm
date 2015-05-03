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

#include "effects/blind_effect.h"

#include <cstdlib>

#include "systems/base/surface.h"

// -----------------------------------------------------------------------
// BlindEffect
// -----------------------------------------------------------------------

BlindEffect::BlindEffect(RLMachine& machine,
                         std::shared_ptr<Surface> src,
                         std::shared_ptr<Surface> dst,
                         const Size& screen_size,
                         int time,
                         int blindSize)
    : Effect(machine, src, dst, screen_size, time), blind_size_(blindSize) {}

BlindEffect::~BlindEffect() {}

void BlindEffect::ComputeGrowing(RLMachine& machine,
                                 int maxSize,
                                 int currentTime) {
  int num_blinds = maxSize / blind_size() + 1;
  int rows_to_display =
      int((float(currentTime) / duration()) * (blind_size() + num_blinds));

  for (int currentBlind = 0; currentBlind < num_blinds; ++currentBlind) {
    if (currentBlind <= rows_to_display) {
      int currentlyDisplayed = std::abs(currentBlind - rows_to_display);
      if (currentlyDisplayed > blind_size())
        currentlyDisplayed = blind_size();

      int polygonStart = currentBlind * blind_size();
      RenderPolygon(polygonStart, polygonStart + currentlyDisplayed);
    }
  }
}

void BlindEffect::ComputeDecreasing(RLMachine& machine,
                                    int maxSize,
                                    int currentTime) {
  int num_blinds = maxSize / blind_size() + 1;
  int rows_to_display =
      int((float(currentTime) / duration()) * (blind_size() + num_blinds));

  for (int currentBlind = num_blinds; currentBlind >= 0; --currentBlind) {
    if ((num_blinds - currentBlind) < rows_to_display) {
      int currentlyDisplayed =
          std::abs(num_blinds - currentBlind - rows_to_display);
      if (currentlyDisplayed > blind_size())
        currentlyDisplayed = blind_size();

      int bottomOfPolygon = currentBlind * blind_size();

      RenderPolygon(bottomOfPolygon, bottomOfPolygon - currentlyDisplayed);
    }
  }
}

bool BlindEffect::BlitOriginalImage() const { return true; }

// -----------------------------------------------------------------------
// BlindTopToBottomEffect
// -----------------------------------------------------------------------

BlindTopToBottomEffect::BlindTopToBottomEffect(RLMachine& machine,
                                               std::shared_ptr<Surface> src,
                                               std::shared_ptr<Surface> dst,
                                               const Size& screen_size,
                                               int time,
                                               int blindSize)
    : BlindEffect(machine, src, dst, screen_size, time, blindSize) {}

BlindTopToBottomEffect::~BlindTopToBottomEffect() {}

void BlindTopToBottomEffect::PerformEffectForTime(RLMachine& machine,
                                                  int currentTime) {
  ComputeGrowing(machine, height(), currentTime);
}

void BlindTopToBottomEffect::RenderPolygon(int polyStart, int polyEnd) {
  src_surface().RenderToScreen(Rect::GRP(0, polyStart, width(), polyEnd),
                               Rect::GRP(0, polyStart, width(), polyEnd),
                               255);
}

// -----------------------------------------------------------------------
// BlindBottomToTopEffect
// -----------------------------------------------------------------------

BlindBottomToTopEffect::BlindBottomToTopEffect(RLMachine& machine,
                                               std::shared_ptr<Surface> src,
                                               std::shared_ptr<Surface> dst,
                                               const Size& screen_size,
                                               int time,
                                               int blindSize)
    : BlindEffect(machine, src, dst, screen_size, time, blindSize) {}

BlindBottomToTopEffect::~BlindBottomToTopEffect() {}

void BlindBottomToTopEffect::PerformEffectForTime(RLMachine& machine,
                                                  int currentTime) {
  ComputeDecreasing(machine, height(), currentTime);
}

void BlindBottomToTopEffect::RenderPolygon(int polyStart, int polyEnd) {
  // Render polygon
  src_surface().RenderToScreen(Rect::GRP(0, polyEnd, width(), polyStart),
                               Rect::GRP(0, polyEnd, width(), polyStart),
                               255);
}

// -----------------------------------------------------------------------
// BlindLeftToRightEffect
// -----------------------------------------------------------------------

BlindLeftToRightEffect::BlindLeftToRightEffect(RLMachine& machine,
                                               std::shared_ptr<Surface> src,
                                               std::shared_ptr<Surface> dst,
                                               const Size& screen_size,
                                               int time,
                                               int blindSize)
    : BlindEffect(machine, src, dst, screen_size, time, blindSize) {}

BlindLeftToRightEffect::~BlindLeftToRightEffect() {}

void BlindLeftToRightEffect::PerformEffectForTime(RLMachine& machine,
                                                  int currentTime) {
  ComputeGrowing(machine, width(), currentTime);
}

void BlindLeftToRightEffect::RenderPolygon(int polyStart, int polyEnd) {
  src_surface().RenderToScreen(Rect::GRP(polyStart, 0, polyEnd, height()),
                               Rect::GRP(polyStart, 0, polyEnd, height()),
                               255);
}

// -----------------------------------------------------------------------
// BlindRightToLeftEffect
// -----------------------------------------------------------------------

BlindRightToLeftEffect::BlindRightToLeftEffect(RLMachine& machine,
                                               std::shared_ptr<Surface> src,
                                               std::shared_ptr<Surface> dst,
                                               const Size& screen_size,
                                               int time,
                                               int blindSize)
    : BlindEffect(machine, src, dst, screen_size, time, blindSize) {}

BlindRightToLeftEffect::~BlindRightToLeftEffect() {}

void BlindRightToLeftEffect::PerformEffectForTime(RLMachine& machine,
                                                  int currentTime) {
  ComputeDecreasing(machine, width(), currentTime);
}

void BlindRightToLeftEffect::RenderPolygon(int polyStart, int polyEnd) {
  src_surface().RenderToScreen(Rect::GRP(polyEnd, 0, polyStart, height()),
                               Rect::GRP(polyEnd, 0, polyStart, height()),
                               255);
}
