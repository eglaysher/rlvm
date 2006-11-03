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

#include <iostream>
#include <sstream>

#include "libReallive/defs.h"

using namespace std;
using namespace libReallive;

/** 
 * Returns a constructed LongOperation with the following properties
 * to perform a transition.
 * 
 * @param filename      Image file to use
 * @param x             Source x coordinate
 * @param y             Source y coordinate
 * @param width         Source width
 * @param height        Source hight
 * @param dx            Destination x coordinate
 * @param dy            Destination y coordinate
 * @param time          Length of transition, in ms.
 * @param style         The style of transition. This factory does a
 *                      big switch statement on this value.
 * @param direction     For wipes and pans, sets the wipe direction.
 * @param interpolation Smooths certain transitions. For For dithered
 *                      fades, interpolation N adds N intermediate
 *                      steps so that the transition fades between
 *                      patterns rather than stepping between them.
 *                      For wipes, interpolation N replaces the hard
 *                      boundary with a soft edge of thickness roughly
 *                      2^N * 2.5 pixels.
 * @param xsize         X size of pattern in some transitions
 * @param ysize         Y size of pattern in some transitions
 * @param a             Unknown
 * @param b             Unknown
 * @param opacity       Opacity of the new image composited onto the
 *                      old image in DC1. (DC1 is then copied onto DC0
 *                      with this transition object.)
 * @param c             Unknown
 * 
 * @return A LongOperation which will perform the following transition
 *         and then exit.
 */
LongOperation* EffectFactory::build(RLMachine& machine, 
  int x, int y, int width, int height, int dx, int dy, int time, int style,
  int direction, int interpolation, int xsize, int ysize, int a, int b,
  int opacity, int c)
{
  // There is a completely ridaculous number of transitions here! Damn
  // you, VisualArts, for making something so simple sounding so
  // confusing and hard to implement!
  switch(style)
  {
  case 10:
    return new WipeEffect(machine, x, y, width, height, dx, dy, time, 
                          direction, interpolation);
  case 0:
  case 50:
  default:
    cerr << "FadeEffect(" << x << "," << y << "," << width << "," 
         << height << "," << dx << "," << dy << "," << time << ")"
         << endl;
    return new FadeEffect(machine, x, y, width, height, dx, dy, time);
  }

  stringstream ss;
  ss << "Unsupported effect number in #SEL:" << style;
  throw Error(ss.str());
}
