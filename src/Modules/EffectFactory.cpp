// This file is part of RLVM, a RealLive virutal machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006 El Riot
//  
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
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
 * @file   EffectFactory.cpp
 * @author Elliot Glaysher
 * @date   Thu Nov  2 20:36:53 2006
 * 
 * @brief  Factory that creates all Effect s.
 */

#include "Modules/EffectFactory.hpp"
#include "Modules/FadeEffect.hpp"
#include "Modules/WipeEffect.hpp"
#include "Modules/ScrollOnScrollOff.hpp"

#include <iostream>
#include <sstream>

//#include "libReallive/defs.h"
#include "libReallive/gameexe.h"

#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/GraphicsSystem.hpp"

using namespace std;
using namespace libReallive;

const int SEL_SIZE = 16;

// -----------------------------------------------------------------------

Effect* EffectFactory::buildFromSEL(RLMachine& machine, 
                                           boost::shared_ptr<Surface> src, 
                                           boost::shared_ptr<Surface> dst,
                                           boost::shared_ptr<Surface> final,
                                           int selNum)
{
  Gameexe& gexe = machine.system().gameexe();
  vector<int> selParams = gexe("SEL", selNum).to_intVector();

  return build(machine, src, dst, final, 
               selParams[6], selParams[7], 
               selParams[8], selParams[9], selParams[10], selParams[11],
               selParams[12], selParams[13], selParams[15]);
}

// -----------------------------------------------------------------------

Effect* EffectFactory::buildFromSELR(RLMachine& machine, 
                                            boost::shared_ptr<Surface> src, 
                                            boost::shared_ptr<Surface> dst,
                                            boost::shared_ptr<Surface> final,
                                            int selNum)
{
  Gameexe& gexe = machine.system().gameexe();
  vector<int> selParams = gexe("SELR", selNum).to_intVector();

  return build(machine, src, dst, final,
               selParams[6], selParams[7], 
               selParams[8], selParams[9], selParams[10], selParams[11],
               selParams[12], selParams[13], selParams[15]);  
}

// -----------------------------------------------------------------------

//  int x, int y, int width, int height, int dx, int dy, 
Effect* EffectFactory::build(
  RLMachine& machine, boost::shared_ptr<Surface> src, 
  boost::shared_ptr<Surface> dst, 
  boost::shared_ptr<Surface> final,
  int time, int style,
  int direction, int interpolation, int xsize, int ysize, int a, int b,
  int c)
{
  int width = machine.system().graphics().screenWidth();
  int height = machine.system().graphics().screenHeight();

  // There is a completely ridaculous number of transitions here! Damn
  // you, VisualArts, for making something so simple sounding so
  // confusing and hard to implement!
  switch(style)
  {
  case 10:
    return buildWipeEffect(machine, src, dst, final, width, height, time, 
                           direction, interpolation);
  // We have the bunch of similar effects that are all implemented by
  // ScrollSquashSlideBaseEffect
  case 15:
  case 16:
  case 17:
  case 18:
  case 20:
  case 21:
  {
    ScrollSquashSlideDrawer* drawer = buildScrollSquashSlideDrawer(direction);
    ScrollSquashSlideEffectTypeBase* effect = buildScrollSquashSlideTypeBase(style);
    return new ScrollSquashSlideBaseEffect(machine, src, dst, final, drawer, effect,
                                           width, height, time);
  }
  case 0:
  case 50:
  default:
    return new FadeEffect(machine, src, dst, final, width, height, time);
  }

  ostringstream ss;
  ss << "Unsupported effect number in #SEL:" << style;
  throw Error(ss.str());
}

// -----------------------------------------------------------------------
// Private methods
// -----------------------------------------------------------------------

/// Which direction we wipe in 
enum ScreenDirection {
  TOP_TO_BOTTOM = 0, ///< From the top to the bottom
  BOTTOM_TO_TOP = 1, ///< From the bottom to the top
  LEFT_TO_RIGHT = 2, ///< From left to right
  RIGHT_TO_LEFT = 3  ///< From right to left
};

// -----------------------------------------------------------------------

/** 
 * Creates a specific subclass of WipeEffect for \#SEL #10, Wipe.
 */
Effect* EffectFactory::buildWipeEffect(
  RLMachine& machine, boost::shared_ptr<Surface> src,
  boost::shared_ptr<Surface> dst,
  boost::shared_ptr<Surface> final,
  int width, int height, int time, 
  int direction, int interpolation)
{
  switch(direction)
  {
  case TOP_TO_BOTTOM:
    return new WipeTopToBottomEffect(machine, src, dst, final, width, height, 
                                     time, interpolation);
  case BOTTOM_TO_TOP:
    return new WipeBottomToTopEffect(machine, src, dst, final, width, height, 
                                     time, interpolation);
  case LEFT_TO_RIGHT:
    return new WipeLeftToRightEffect(machine, src, dst, final, width, height,
                                     time, interpolation);
  case RIGHT_TO_LEFT:
    return new WipeRightToLeftEffect(machine, src, dst, final, width, height,
                                     time, interpolation);
  default:
    cerr << "WARNING! Unsupported direction " << direction 
         << " in EffectFactory::buildWipeEffect. Returning Top to"
         << " Bottom effect." << endl;
    return new WipeTopToBottomEffect(machine, src, dst, final, width, height, 
                                     time, interpolation);
  };
}

// -----------------------------------------------------------------------

ScrollSquashSlideDrawer* EffectFactory::buildScrollSquashSlideDrawer(
  int drawerType)
{
  switch(drawerType)
  {
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

// -----------------------------------------------------------------------

ScrollSquashSlideEffectTypeBase* EffectFactory::buildScrollSquashSlideTypeBase(
  int style)
{
  switch(style)
  {
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
    throw libReallive::Error("Impossible style number in EffectFactory::buildScrollSquashSlideTypeBase");
  };
}
