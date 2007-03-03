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
 * @file   EffectFactory.hpp
 * @author Elliot Glaysher
 * @date   Thu Nov  2 20:37:58 2006
 * @ingroup TransitionEffects
 * @brief  Factory that creates all Effect s.
 */

#ifndef __EffectFactory_hpp__
#define __EffectFactory_hpp__

#include <boost/shared_ptr.hpp>

class RLMachine;

class Surface;
class Effect;
class ScrollSquashSlideDrawer;
class ScrollSquashSlideEffectTypeBase;

/**
 * Factory that creates all Effect s. This factory is called with
 * either a Gameexe and the \#SEL or \#SELR number, or it is passed the
 * equivalent parameters.
 *
 * @ingroup TransitionEffects
 */
class EffectFactory
{
public:
  /** 
   * Builds an Effect based off the \#SEL.selnum line in the
   * Gameexe.ini file. The coordinates, which are in grp* format (x1,
   * y1, x2, y2), are converted to rec* format and then based to the
   * build() method.
   * 
   * @param machine  Machine to read Gameexe from
   * @param selnum   \#SEL to read
   * @throw Error selnum is not a valid \#SEL number, Gameexe file is
   *              malformed.
   * @return A LongOperation which will perform the following transition
   *         and then exit.
   */
  static Effect* buildFromSEL(RLMachine& machine, 
                              boost::shared_ptr<Surface> src, 
                              boost::shared_ptr<Surface> dst,
                              boost::shared_ptr<Surface> final,
                              int selnum);

  /** 
   * Builds an Effect based off the \#SELR.selnum line in the
   * Gameexe.ini file. The
   * 
   * @param machine  Machine to read Gameexe from
   * @param selnum   \#SELR to read
   * @throw Error selnum is not a valid \#SELR number, Gameexe file is
   *              malformed.
   * @return A LongOperation which will perform the following transition
   *         and then exit.
   */  
  static Effect* buildFromSELR(RLMachine& machine, 
                               boost::shared_ptr<Surface> src, 
                               boost::shared_ptr<Surface> dst,
                               boost::shared_ptr<Surface> final,
                               int selnum);

  /** 
   * Returns a constructed LongOperation with the following properties
   * to perform a transition. Note: Effect's external interface work on
   * the rec* coordinate system (x, y, width, height) instead of the
   * grp* coordinate system (x1, y1, x2, y2) .
   * 
   * @param machine       Machine to construct with
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
   * @param c             Unknown
   * 
   * @return A LongOperation which will perform the following transition
   *         and then exit.
   */
  static Effect* build(
    RLMachine& machine, boost::shared_ptr<Surface> src, 
    boost::shared_ptr<Surface> dst,
    boost::shared_ptr<Surface> final,  
    int time, int style,
    int direction, int interpolation, int xsize, int ysize, int a, int b,
    int c);

private:
  static Effect* buildWipeEffect(
    RLMachine& machine, boost::shared_ptr<Surface> src, 
    boost::shared_ptr<Surface> dst,
    boost::shared_ptr<Surface> final,  
    int width, int height, int time, int direction, int interpolation);

  static Effect* buildBlindEffect(
    RLMachine& machine, boost::shared_ptr<Surface> src,
    boost::shared_ptr<Surface> dst,
    boost::shared_ptr<Surface> final,
    int width, int height, int time, 
    int direction, int xsize, int ysize);

  static ScrollSquashSlideDrawer* buildScrollSquashSlideDrawer(
    int drawerType);

  static ScrollSquashSlideEffectTypeBase* buildScrollSquashSlideTypeBase(
    int style);
};

#endif
