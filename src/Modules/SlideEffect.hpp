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
 * @file   SlideEffect.hpp
 * @author Elliot Glaysher
 * @date   Thu Nov  2 20:34:27 2006
 * 
 * @brief  Implements #SEL transition style #10, Slide.
 */

#ifndef __SlideEffect_hpp__
#define __SlideEffect_hpp__

#include "Modules/Effect.hpp"

class GraphicsSystem;

/**
 * Base class for the four classess that implement #SEL transition
 * style #15, Slide. There are four direct subclasses from SlideEffect
 * that implement the individual directions that we wipe in.
 */
class SlideEffect : public Effect
{
private:
  virtual bool blitOriginalImage() const;

protected:
  int calculateAmountVisible(int currentTime, int screenSize);

public:
  SlideEffect(RLMachine& machine, int x, int y, int width, 
             int height, int dx, int dy, int time);
};

// -----------------------------------------------------------------------

/**
 * Implements SEL #15, Slide, with direction 0, top to bottom.
 */
class SlideTopToBottomEffect : public SlideEffect
{
protected:
  virtual void performEffectForTime(RLMachine& machine, int currentTime);

public:
  SlideTopToBottomEffect(RLMachine& machine, int x, int y, int width, 
                         int height, int dx, int dy, int time);
};

// -----------------------------------------------------------------------

/**
 * Implements SEL #15, Slide, with direction 1, bottom to top.
 */
class SlideBottomToTopEffect : public SlideEffect
{
protected:
  virtual void performEffectForTime(RLMachine& machine, int currentTime);

public:
  SlideBottomToTopEffect(RLMachine& machine, int x, int y, int width, 
                         int height, int dx, int dy, int time);
};

// -----------------------------------------------------------------------

/**
 * Implements SEL #15, Slide, with direction 2, left to right.
 */
class SlideLeftToRightEffect : public SlideEffect
{
protected:
  virtual void performEffectForTime(RLMachine& machine, int currentTime);

public:
  SlideLeftToRightEffect(RLMachine& machine, int x, int y, int width, 
                         int height, int dx, int dy, int time);
};

// -----------------------------------------------------------------------

/**
 * Implements SEL #15, Slide, with direction 3, right to left.
 */
class SlideRightToLeftEffect : public SlideEffect
{
protected:
  virtual void performEffectForTime(RLMachine& machine, int currentTime);

public:
  SlideRightToLeftEffect(RLMachine& machine, int x, int y, int width, 
                         int height, int dx, int dy, int time);
};



#endif
