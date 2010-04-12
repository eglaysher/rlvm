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

#include "Effects/EffectFactory.hpp"

#include <iostream>
#include <sstream>
#include <vector>

#include "Effects/BlindEffect.hpp"
#include "Effects/FadeEffect.hpp"
#include "Effects/ScrollOnScrollOff.hpp"
#include "Effects/WipeEffect.hpp"
#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/SystemError.hpp"
#include "Utilities/Exception.hpp"
#include "Utilities/Graphics.hpp"
#include "libReallive/gameexe.h"

using namespace std;
using namespace libReallive;

const int SEL_SIZE = 16;

// -----------------------------------------------------------------------
// EffectFactory
// -----------------------------------------------------------------------

Effect* EffectFactory::buildFromSEL(RLMachine& machine,
                                    boost::shared_ptr<Surface> src,
                                    boost::shared_ptr<Surface> dst,
                                    int selNum) {
  vector<int> selParams = getSELEffect(machine, selNum);

  return build(machine, src, dst,
               selParams[6], selParams[7],
               selParams[8], selParams[9], selParams[10], selParams[11],
               selParams[12], selParams[13], selParams[15]);
}

Effect* EffectFactory::build(
    RLMachine& machine, boost::shared_ptr<Surface> src,
    boost::shared_ptr<Surface> dst,
    int time, int style,
    int direction, int interpolation, int xsize, int ysize, int a, int b,
    int c) {
  Size screenSize = machine.system().graphics().screenSize();

  // There is a completely ridiculous number of transitions here! Damn
  // you, VisualArts, for making something so simple sounding so
  // confusing and hard to implement!
  switch (style) {
    case 10:
      return buildWipeEffect(machine, src, dst, screenSize, time,
                             direction, interpolation);
      // We have the bunch of similar effects that are all implemented by
      // ScrollSquashSlideBaseEffect
    case 15:
    case 16:
    case 17:
    case 18:
    case 20:
    case 21: {
      ScrollSquashSlideDrawer* drawer = buildScrollSquashSlideDrawer(direction);
      ScrollSquashSlideEffectTypeBase* effect =
          buildScrollSquashSlideTypeBase(style);
      return new ScrollSquashSlideBaseEffect(machine, src, dst, drawer, effect,
                                             screenSize, time);
    }
    case 120:
      return buildBlindEffect(machine, src, dst, screenSize, time,
                              direction, xsize, ysize);
    case 0:
    case 50:
    default:
      return new FadeEffect(machine, src, dst, screenSize, time);
  }
}

// -----------------------------------------------------------------------
// Private methods
// -----------------------------------------------------------------------

/// Which direction we wipe in
enum ScreenDirection {
  TOP_TO_BOTTOM = 0,  // From the top to the bottom
  BOTTOM_TO_TOP = 1,  // From the bottom to the top
  LEFT_TO_RIGHT = 2,  // From left to right
  RIGHT_TO_LEFT = 3   // From right to left
};

Effect* EffectFactory::buildWipeEffect(
    RLMachine& machine, boost::shared_ptr<Surface> src,
    boost::shared_ptr<Surface> dst, const Size& screenSize, int time,
    int direction, int interpolation) {
  switch (direction) {
    case TOP_TO_BOTTOM:
      return new WipeTopToBottomEffect(machine, src, dst, screenSize,
                                       time, interpolation);
    case BOTTOM_TO_TOP:
      return new WipeBottomToTopEffect(machine, src, dst, screenSize,
                                       time, interpolation);
    case LEFT_TO_RIGHT:
      return new WipeLeftToRightEffect(machine, src, dst, screenSize,
                                       time, interpolation);
    case RIGHT_TO_LEFT:
      return new WipeRightToLeftEffect(machine, src, dst, screenSize,
                                       time, interpolation);
    default:
      cerr << "WARNING! Unsupported direction " << direction
           << " in EffectFactory::buildWipeEffect. Returning Top to"
           << " Bottom effect." << endl;
      return new WipeTopToBottomEffect(machine, src, dst, screenSize,
                                       time, interpolation);
  };
}

Effect* EffectFactory::buildBlindEffect(
    RLMachine& machine, boost::shared_ptr<Surface> src,
    boost::shared_ptr<Surface> dst, const Size& screenSize, int time,
    int direction, int xsize, int ysize) {
  // RL does something really weird: if the wrong xsize/ysize was set
  // (the correct one is zero), it uses the other.
  switch (direction) {
    case TOP_TO_BOTTOM:
      if (xsize == 0 && ysize > 0)
        xsize = ysize;
      return new BlindTopToBottomEffect(machine, src, dst, screenSize,
                                        time, xsize);
    case BOTTOM_TO_TOP:
      if (xsize == 0 && ysize > 0)
        xsize = ysize;
      return new BlindBottomToTopEffect(machine, src, dst, screenSize,
                                        time, xsize);
    case LEFT_TO_RIGHT:
      if (ysize == 0 && xsize > 0)
        ysize = xsize;
      return new BlindLeftToRightEffect(machine, src, dst, screenSize,
                                        time, ysize);
    case RIGHT_TO_LEFT:
      if (ysize == 0 && xsize > 0)
        ysize = xsize;
      return new BlindRightToLeftEffect(machine, src, dst, screenSize,
                                        time, ysize);

    default:
      cerr << "WARNING! Unsupported direction " << direction
           << " in EffectFactory::buildWipeEffect. Returning Top to"
           << " Bottom effect." << endl;
      if (xsize == 0 && ysize > 0)
        xsize = ysize;
      return new BlindTopToBottomEffect(machine, src, dst, screenSize,
                                        time, xsize);
  };
}

ScrollSquashSlideDrawer* EffectFactory::buildScrollSquashSlideDrawer(
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
      cerr << "WARNING! Unsupported direction " << drawerType
           << " in EffectFactory::buildWipeEffect. Returning Top to"
           << " Bottom effect." << endl;
      return new TopToBottomDrawer;
  };
}

ScrollSquashSlideEffectTypeBase* EffectFactory::buildScrollSquashSlideTypeBase(
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
      throw SystemError("Impossible style number in "
                        "EffectFactory::buildScrollSquashSlideTypeBase");
  };
}
