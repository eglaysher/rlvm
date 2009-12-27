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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// -----------------------------------------------------------------------

/**
 * @file   BlindEffect.cpp
 * @author Elliot Glaysher
 * @date   Tue Feb 27 21:11:04 2007
 *
 * @ingroup TransitionEffects
 * @brief  Implements \#SEL transition style \#10, Blind.
 */

#include "Effects/BlindEffect.hpp"
#include "Systems/Base/Surface.hpp"

#include <boost/bind.hpp>
#include <boost/function.hpp>

using boost::bind;
using boost::function;
using boost::ref;

// -----------------------------------------------------------------------
// BlindEffect base class
// -----------------------------------------------------------------------

bool BlindEffect::blitOriginalImage() const {
  return true;
}

// -----------------------------------------------------------------------


BlindEffect::BlindEffect(RLMachine& machine, boost::shared_ptr<Surface> src,
                         boost::shared_ptr<Surface> dst,
                         const Size& screenSize, int time,
                         int blindSize)
  : Effect(machine, src, dst, screenSize, time),
    blind_size_(blindSize) {
}

// -----------------------------------------------------------------------

BlindEffect::~BlindEffect() {}

// -----------------------------------------------------------------------

void BlindEffect::computeGrowing(RLMachine& machine, int maxSize,
                                 int currentTime) {
  int numBlinds = maxSize / blindSize() + 1;
  int rowsToDisplay = int((float(currentTime) / duration()) *
                          (blindSize() + numBlinds));

  for (int currentBlind = 0; currentBlind < numBlinds; ++currentBlind) {
    if (currentBlind <= rowsToDisplay) {
      int currentlyDisplayed = abs(currentBlind - rowsToDisplay);
      if (currentlyDisplayed > blindSize())
        currentlyDisplayed = blindSize();

      int polygonStart = currentBlind * blindSize();
      renderPolygon(polygonStart, polygonStart + currentlyDisplayed);
    }
  }
}

// -----------------------------------------------------------------------

void BlindEffect::computeDecreasing(RLMachine& machine, int maxSize,
                                    int currentTime) {
  int numBlinds = maxSize / blindSize() + 1;
  int rowsToDisplay = int((float(currentTime) / duration()) *
                          (blindSize() + numBlinds));

  for (int currentBlind = numBlinds; currentBlind >= 0; --currentBlind) {
    if ((numBlinds - currentBlind) < rowsToDisplay) {
      int currentlyDisplayed = abs(numBlinds - currentBlind - rowsToDisplay);
      if (currentlyDisplayed > blindSize())
        currentlyDisplayed = blindSize();

      int bottomOfPolygon = currentBlind * blindSize();

      renderPolygon(bottomOfPolygon, bottomOfPolygon - currentlyDisplayed);
    }
  }
}

// -----------------------------------------------------------------------
// BlindTopToBottomEffect
// -----------------------------------------------------------------------

BlindTopToBottomEffect::BlindTopToBottomEffect(
  RLMachine& machine, boost::shared_ptr<Surface> src,
  boost::shared_ptr<Surface> dst,
  const Size& screenSize, int time, int blindSize)
  : BlindEffect(machine, src, dst, screenSize, time, blindSize) {}

// -----------------------------------------------------------------------

void BlindTopToBottomEffect::performEffectForTime(RLMachine& machine,
                                                  int currentTime) {
  computeGrowing(machine, height(), currentTime);
}

// -----------------------------------------------------------------------

void BlindTopToBottomEffect::renderPolygon(int polyStart, int polyEnd) {
  srcSurface().renderToScreen(
    Rect::GRP(0, polyStart, width(), polyEnd),
    Rect::GRP(0, polyStart, width(), polyEnd), 255);
}

// -----------------------------------------------------------------------
// BlindBottomToTopEffect
// -----------------------------------------------------------------------

BlindBottomToTopEffect::BlindBottomToTopEffect(
  RLMachine& machine, boost::shared_ptr<Surface> src,
  boost::shared_ptr<Surface> dst,
  const Size& screenSize, int time, int blindSize)
  : BlindEffect(machine, src, dst, screenSize, time, blindSize) {}

// -----------------------------------------------------------------------

void BlindBottomToTopEffect::performEffectForTime(RLMachine& machine,
                                                  int currentTime) {
  computeDecreasing(machine, height(), currentTime);
}

// -----------------------------------------------------------------------

void BlindBottomToTopEffect::renderPolygon(int polyStart, int polyEnd) {
  // Render polygon
  srcSurface().
    renderToScreen(
      Rect::GRP(0, polyEnd, width(), polyStart),
      Rect::GRP(0, polyEnd, width(), polyStart), 255);
}

// -----------------------------------------------------------------------
// BlindLeftToRightEffect
// -----------------------------------------------------------------------

BlindLeftToRightEffect::BlindLeftToRightEffect(
  RLMachine& machine, boost::shared_ptr<Surface> src,
  boost::shared_ptr<Surface> dst,
  const Size& screenSize, int time, int blindSize)
  : BlindEffect(machine, src, dst, screenSize, time, blindSize) {
}

// -----------------------------------------------------------------------

void BlindLeftToRightEffect::performEffectForTime(RLMachine& machine,
                                                  int currentTime) {
  computeGrowing(machine, width(), currentTime);
}

// -----------------------------------------------------------------------

void BlindLeftToRightEffect::renderPolygon(int polyStart, int polyEnd) {
  srcSurface().renderToScreen(
    Rect::GRP(polyStart, 0, polyEnd, height()),
    Rect::GRP(polyStart, 0, polyEnd, height()), 255);
}

// -----------------------------------------------------------------------
// BlindRightToLeftEffect
// -----------------------------------------------------------------------

BlindRightToLeftEffect::BlindRightToLeftEffect(
  RLMachine& machine, boost::shared_ptr<Surface> src,
  boost::shared_ptr<Surface> dst,
  const Size& screenSize, int time, int blindSize)
  : BlindEffect(machine, src, dst, screenSize, time, blindSize) {}

// -----------------------------------------------------------------------

void BlindRightToLeftEffect::performEffectForTime(RLMachine& machine,
                                                  int currentTime) {
  computeDecreasing(machine, width(), currentTime);
}

// -----------------------------------------------------------------------

void BlindRightToLeftEffect::renderPolygon(int polyStart, int polyEnd) {
  srcSurface().
    renderToScreen(
      Rect::GRP(polyEnd, 0, polyStart, height()),
      Rect::GRP(polyEnd, 0, polyStart, height()), 255);
}
