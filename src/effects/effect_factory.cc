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

#include "effects/effect_factory.h"

#include <iostream>
#include <sstream>
#include <vector>

#include "effects/blind_effect.h"
#include "effects/fade_effect.h"
#include "effects/scroll_on_scroll_off.h"
#include "effects/wipe_effect.h"
#include "libreallive/gameexe.h"
#include "machine/rlmachine.h"
#include "systems/base/graphics_system.h"
#include "systems/base/surface.h"
#include "systems/base/system.h"
#include "systems/base/system_error.h"
#include "utilities/exception.h"
#include "utilities/graphics.h"

// -----------------------------------------------------------------------
// EffectFactory
// -----------------------------------------------------------------------

Effect* EffectFactory::BuildFromSEL(RLMachine& machine,
                                    std::shared_ptr<Surface> src,
                                    std::shared_ptr<Surface> dst,
                                    int selNum) {
  std::vector<int> sel_params = GetSELEffect(machine, selNum);

  return Build(machine,
               src,
               dst,
               sel_params[6],
               sel_params[7],
               sel_params[8],
               sel_params[9],
               sel_params[10],
               sel_params[11],
               sel_params[12],
               sel_params[13],
               sel_params[15]);
}

Effect* EffectFactory::Build(RLMachine& machine,
                             std::shared_ptr<Surface> src,
                             std::shared_ptr<Surface> dst,
                             int time,
                             int style,
                             int direction,
                             int interpolation,
                             int xsize,
                             int ysize,
                             int a,
                             int b,
                             int c) {
  Size screen_size = machine.system().graphics().screen_size();

  // Ensure that both of our images are on the graphics card so we don't
  // stutter during the loop. These can be NULL in unit tests.
  if (src)
    src->EnsureUploaded();
  if (dst)
    dst->EnsureUploaded();

  // There is a completely ridiculous number of transitions here! Damn
  // you, VisualArts, for making something so simple sounding so
  // confusing and hard to implement!
  switch (style) {
    case 10:
      return BuildWipeEffect(
          machine, src, dst, screen_size, time, direction, interpolation);
    // We have the bunch of similar effects that are all implemented by
    // ScrollSquashSlideBaseEffect
    case 15:
    case 16:
    case 17:
    case 18:
    case 20:
    case 21: {
      ScrollSquashSlideDrawer* drawer = BuildScrollSquashSlideDrawer(direction);
      ScrollSquashSlideEffectTypeBase* effect =
          BuildScrollSquashSlideTypeBase(style);
      return new ScrollSquashSlideBaseEffect(
          machine, src, dst, drawer, effect, screen_size, time);
    }
    case 120:
      return BuildBlindEffect(
          machine, src, dst, screen_size, time, direction, xsize, ysize);
    case 0:
    case 50:
    default:
      return new FadeEffect(machine, src, dst, screen_size, time);
  }
}

// -----------------------------------------------------------------------
// Private methods
// -----------------------------------------------------------------------

// Which direction we wipe in
enum ScreenDirection {
  TOP_TO_BOTTOM = 0,  // From the top to the bottom
  BOTTOM_TO_TOP = 1,  // From the bottom to the top
  LEFT_TO_RIGHT = 2,  // From left to right
  RIGHT_TO_LEFT = 3   // From right to left
};

Effect* EffectFactory::BuildWipeEffect(RLMachine& machine,
                                       std::shared_ptr<Surface> src,
                                       std::shared_ptr<Surface> dst,
                                       const Size& screen_size,
                                       int time,
                                       int direction,
                                       int interpolation) {
  switch (direction) {
    case TOP_TO_BOTTOM:
      return new WipeTopToBottomEffect(
          machine, src, dst, screen_size, time, interpolation);
    case BOTTOM_TO_TOP:
      return new WipeBottomToTopEffect(
          machine, src, dst, screen_size, time, interpolation);
    case LEFT_TO_RIGHT:
      return new WipeLeftToRightEffect(
          machine, src, dst, screen_size, time, interpolation);
    case RIGHT_TO_LEFT:
      return new WipeRightToLeftEffect(
          machine, src, dst, screen_size, time, interpolation);
    default:
      std::cerr << "WARNING! Unsupported direction " << direction
                << " in EffectFactory::buildWipeEffect. Returning Top to"
                << " Bottom effect." << std::endl;
      return new WipeTopToBottomEffect(
          machine, src, dst, screen_size, time, interpolation);
  }
}

Effect* EffectFactory::BuildBlindEffect(RLMachine& machine,
                                        std::shared_ptr<Surface> src,
                                        std::shared_ptr<Surface> dst,
                                        const Size& screen_size,
                                        int time,
                                        int direction,
                                        int xsize,
                                        int ysize) {
  // RL does something really weird: if the wrong xsize/ysize was set
  // (the correct one is zero), it uses the other.
  switch (direction) {
    case TOP_TO_BOTTOM:
      if (xsize == 0 && ysize > 0)
        xsize = ysize;
      return new BlindTopToBottomEffect(
          machine, src, dst, screen_size, time, xsize);
    case BOTTOM_TO_TOP:
      if (xsize == 0 && ysize > 0)
        xsize = ysize;
      return new BlindBottomToTopEffect(
          machine, src, dst, screen_size, time, xsize);
    case LEFT_TO_RIGHT:
      if (ysize == 0 && xsize > 0)
        ysize = xsize;
      return new BlindLeftToRightEffect(
          machine, src, dst, screen_size, time, ysize);
    case RIGHT_TO_LEFT:
      if (ysize == 0 && xsize > 0)
        ysize = xsize;
      return new BlindRightToLeftEffect(
          machine, src, dst, screen_size, time, ysize);

    default:
      std::cerr << "WARNING! Unsupported direction " << direction
                << " in EffectFactory::buildWipeEffect. Returning Top to"
                << " Bottom effect." << std::endl;
      if (xsize == 0 && ysize > 0)
        xsize = ysize;
      return new BlindTopToBottomEffect(
          machine, src, dst, screen_size, time, xsize);
  }
}

ScrollSquashSlideDrawer* EffectFactory::BuildScrollSquashSlideDrawer(
    int drawerType) {
  switch (drawerType) {
    case TOP_TO_BOTTOM:
      return new TopToBottomDrawer;
    case BOTTOM_TO_TOP:
      return new BottomToTopDrawer;
    case LEFT_TO_RIGHT:
      return new LeftToRightDrawer;
    case RIGHT_TO_LEFT:
      return new RightToLeftDrawer;
    default:
      std::cerr << "WARNING! Unsupported direction " << drawerType
                << " in EffectFactory::buildWipeEffect. Returning Top to"
                << " Bottom effect." << std::endl;
      return new TopToBottomDrawer;
  }
}

ScrollSquashSlideEffectTypeBase* EffectFactory::BuildScrollSquashSlideTypeBase(
    int style) {
  switch (style) {
    case 15:
      return new ScrollOnScrollOff;
    case 16:
      return new ScrollOnSquashOff;
    case 17:
      return new SquashOnScrollOff;
    case 18:
      return new SquashOnSquashOff;
    case 20:
      return new SlideOn;
    case 21:
      return new SlideOff;
    default:
      throw SystemError(
          "Impossible style number in "
          "EffectFactory::buildScrollSquashSlideTypeBase");
  }
}
