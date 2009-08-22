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
 * @file   ScrollOnScrollOff.hpp
 * @author Elliot Glaysher
 * @date   Thu Nov  2 20:34:27 2006
 * @ingroup TransitionEffects
 * @brief  Implements a bunch of \#SEL transitions.
 */

#ifndef SRC_EFFECTS_SCROLLONSCROLLOFF_HPP_
#define SRC_EFFECTS_SCROLLONSCROLLOFF_HPP_

#include "Effects/Effect.hpp"

class GraphicsSystem;
class ScrollSquashSlideDrawer;
class ScrollSquashSlideEffectTypeBase;

/**
 * @ingroup TransitionEffects
 *
 * Base class for all the classess that implement variations on \#SEL
 * transition styles #15 (Scroll on, Scroll off), #16 (Scroll on,
 * Squash off), #17 (Squash on, Scroll off), #18 (Squash on, Squash
 * off), #20 (Slide on), #21 (Slide off).
 *
 * These effects are all very similar and are implemented by passing
 * two behaviour classes to an instance of
 * ScrollSquashSlideBaseEffect. The first behavioural class are the
 * subclassess of ScrollSquashSlideDrawer, which describe the
 * direction to draw in. The second is
 * ScrollSquashSlideEffectTypeBase, which defines what combination of
 * primitives to use.
 *
 * There are four drawer classes:
 * - TopToBottomDrawer
 * - BottomToTopDrawer
 * - LeftToRightDrawer
 * - RightToLeftDrawer
 *
 * and six effect type classes:
 * - ScrollOnScrollOff
 * - ScrollOnSquashOff
 * - SquashOnScrollOff
 * - SquashOnSquashOff
 * - SlideOn
 * - SlideOff
 */
class ScrollSquashSlideBaseEffect : public Effect
{
private:
  /// Drawer behavior class
  boost::scoped_ptr<ScrollSquashSlideDrawer> drawer_;

  /// Effect type behavior class
  boost::scoped_ptr<ScrollSquashSlideEffectTypeBase> effect_type_;

  /// Don't blit the original image.
  virtual bool blitOriginalImage() const;

  /// Calculates the amountVisible passed into composeEffectsFor().
  int calculateAmountVisible(int currentTime, int screenSize);

  /// Implement the Effect interface
  virtual void performEffectForTime(RLMachine& machine, int currentTime);

public:
  ScrollSquashSlideBaseEffect(RLMachine& machine,
                              boost::shared_ptr<Surface> src,
                              boost::shared_ptr<Surface> dst,
                              ScrollSquashSlideDrawer* drawer,
                              ScrollSquashSlideEffectTypeBase* effectType,
                              const Size& s, int time);

  virtual ~ScrollSquashSlideBaseEffect();
};

// -----------------------------------------------------------------------

/**
 * @name Drawer Behavior classes
 *
 * These classess implement drawing for directions; They are used by
 * child classes of ScrollOnScrollOff to perform the requested
 * operation in a certain direction.
 *
 * There are four, all representing the four directions used in these selections
 */
///@}

/**
 * Base interface which describes the (very) high level primatives
 * that are composed in the ScrollSquashSlideEffectTypeBase subclasses.
 *
 * @ingroup TransitionEffects
 */
class ScrollSquashSlideDrawer
{
public:
  ScrollSquashSlideDrawer();
  virtual ~ScrollSquashSlideDrawer();

  virtual int getMaxSize(GraphicsSystem& gs) = 0;
  virtual void scrollOn(GraphicsSystem&, ScrollSquashSlideBaseEffect&, int amountVisible, int width, int height) = 0;
  virtual void scrollOff(GraphicsSystem&, ScrollSquashSlideBaseEffect&, int amountVisible, int width, int height) = 0;
  virtual void squashOn(GraphicsSystem&, ScrollSquashSlideBaseEffect&, int amountVisible, int width, int height) = 0;
  virtual void squashOff(GraphicsSystem&, ScrollSquashSlideBaseEffect&, int amountVisible, int width, int height) = 0;
};

// -----------------------------------------------------------------------

/**
 * @ingroup TransitionEffects
 */
class TopToBottomDrawer : public ScrollSquashSlideDrawer
{
public:
  virtual int getMaxSize(GraphicsSystem& gs);
  virtual void scrollOn(GraphicsSystem&, ScrollSquashSlideBaseEffect&, int amountVisible, int width, int height);
  virtual void scrollOff(GraphicsSystem&, ScrollSquashSlideBaseEffect&, int amountVisible, int width, int height);
  virtual void squashOn(GraphicsSystem&, ScrollSquashSlideBaseEffect&, int amountVisible, int width, int height);
  virtual void squashOff(GraphicsSystem&, ScrollSquashSlideBaseEffect&, int amountVisible, int width, int height);
};

// -----------------------------------------------------------------------

/**
 * @ingroup TransitionEffects
 */
class BottomToTopDrawer : public ScrollSquashSlideDrawer
{
public:
  virtual int getMaxSize(GraphicsSystem& gs);
  virtual void scrollOn(GraphicsSystem&, ScrollSquashSlideBaseEffect&, int amountVisible, int width, int height);
  virtual void scrollOff(GraphicsSystem&, ScrollSquashSlideBaseEffect&, int amountVisible, int width, int height);
  virtual void squashOn(GraphicsSystem&, ScrollSquashSlideBaseEffect&, int amountVisible, int width, int height);
  virtual void squashOff(GraphicsSystem&, ScrollSquashSlideBaseEffect&, int amountVisible, int width, int height);
};

