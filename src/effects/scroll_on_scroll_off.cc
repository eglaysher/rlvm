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
    std::shared_ptr<Surface> src,
    std::shared_ptr<Surface> dst,
    ScrollSquashSlideDrawer* drawer,
    ScrollSquashSlideEffectTypeBase* effect_type,
    const Size& s,
    int time)
    : Effect(machine, src, dst, s, time),
      drawer_(drawer),
      effect_type_(effect_type) {}

ScrollSquashSlideBaseEffect::~ScrollSquashSlideBaseEffect() {}

int ScrollSquashSlideBaseEffect::CalculateAmountVisible(int current_time,
                                                        int screen_size) {
  return int((float(current_time) / duration()) * screen_size);
}

bool ScrollSquashSlideBaseEffect::BlitOriginalImage() const { return false; }

void ScrollSquashSlideBaseEffect::PerformEffectForTime(RLMachine& machine,
                                                       int current_time) {
  GraphicsSystem& graphics = machine.system().graphics();
  int amount_visible =
      CalculateAmountVisible(current_time, drawer_->GetMaxSize(graphics));
  effect_type_->ComposeEffectsFor(graphics, *this, *drawer_, amount_visible);
}

// -----------------------------------------------------------------------
// Direction Interface
// -----------------------------------------------------------------------

// ------------------------------------------- [ ScrollSquashSlideDrawer ]

ScrollSquashSlideDrawer::ScrollSquashSlideDrawer() {}
ScrollSquashSlideDrawer::~ScrollSquashSlideDrawer() {}

// ------------------------------------------------- [ TopToBottomDrawer ]

int TopToBottomDrawer::GetMaxSize(GraphicsSystem& gs) {
  return gs.screen_size().height();
}

void TopToBottomDrawer::ScrollOff(GraphicsSystem& graphics,
                                  ScrollSquashSlideBaseEffect& effect,
                                  int amount_visible,
                                  int width,
                                  int height) {
  effect.dst_surface().RenderToScreen(
      Rect::GRP(0, 0, width, height - amount_visible),
      Rect::GRP(0, amount_visible, width, height),
      255);
}

void TopToBottomDrawer::ScrollOn(GraphicsSystem& graphics,
                                 ScrollSquashSlideBaseEffect& effect,
                                 int amount_visible,
                                 int width,
                                 int height) {
  effect.src_surface().RenderToScreen(
      Rect::GRP(0, height - amount_visible, width, height),
      Rect::GRP(0, 0, width, amount_visible),
      255);
}

void TopToBottomDrawer::SquashOff(GraphicsSystem& graphics,
                                  ScrollSquashSlideBaseEffect& effect,
                                  int amount_visible,
                                  int width,
                                  int height) {
  effect.dst_surface().RenderToScreen(
      Rect::GRP(0, 0, width, height),
      Rect::GRP(0, amount_visible, width, height),
      255);
}

void TopToBottomDrawer::SquashOn(GraphicsSystem& graphics,
                                 ScrollSquashSlideBaseEffect& effect,
                                 int amount_visible,
                                 int width,
                                 int height) {
  effect.src_surface().RenderToScreen(Rect::GRP(0, 0, width, height),
                                      Rect::GRP(0, 0, width, amount_visible),
                                      255);
}

// ------------------------------------------------- [ BottomToTopDrawer ]

int BottomToTopDrawer::GetMaxSize(GraphicsSystem& gs) {
  return gs.screen_size().height();
}

void BottomToTopDrawer::ScrollOn(GraphicsSystem& graphics,
                                 ScrollSquashSlideBaseEffect& effect,
                                 int amount_visible,
                                 int width,
                                 int height) {
  effect.src_surface().RenderToScreen(
      Rect::GRP(0, 0, width, amount_visible),
      Rect::GRP(0, height - amount_visible, width, height),
      255);
}

void BottomToTopDrawer::ScrollOff(GraphicsSystem& graphics,
                                  ScrollSquashSlideBaseEffect& effect,
                                  int amount_visible,
                                  int width,
                                  int height) {
  effect.dst_surface().RenderToScreen(
      Rect::GRP(0, amount_visible, width, height),
      Rect::GRP(0, 0, width, height - amount_visible),
      255);
}

void BottomToTopDrawer::SquashOn(GraphicsSystem& graphics,
                                 ScrollSquashSlideBaseEffect& effect,
                                 int amount_visible,
                                 int width,
                                 int height) {
  effect.src_surface().RenderToScreen(
      Rect::GRP(0, 0, width, height),
      Rect::GRP(0, height - amount_visible, width, height),
      255);
}

void BottomToTopDrawer::SquashOff(GraphicsSystem& graphics,
                                  ScrollSquashSlideBaseEffect& effect,
                                  int amount_visible,
                                  int width,
                                  int height) {
  effect.dst_surface().RenderToScreen(
      Rect::GRP(0, 0, width, height),
      Rect::GRP(0, 0, width, height - amount_visible),
      255);
}

// ------------------------------------------------- [ LeftToRightDrawer ]

int LeftToRightDrawer::GetMaxSize(GraphicsSystem& gs) {
  return gs.screen_size().width();
}

void LeftToRightDrawer::ScrollOn(GraphicsSystem& graphics,
                                 ScrollSquashSlideBaseEffect& effect,
                                 int amount_visible,
                                 int width,
                                 int height) {
  effect.src_surface().RenderToScreen(
      Rect::GRP(width - amount_visible, 0, width, height),
      Rect::GRP(0, 0, amount_visible, height),
      255);
}

