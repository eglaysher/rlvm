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
 * @file   BlindEffect.hpp
 * @author Elliot Glaysher
 * @date   Tue Feb 27 21:04:47 2007
 *
 * @ingroup TransitionEffects
 * @brief  Implements \#SEL transition style \#10, Blind.
 */

#ifndef __BlindEffect_hpp__
#define __BlindEffect_hpp__

#include "Effects/Effect.hpp"

/**
 * Base class for implementing \#SEL transition style \#10, Blind.
 * 
 * @ingroup TransitionEffects
 */
class BlindEffect : public Effect
{
private:
  int m_blindSize;

  virtual bool blitOriginalImage() const;

protected:
  // We'll probably need some utility stuff here.

  const int blindSize() const { return m_blindSize; }

  virtual void computeGrowing(
    RLMachine& machine, int maxSize, int currentTime);
  virtual void computeDecreasing(
    RLMachine& machine, int maxSize, int currentTime);

  virtual void renderPolygon(int polyStart, int polyEnd) = 0;

public:
  BlindEffect(RLMachine& machine, boost::shared_ptr<Surface> src,
             boost::shared_ptr<Surface> dst, 
             int width, int height, int time, int blindSize);

  ~BlindEffect();
};

// -----------------------------------------------------------------------

/**
 * @ingroup TransitionEffects
 */
class BlindTopToBottomEffect : public BlindEffect
{
protected:
  virtual void performEffectForTime(RLMachine& machine, int currentTime);
  virtual void renderPolygon(int polyStart, int polyEnd);

public:
  BlindTopToBottomEffect(RLMachine& machine, boost::shared_ptr<Surface> src,
             boost::shared_ptr<Surface> dst, 
             int width, int height, int time, int blindSize);
};

// -----------------------------------------------------------------------

/**
 * @ingroup TransitionEffects
 */
class BlindBottomToTopEffect : public BlindEffect
{
protected:
  virtual void performEffectForTime(RLMachine& machine, int currentTime);
  virtual void renderPolygon(int polyStart, int polyEnd);

public:
  BlindBottomToTopEffect(RLMachine& machine, boost::shared_ptr<Surface> src,
             boost::shared_ptr<Surface> dst, 
             int width, int height, int time, int blindSize);
};

// -----------------------------------------------------------------------

/**
 * @ingroup TransitionEffects
 */
class BlindLeftToRightEffect : public BlindEffect
{
protected:
  virtual void performEffectForTime(RLMachine& machine, int currentTime);
  virtual void renderPolygon(int polyStart, int polyEnd);

public:
  BlindLeftToRightEffect(RLMachine& machine, boost::shared_ptr<Surface> src,
             boost::shared_ptr<Surface> dst, 
             int width, int height, int time, int blindSize);
};

// -----------------------------------------------------------------------

/**
 * @ingroup TransitionEffects
 */
class BlindRightToLeftEffect : public BlindEffect
{
protected:
  virtual void performEffectForTime(RLMachine& machine, int currentTime);
  virtual void renderPolygon(int polyStart, int polyEnd);

public:
  BlindRightToLeftEffect(RLMachine& machine, boost::shared_ptr<Surface> src,
             boost::shared_ptr<Surface> dst, 
             int width, int height, int time, int blindSize);
};

#endif
