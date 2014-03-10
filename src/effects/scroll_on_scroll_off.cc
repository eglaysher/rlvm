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

#include "effects/scroll_on_scroll_off.h"

#include <cmath>

#include "machine/rlmachine.h"
#include "systems/base/system.h"
#include "systems/base/graphics_system.h"
#include "systems/base/surface.h"

// -----------------------------------------------------------------------
// ScrollOnScrollOff base class
// -----------------------------------------------------------------------
ScrollSquashSlideBaseEffect::ScrollSquashSlideBaseEffect(
    RLMachine& machine,
    boost::shared_ptr<Surface> src,
    boost::shared_ptr<Surface> dst,
    ScrollSquashSlideDrawer* drawer,
    ScrollSquashSlideEffectTypeBase* effectType,
    const Size& s,
    int time)
    : Effect(machine, src, dst, s, time),
      drawer_(drawer),
      effect_type_(effectType) {}

ScrollSquashSlideBaseEffect::~ScrollSquashSlideBaseEffect() {}

int ScrollSquashSlideBaseEffect::calculateAmountVisible(int currentTime,
                                                        int screenSize) {
  return int((float(currentTime) / duration()) * screenSize);
}

bool ScrollSquashSlideBaseEffect::blitOriginalImage() const { return false; }

void ScrollSquashSlideBaseEffect::performEffectForTime(RLMachine& machine,
                                                       int currentTime) {
  GraphicsSystem& graphics = machine.system().graphics();
  int amountVisible =
      calculateAmountVisible(currentTime, drawer_->getMaxSize(graphics));
  effect_type_->composeEffectsFor(graphics, *this, *drawer_, amountVisible);
}

// -----------------------------------------------------------------------
// Direction Interface
// -----------------------------------------------------------------------

// ------------------------------------------- [ ScrollSquashSlideDrawer ]

ScrollSquashSlideDrawer::ScrollSquashSlideDrawer() {}
ScrollSquashSlideDrawer::~ScrollSquashSlideDrawer() {}

// ------------------------------------------------- [ TopToBottomDrawer ]

int TopToBottomDrawer::getMaxSize(GraphicsSystem& gs) {
  return gs.screenSize().height();
}

void TopToBottomDrawer::scrollOff(GraphicsSystem& graphics,
                                  ScrollSquashSlideBaseEffect& effect,
                                  int amountVisible,
                                  int width,
                                  int height) {
  effect.dstSurface().renderToScreen(
      Rect::GRP(0, 0, width, height - amountVisible),
      Rect::GRP(0, amountVisible, width, height),
      255);
}

void TopToBottomDrawer::scrollOn(GraphicsSystem& graphics,
                                 ScrollSquashSlideBaseEffect& effect,
                                 int amountVisible,
                                 int width,
                                 int height) {
  effect.srcSurface().renderToScreen(
      Rect::GRP(0, height - amountVisible, width, height),
      Rect::GRP(0, 0, width, amountVisible),
      255);
}

void TopToBottomDrawer::squashOff(GraphicsSystem& graphics,
                                  ScrollSquashSlideBaseEffect& effect,
                                  int amountVisible,
                                  int width,
                                  int height) {
  effect.dstSurface().renderToScreen(Rect::GRP(0, 0, width, height),
                                     Rect::GRP(0, amountVisible, width, height),
                                     255);
}

void TopToBottomDrawer::squashOn(GraphicsSystem& graphics,
                                 ScrollSquashSlideBaseEffect& effect,
                                 int amountVisible,
                                 int width,
                                 int height) {
  effect.srcSurface().renderToScreen(Rect::GRP(0, 0, width, height),
                                     Rect::GRP(0, 0, width, amountVisible),
                                     255);
}

// ------------------------------------------------- [ BottomToTopDrawer ]

int BottomToTopDrawer::getMaxSize(GraphicsSystem& gs) {
  return gs.screenSize().height();
}