void LeftToRightDrawer::ScrollOff(GraphicsSystem& graphics,
                                  ScrollSquashSlideBaseEffect& effect,
                                  int amount_visible,
                                  int width,
                                  int height) {
  effect.dst_surface().RenderToScreen(
      Rect::GRP(0, 0, width - amount_visible, height),
      Rect::GRP(amount_visible, 0, width, height),
      255);
}

void LeftToRightDrawer::SquashOn(GraphicsSystem& graphics,
                                 ScrollSquashSlideBaseEffect& effect,
                                 int amount_visible,
                                 int width,
                                 int height) {
  effect.src_surface().RenderToScreen(Rect::GRP(0, 0, width, height),
                                      Rect::GRP(0, 0, amount_visible, height),
                                      255);
}

void LeftToRightDrawer::SquashOff(GraphicsSystem& graphics,
                                  ScrollSquashSlideBaseEffect& effect,
                                  int amount_visible,
                                  int width,
                                  int height) {
  effect.dst_surface().RenderToScreen(
      Rect::GRP(0, 0, width, height),
      Rect::GRP(amount_visible, 0, width, height),
      255);
}

// ------------------------------------------------- [ RightToLeftDrawer ]

int RightToLeftDrawer::GetMaxSize(GraphicsSystem& gs) {
  return gs.screen_size().width();
}

void RightToLeftDrawer::ScrollOff(GraphicsSystem& graphics,
                                  ScrollSquashSlideBaseEffect& effect,
                                  int amount_visible,
                                  int width,
                                  int height) {
  effect.dst_surface().RenderToScreen(
      Rect::GRP(amount_visible, 0, width, height),
      Rect::GRP(0, 0, width - amount_visible, height),
      255);
}

void RightToLeftDrawer::ScrollOn(GraphicsSystem& graphics,
                                 ScrollSquashSlideBaseEffect& effect,
                                 int amount_visible,
                                 int width,
                                 int height) {
  effect.src_surface().RenderToScreen(
      Rect::GRP(0, 0, amount_visible, height),
      Rect::GRP(width - amount_visible, 0, width, height),
      255);
}

void RightToLeftDrawer::SquashOff(GraphicsSystem& graphics,
                                  ScrollSquashSlideBaseEffect& effect,
                                  int amount_visible,
                                  int width,
                                  int height) {
  effect.dst_surface().RenderToScreen(
      Rect::GRP(0, 0, width, height),
      Rect::GRP(0, 0, width - amount_visible, height),
      255);
}

void RightToLeftDrawer::SquashOn(GraphicsSystem& graphics,
                                 ScrollSquashSlideBaseEffect& effect,
                                 int amount_visible,
                                 int width,
                                 int height) {
  effect.src_surface().RenderToScreen(
      Rect::GRP(0, 0, width, height),
      Rect::GRP(width - amount_visible, 0, width, height),
      255);
}

// -----------------------------------------------------------------------
// Effect Type Interface
// -----------------------------------------------------------------------

ScrollSquashSlideEffectTypeBase::~ScrollSquashSlideEffectTypeBase() {}

void ScrollOnScrollOff::ComposeEffectsFor(GraphicsSystem& system,
                                          ScrollSquashSlideBaseEffect& effect,
                                          ScrollSquashSlideDrawer& drawer,
                                          int amount_visible) {
  Size s = system.screen_size();
  drawer.ScrollOn(system, effect, amount_visible, s.width(), s.height());
  drawer.ScrollOff(system, effect, amount_visible, s.width(), s.height());
}

void ScrollOnSquashOff::ComposeEffectsFor(GraphicsSystem& system,
                                          ScrollSquashSlideBaseEffect& effect,
                                          ScrollSquashSlideDrawer& drawer,
                                          int amount_visible) {
  Size s = system.screen_size();
  drawer.ScrollOn(system, effect, amount_visible, s.width(), s.height());
  drawer.SquashOff(system, effect, amount_visible, s.width(), s.height());
}

void SquashOnScrollOff::ComposeEffectsFor(GraphicsSystem& system,
                                          ScrollSquashSlideBaseEffect& effect,
                                          ScrollSquashSlideDrawer& drawer,
                                          int amount_visible) {
  Size s = system.screen_size();
  drawer.SquashOn(system, effect, amount_visible, s.width(), s.height());
  drawer.ScrollOff(system, effect, amount_visible, s.width(), s.height());
}

void SquashOnSquashOff::ComposeEffectsFor(GraphicsSystem& system,
                                          ScrollSquashSlideBaseEffect& effect,
                                          ScrollSquashSlideDrawer& drawer,
                                          int amount_visible) {
  Size s = system.screen_size();
  drawer.SquashOn(system, effect, amount_visible, s.width(), s.height());
  drawer.SquashOff(system, effect, amount_visible, s.width(), s.height());
}

void SlideOn::ComposeEffectsFor(GraphicsSystem& system,
                                ScrollSquashSlideBaseEffect& effect,
                                ScrollSquashSlideDrawer& drawer,
                                int amount_visible) {
  Size s = system.screen_size();
  Rect screen_rect = system.screen_rect();

  // Draw the old image
  effect.dst_surface().RenderToScreen(screen_rect, screen_rect, 255);

  drawer.ScrollOn(system, effect, amount_visible, s.width(), s.height());
}

void SlideOff::ComposeEffectsFor(GraphicsSystem& system,
                                 ScrollSquashSlideBaseEffect& effect,
                                 ScrollSquashSlideDrawer& drawer,
                                 int amount_visible) {
  Size s = system.screen_size();
  Rect screen_rect = system.screen_rect();

  effect.src_surface().RenderToScreen(screen_rect, screen_rect, 255);

  drawer.ScrollOff(system, effect, amount_visible, s.width(), s.height());
}
