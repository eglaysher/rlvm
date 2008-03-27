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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//  
// -----------------------------------------------------------------------

/**
 * @file   WipeEffect.hpp
 * @author Elliot Glaysher
 * @date   Thu Nov  2 20:34:27 2006
 * 
 * @ingroup TransitionEffects
 * @brief  Implements \#SEL transition style \#10, Wipe.
 */

#ifndef __WipeEffect_hpp__
#define __WipeEffect_hpp__

#include "Effects/Effect.hpp"

#include <boost/shared_ptr.hpp>

class GraphicsSystem;

/**
 * Base class for the four classess that implement \#SEL transition
 * style #10, Wipe. There are four direct subclasses from WipeEffect
 * that implement the individual directions that we wipe in.
 *
 * @ingroup TransitionEffects
 */
class WipeEffect : public Effect
{
private:
  int m_interpolation;
  int m_interpolationInPixels;

  virtual bool blitOriginalImage() const;

protected:
  void calculateSizes(int currentTime,
                      int& sizeOfInterpolation, 
                      int& sizeOfMainPolygon,
                      int sizeOfScreen);

public:
  WipeEffect(RLMachine& machine, boost::shared_ptr<Surface> src,
             boost::shared_ptr<Surface> dst, 
             int width, int height, int time, int interpolation);
};

// -----------------------------------------------------------------------

/**
 * Implements SEL #10, Wipe, with direction 0, top to bottom.
 *
 * @ingroup TransitionEffects
 */
class WipeTopToBottomEffect : public WipeEffect
{
protected:
  virtual void performEffectForTime(RLMachine& machine, int currentTime);

public:
  WipeTopToBottomEffect(RLMachine& machine, boost::shared_ptr<Surface> src,
                        boost::shared_ptr<Surface> dst, 
                        int width, int height, int time,
                        int interpolation);
};

// -----------------------------------------------------------------------

/**
 * Implements SEL #10, Wipe, with direction 1, bottom to top.
 *
 * @ingroup TransitionEffects
 */
class WipeBottomToTopEffect : public WipeEffect
{
protected:
  virtual void performEffectForTime(RLMachine& machine, int currentTime);

public:
  WipeBottomToTopEffect(RLMachine& machine, boost::shared_ptr<Surface> src,
                        boost::shared_ptr<Surface> dst, 
                        int width, 
                        int height, int time,
                        int interpolation);
};

// -----------------------------------------------------------------------

/**
 * Implements SEL #10, Wipe, with direction 2, left to right.
 *
 * @ingroup TransitionEffects
 */
class WipeLeftToRightEffect : public WipeEffect
{
protected:
  virtual void performEffectForTime(RLMachine& machine, int currentTime);

public:
  WipeLeftToRightEffect(RLMachine& machine, boost::shared_ptr<Surface> src,
                        boost::shared_ptr<Surface> dst, 
                        int width, int height, int time,
                        int interpolation);
};

// -----------------------------------------------------------------------

/**
 * Implements SEL #10, Wipe, with direction 2, left to right.
 *
 * @ingroup TransitionEffects
 */
class WipeRightToLeftEffect : public WipeEffect
{
protected:
  virtual void performEffectForTime(RLMachine& machine, int currentTime);

public:
  WipeRightToLeftEffect(RLMachine& machine, boost::shared_ptr<Surface> src,
                        boost::shared_ptr<Surface> dst, 
                        int width, int height, int time,
                        int interpolation);
};

#endif