void BottomToTopDrawer::scrollOn(GraphicsSystem& graphics,
                                 ScrollSquashSlideBaseEffect& effect,
                                 int amountVisible,
                                 int width,
                                 int height) {
  effect.srcSurface().renderToScreen(
      Rect::GRP(0, 0, width, amountVisible),
      Rect::GRP(0, height - amountVisible, width, height),
      255);
}

void BottomToTopDrawer::scrollOff(GraphicsSystem& graphics,
                                  ScrollSquashSlideBaseEffect& effect,
                                  int amountVisible,
                                  int width,
                                  int height) {
  effect.dstSurface().renderToScreen(
      Rect::GRP(0, amountVisible, width, height),
      Rect::GRP(0, 0, width, height - amountVisible),
      255);
}

void BottomToTopDrawer::squashOn(GraphicsSystem& graphics,
                                 ScrollSquashSlideBaseEffect& effect,
                                 int amountVisible,
                                 int width,
                                 int height) {
  effect.srcSurface().renderToScreen(
      Rect::GRP(0, 0, width, height),
      Rect::GRP(0, height - amountVisible, width, height),
      255);
}

void BottomToTopDrawer::squashOff(GraphicsSystem& graphics,
                                  ScrollSquashSlideBaseEffect& effect,
                                  int amountVisible,
                                  int width,
                                  int height) {
  effect.dstSurface().renderToScreen(
      Rect::GRP(0, 0, width, height),
      Rect::GRP(0, 0, width, height - amountVisible),
      255);
}

// ------------------------------------------------- [ LeftToRightDrawer ]

int LeftToRightDrawer::getMaxSize(GraphicsSystem& gs) {
  return gs.screenSize().width();
}

void LeftToRightDrawer::scrollOn(GraphicsSystem& graphics,
                                 ScrollSquashSlideBaseEffect& effect,
                                 int amountVisible,
                                 int width,
                                 int height) {
  effect.srcSurface().renderToScreen(
      Rect::GRP(width - amountVisible, 0, width, height),
      Rect::GRP(0, 0, amountVisible, height),
      255);
}

void LeftToRightDrawer::scrollOff(GraphicsSystem& graphics,
                                  ScrollSquashSlideBaseEffect& effect,
                                  int amountVisible,
                                  int width,
                                  int height) {
  effect.dstSurface().renderToScreen(
      Rect::GRP(0, 0, width - amountVisible, height),
      Rect::GRP(amountVisible, 0, width, height),
      255);
}

void LeftToRightDrawer::squashOn(GraphicsSystem& graphics,
                                 ScrollSquashSlideBaseEffect& effect,
                                 int amountVisible,
                                 int width,
                                 int height) {
  effect.srcSurface().renderToScreen(Rect::GRP(0, 0, width, height),
                                     Rect::GRP(0, 0, amountVisible, height),
                                     255);
}

void LeftToRightDrawer::squashOff(GraphicsSystem& graphics,
                                  ScrollSquashSlideBaseEffect& effect,
                                  int amountVisible,
                                  int width,
                                  int height) {
  effect.dstSurface().renderToScreen(Rect::GRP(0, 0, width, height),
                                     Rect::GRP(amountVisible, 0, width, height),
                                     255);
}

// ------------------------------------------------- [ RightToLeftDrawer ]

int RightToLeftDrawer::getMaxSize(GraphicsSystem& gs) {
  return gs.screenSize().width();
}

void RightToLeftDrawer::scrollOff(GraphicsSystem& graphics,
                                  ScrollSquashSlideBaseEffect& effect,
                                  int amountVisible,
                                  int width,
                                  int height) {
  effect.dstSurface().renderToScreen(
      Rect::GRP(amountVisible, 0, width, height),
      Rect::GRP(0, 0, width - amountVisible, height),
      255);
}

