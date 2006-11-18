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
 * @file   ScrollOnScrollOff.hpp
 * @author Elliot Glaysher
 * @date   Thu Nov  2 20:34:27 2006
 * 
 * @brief  Implements #SEL transition style #10, ScrollOnScrollOff.
 */

#ifndef __ScrollOnScrollOff_hpp__
#define __ScrollOnScrollOff_hpp__

#include "Modules/Effect.hpp"

class GraphicsSystem;

/**
 * @ingroup TransitionEffects
 * 
 * @{
 */

/**
 * Base class for the four classess that implement #SEL transition
 * style #15, ScrollOnScrollOff. There are four direct subclasses from ScrollOnScrollOff
 * that implement the individual directions that we wipe in.
 */
class ScrollOnScrollOff : public Effect
{
private:
  virtual bool blitOriginalImage() const;

protected:
  int calculateAmountVisible(int currentTime, int screenSize);

public:
  ScrollOnScrollOff(RLMachine& machine, int width, int height, int time);
};

// -----------------------------------------------------------------------

/**
 * Implements SEL #15, ScrollOnScrollOff, with direction 0, top to bottom.
 */
class ScrollOnScrollOffTopToBottomEffect : public ScrollOnScrollOff
{
protected:
  virtual void performEffectForTime(RLMachine& machine, int currentTime);

public:
  ScrollOnScrollOffTopToBottomEffect(RLMachine& machine, int width, 
                         int height, int time);
};

// -----------------------------------------------------------------------

/**
 * Implements SEL #15, ScrollOnScrollOff, with direction 1, bottom to top.
 */
class ScrollOnScrollOffBottomToTopEffect : public ScrollOnScrollOff
{
protected:
  virtual void performEffectForTime(RLMachine& machine, int currentTime);

public:
  ScrollOnScrollOffBottomToTopEffect(RLMachine& machine, int width, 
                         int height, int time);
};

// -----------------------------------------------------------------------

/**
 * Implements SEL #15, ScrollOnScrollOff, with direction 2, left to right.
 */
class ScrollOnScrollOffLeftToRightEffect : public ScrollOnScrollOff
{
protected:
  virtual void performEffectForTime(RLMachine& machine, int currentTime);

public:
  ScrollOnScrollOffLeftToRightEffect(RLMachine& machine, int width, 
                         int height, int time);
};

// -----------------------------------------------------------------------

/**
 * Implements SEL #15, ScrollOnScrollOff, with direction 3, right to left.
 */
class ScrollOnScrollOffRightToLeftEffect : public ScrollOnScrollOff
{
protected:
  virtual void performEffectForTime(RLMachine& machine, int currentTime);

public:
  ScrollOnScrollOffRightToLeftEffect(RLMachine& machine, int width, 
                         int height, int time);
};

// @}

#endif