// -----------------------------------------------------------------------

/**
 * @ingroup TransitionEffects
 */
class LeftToRightDrawer : public ScrollSquashSlideDrawer
{
public:
  virtual int getMaxSize(GraphicsSystem& gs);
  virtual void scrollOn(GraphicsSystem&, ScrollSquashSlideBaseEffect&, int amountVisible, int width, int height);
  virtual void scrollOff(GraphicsSystem&, ScrollSquashSlideBaseEffect&, int amountVisible, int width, int height);
  virtual void squashOn(GraphicsSystem&, ScrollSquashSlideBaseEffect&, int amountVisible, int width, int height);
  virtual void squashOff(GraphicsSystem&, ScrollSquashSlideBaseEffect&, int amountVisible, int width, int height);
};

// -----------------------------------------------------------------------

/**
 * @ingroup TransitionEffects
 */
class RightToLeftDrawer : public ScrollSquashSlideDrawer
{
public:
  virtual int getMaxSize(GraphicsSystem& gs);
  virtual void scrollOn(GraphicsSystem&, ScrollSquashSlideBaseEffect&, int amountVisible, int width, int height);
  virtual void scrollOff(GraphicsSystem&, ScrollSquashSlideBaseEffect&, int amountVisible, int width, int height);
  virtual void squashOn(GraphicsSystem&, ScrollSquashSlideBaseEffect&, int amountVisible, int width, int height);
  virtual void squashOff(GraphicsSystem&, ScrollSquashSlideBaseEffect&, int amountVisible, int width, int height);
};
//@}

// -----------------------------------------------------------------------

/**
 * @name Effect Types
 *
 * Each EffectType that derives from ScrollSquashSlideEffectTypeBase
 * represents one of the SEL effects.
 */
///@{

/**
 * Base class that defines the interface
 *
 * @ingroup TransitionEffects
 */
class ScrollSquashSlideEffectTypeBase
{
public:
  virtual ~ScrollSquashSlideEffectTypeBase();
  virtual void composeEffectsFor(GraphicsSystem& system,
                                 ScrollSquashSlideBaseEffect& event,
                                 ScrollSquashSlideDrawer& drawer,
                                 int amountVisible) = 0;
};

// -----------------------------------------------------------------------

/**
 * Behavioural class that defines the high level behaviour for SEL
 * #15, Scroll On/Scroll Off.
 *
 * @ingroup TransitionEffects
 */
class ScrollOnScrollOff : public ScrollSquashSlideEffectTypeBase
{
public:
  virtual void composeEffectsFor(GraphicsSystem& system,
                                 ScrollSquashSlideBaseEffect& event,
                                 ScrollSquashSlideDrawer& drawer,
                                 int amountVisible);
};

// -----------------------------------------------------------------------

/**
 * Behavioural class that defines the high level behaviour for SEL
 * #16, Scroll On/Squash Off.
 *
 * @ingroup TransitionEffects
 */
class ScrollOnSquashOff : public ScrollSquashSlideEffectTypeBase
{
public:
  virtual void composeEffectsFor(GraphicsSystem& system,
                                 ScrollSquashSlideBaseEffect& event,
                                 ScrollSquashSlideDrawer& drawer,
                                 int amountVisible);
};

// -----------------------------------------------------------------------

/**
 * Behavioural class that defines the high level behaviour for SEL
 * #17, Squash On/Scroll Off.
 *
 * @ingroup TransitionEffects
 */
class SquashOnScrollOff : public ScrollSquashSlideEffectTypeBase
{
public:
  virtual void composeEffectsFor(GraphicsSystem& system,
                                 ScrollSquashSlideBaseEffect& event,
                                 ScrollSquashSlideDrawer& drawer,
                                 int amountVisible);
};

// -----------------------------------------------------------------------


/**
 * Behavioural class that defines the high level behaviour for SEL
 * #18, Squash On/Squash Off.
 *
 * @ingroup TransitionEffects
 */
class SquashOnSquashOff : public ScrollSquashSlideEffectTypeBase
{
public:
  virtual void composeEffectsFor(GraphicsSystem& system,
                                 ScrollSquashSlideBaseEffect& event,
                                 ScrollSquashSlideDrawer& drawer,
                                 int amountVisible);
};

// -----------------------------------------------------------------------

/**
 * Behavioural class that defines the high level behaviour for SEL
 * #20, Slide On.
 *
 */
class SlideOn : public ScrollSquashSlideEffectTypeBase
{
public:
  virtual void composeEffectsFor(GraphicsSystem& system,
                                 ScrollSquashSlideBaseEffect& event,
                                 ScrollSquashSlideDrawer& drawer,
                                 int amountVisible);
};

// -----------------------------------------------------------------------

/**
 * Behavioural class that defines the high level behaviour for SEL
 * #21, Slide Off.
 *
 * @ingroup TransitionEffects
 */
class SlideOff : public ScrollSquashSlideEffectTypeBase
{
public:
  virtual void composeEffectsFor(GraphicsSystem& system,
                                 ScrollSquashSlideBaseEffect& event,
                                 ScrollSquashSlideDrawer& drawer,
                                 int amountVisible);
};

///@}

#endif  // SRC_EFFECTS_SCROLLONSCROLLOFF_HPP_