void RightToLeftDrawer::scrollOn(GraphicsSystem& graphics,
                                 ScrollSquashSlideBaseEffect& effect,
                                 int amountVisible,
                                 int width,
                                 int height) {
  effect.srcSurface().renderToScreen(
      Rect::GRP(0, 0, amountVisible, height),
      Rect::GRP(width - amountVisible, 0, width, height),
      255);
}

void RightToLeftDrawer::squashOff(GraphicsSystem& graphics,
                                  ScrollSquashSlideBaseEffect& effect,
                                  int amountVisible,
                                  int width,
                                  int height) {
  effect.dstSurface().renderToScreen(
      Rect::GRP(0, 0, width, height),
      Rect::GRP(0, 0, width - amountVisible, height),
      255);
}

void RightToLeftDrawer::squashOn(GraphicsSystem& graphics,
                                 ScrollSquashSlideBaseEffect& effect,
                                 int amountVisible,
                                 int width,
                                 int height) {
  effect.srcSurface().renderToScreen(
      Rect::GRP(0, 0, width, height),
      Rect::GRP(width - amountVisible, 0, width, height),
      255);
}

// -----------------------------------------------------------------------
// Effect Type Interface
// -----------------------------------------------------------------------

ScrollSquashSlideEffectTypeBase::~ScrollSquashSlideEffectTypeBase() {}

void ScrollOnScrollOff::composeEffectsFor(GraphicsSystem& system,
                                          ScrollSquashSlideBaseEffect& effect,
                                          ScrollSquashSlideDrawer& drawer,
                                          int amountVisible) {
  Size s = system.screenSize();
  drawer.scrollOn(system, effect, amountVisible, s.width(), s.height());
  drawer.scrollOff(system, effect, amountVisible, s.width(), s.height());
}

void ScrollOnSquashOff::composeEffectsFor(GraphicsSystem& system,
                                          ScrollSquashSlideBaseEffect& effect,
                                          ScrollSquashSlideDrawer& drawer,
                                          int amountVisible) {
  Size s = system.screenSize();
  drawer.scrollOn(system, effect, amountVisible, s.width(), s.height());
  drawer.squashOff(system, effect, amountVisible, s.width(), s.height());
}

void SquashOnScrollOff::composeEffectsFor(GraphicsSystem& system,
                                          ScrollSquashSlideBaseEffect& effect,
                                          ScrollSquashSlideDrawer& drawer,
                                          int amountVisible) {
  Size s = system.screenSize();
  drawer.squashOn(system, effect, amountVisible, s.width(), s.height());
  drawer.scrollOff(system, effect, amountVisible, s.width(), s.height());
}

void SquashOnSquashOff::composeEffectsFor(GraphicsSystem& system,
                                          ScrollSquashSlideBaseEffect& effect,
                                          ScrollSquashSlideDrawer& drawer,
                                          int amountVisible) {
  Size s = system.screenSize();
  drawer.squashOn(system, effect, amountVisible, s.width(), s.height());
  drawer.squashOff(system, effect, amountVisible, s.width(), s.height());
}

void SlideOn::composeEffectsFor(GraphicsSystem& system,
                                ScrollSquashSlideBaseEffect& effect,
                                ScrollSquashSlideDrawer& drawer,
                                int amountVisible) {
  Size s = system.screenSize();
  Rect screenRect = system.screenRect();

  // Draw the old image
  effect.dstSurface().renderToScreen(screenRect, screenRect, 255);

  drawer.scrollOn(system, effect, amountVisible, s.width(), s.height());
}

void SlideOff::composeEffectsFor(GraphicsSystem& system,
                                 ScrollSquashSlideBaseEffect& effect,
                                 ScrollSquashSlideDrawer& drawer,
                                 int amountVisible) {
  Size s = system.screenSize();
  Rect screenRect = system.screenRect();

  effect.srcSurface().renderToScreen(screenRect, screenRect, 255);

  drawer.scrollOff(system, effect, amountVisible, s.width(), s.height());
}
