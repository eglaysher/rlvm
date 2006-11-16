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
#include "Modules/SlideEffect.hpp"

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

LongOperation* EffectFactory::buildFromSEL(RLMachine& machine, int selNum)
{
  Gameexe& gexe = machine.system().gameexe();
  vector<int> selParams = gexe("SEL", selNum).to_intVector();

  return build(machine, 
// selParams[0], selParams[1], selParams[2], selParams[3],
//                selParams[4], selParams[5], 
               selParams[6], selParams[7], 
               selParams[8], selParams[9], selParams[10], selParams[11],
               selParams[12], selParams[13], selParams[15]);
}

// -----------------------------------------------------------------------

LongOperation* EffectFactory::buildFromSELR(RLMachine& machine, int selNum)
{
  Gameexe& gexe = machine.system().gameexe();
  vector<int> selParams = gexe("SELR", selNum).to_intVector();

  return build(machine, 
               selParams[6], selParams[7], 
               selParams[8], selParams[9], selParams[10], selParams[11],
               selParams[12], selParams[13], selParams[15]);  
}

// -----------------------------------------------------------------------

//  int x, int y, int width, int height, int dx, int dy, 
LongOperation* EffectFactory::build(
  RLMachine& machine, int time, int style,
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
    return buildWipeEffect(machine, width, height, time, 
                           direction, interpolation);
  case 15:
    return buildSlideEffect(machine, width, height, time, 
                           direction);
  case 0:
  case 50:
  default:
    return new FadeEffect(machine, width, height, time);
  }

  stringstream ss;
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
 * Creates a specific subclass of WipeEffect for #SEL #10, Wipe.
 */
LongOperation* EffectFactory::buildWipeEffect(
  RLMachine& machine, int width, int height, int time, 
  int direction, int interpolation)
{
  switch(direction)
  {
  case TOP_TO_BOTTOM:
    return new WipeTopToBottomEffect(machine, width, height, 
                                     time, interpolation);
  case BOTTOM_TO_TOP:
    return new WipeBottomToTopEffect(machine, width, height, 
                                     time, interpolation);
  case LEFT_TO_RIGHT:
    return new WipeLeftToRightEffect(machine, width, height,
                                     time, interpolation);
  case RIGHT_TO_LEFT:
    return new WipeRightToLeftEffect(machine, width, height,
                                     time, interpolation);
  default:
    cerr << "WARNING! Unsupported direction " << direction 
         << " in EffectFactory::buildWipeEffect. Returning Top to"
         << " Bottom effect." << endl;
    return new WipeTopToBottomEffect(machine, width, height, 
                                     time, interpolation);
  };
}

// -----------------------------------------------------------------------

/** 
 * Creates a specific subclass of SlideEffect for #SEL #10, Slide.
 */
LongOperation* EffectFactory::buildSlideEffect(
  RLMachine& machine, int width, int height, int time, int direction)
{
  switch(direction)
  {
  case TOP_TO_BOTTOM:
    return new SlideTopToBottomEffect(machine, width, height, 
                                      time);
  case BOTTOM_TO_TOP:
    return new SlideBottomToTopEffect(machine, width, height, 
                                      time);
  case LEFT_TO_RIGHT:
    return new SlideLeftToRightEffect(machine, width, height,
                                      time);
  case RIGHT_TO_LEFT:
    return new SlideLeftToRightEffect(machine, width, height,
                                     time);

  default:
    cerr << "WARNING! Unsupported direction " << direction 
         << " in EffectFactory::buildSlideEffect. Returning Top to"
         << " Bottom effect." << endl;
    return new SlideTopToBottomEffect(machine, width, height, 
                                      time);
  };
}
