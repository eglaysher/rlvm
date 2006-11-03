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
 * @file   WipeEffect.hpp
 * @author Elliot Glaysher
 * @date   Thu Nov  2 20:34:27 2006
 * 
 * @brief  Implements #SEL transition style #10, Wipe.
 */

#ifndef __WipeEffect_hpp__
#define __WipeEffect_hpp__

#include "Modules/Effect.hpp"

class GraphicsSystem;

/**
 * Implements #SEL transition style #10, Wipe.
 * 
 */
class WipeEffect : public Effect
{
private:
  /// Which direction we wipe in 
  enum Direction {
    WIPE_TOP_TO_BOTTOM = 0, ///< Wipe from the top to the bottom
    WIPE_BOTTOM_TO_TOP = 1, ///< Wipe from the bottom to the top
    WIPE_LEFT_TO_RIGHT = 2, ///< Wipe from left to right
    WIPE_RIGHT_TO_LEFT = 3  ///< Wipe from right to left
  } m_direction;
  int m_interpolation;
  int m_interpolationInPixels;

  virtual bool blitOriginalImage() const;

  void calculateSizes(int currentTime,
                      int& sizeOfInterpolation, 
                      int& sizeOfMainPolygon,
                      int sizeOfScreen);

  void wipeFromTopToBottom(GraphicsSystem& system, int currentTime);
  void wipeFromBottomToTop(GraphicsSystem& system, int currentTime);
  void wipeFromLeftToRight(GraphicsSystem& system, int currentTime);
  void wipeFromRightToLeft(GraphicsSystem& system, int currentTime);

protected:
  virtual void performEffectForTime(RLMachine& machine,
                                    int currentTime);

public:
  WipeEffect(RLMachine& machine, int x, int y, int width, 
             int height, int dx, int dy, int time, int direction,
             int interpolation);
};

#endif
