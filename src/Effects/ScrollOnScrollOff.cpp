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

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

/**
 * @file   ScrollOnScrollOff.cpp
 * @author Elliot Glaysher
 * @date   Thu Nov  2 20:33:21 2006
 * @ingroup TransitionEffects
 * @brief  Implements a bunch of \#SEL transitions.
 */

#include "Effects/ScrollOnScrollOff.hpp"
#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/Surface.hpp"

#include <iostream>
#include <cmath>

using namespace std;

// -----------------------------------------------------------------------
// ScrollOnScrollOff base class
// -----------------------------------------------------------------------

bool ScrollSquashSlideBaseEffect::blitOriginalImage() const
{
  return false; 
}

// -----------------------------------------------------------------------

int ScrollSquashSlideBaseEffect::calculateAmountVisible(int currentTime, int screenSize)
{
  return int((float(currentTime) / duration()) * screenSize);
}

// -----------------------------------------------------------------------

void ScrollSquashSlideBaseEffect::performEffectForTime(
  RLMachine& machine, int currentTime)
{
  GraphicsSystem& graphics = machine.system().graphics();
  int amountVisible = calculateAmountVisible(currentTime,  
                                             m_drawer->getMaxSize(graphics));
  m_effectType->composeEffectsFor(graphics, *this, *m_drawer, amountVisible);
}

// -----------------------------------------------------------------------

ScrollSquashSlideBaseEffect::ScrollSquashSlideBaseEffect(
  RLMachine& machine,
  boost::shared_ptr<Surface> src,
  boost::shared_ptr<Surface> dst,
  ScrollSquashSlideDrawer* drawer,
  ScrollSquashSlideEffectTypeBase* effectType,
  const Size& s, int time)
  : Effect(machine, src, dst, s, time), 
    m_drawer(drawer), m_effectType(effectType)
{
}

// -----------------------------------------------------------------------

ScrollSquashSlideBaseEffect::~ScrollSquashSlideBaseEffect()
{}

// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
// Direction Interface
// -----------------------------------------------------------------------

// ------------------------------------------- [ ScrollSquashSlideDrawer ]

ScrollSquashSlideDrawer::ScrollSquashSlideDrawer()
{}

// -----------------------------------------------------------------------

ScrollSquashSlideDrawer::~ScrollSquashSlideDrawer()
{}

// ------------------------------------------------- [ TopToBottomDrawer ]

int TopToBottomDrawer::getMaxSize(GraphicsSystem& gs)
{
  return gs.screenSize().height();
}

// -----------------------------------------------------------------------

void TopToBottomDrawer::scrollOff(GraphicsSystem& graphics,
                                  ScrollSquashSlideBaseEffect& effect,
                                  int amountVisible, int width, int height)
{
   effect.dstSurface().
     renderToScreen(Rect(0, 0, width, height - amountVisible),
                    Rect(0, amountVisible, width, height), 255);
}

// -----------------------------------------------------------------------

void TopToBottomDrawer::scrollOn(GraphicsSystem& graphics,
                                 ScrollSquashSlideBaseEffect& effect,
                                 int amountVisible, int width, int height)
{
  effect.srcSurface().
    renderToScreen(Rect(0, height - amountVisible, width, height),
                   Rect(0, 0, width, amountVisible), 255);
}

// -----------------------------------------------------------------------

void TopToBottomDrawer::squashOff(GraphicsSystem& graphics,
                                  ScrollSquashSlideBaseEffect& effect,
                                  int amountVisible, int width, int height)
{
  effect.dstSurface().
    renderToScreen(Rect(0, 0, width, height),
                   Rect(0, amountVisible, width, height), 255);
}

// -----------------------------------------------------------------------

void TopToBottomDrawer::squashOn(GraphicsSystem& graphics,
                                 ScrollSquashSlideBaseEffect& effect,
                                 int amountVisible, int width, int height)
{
  effect.srcSurface().
    renderToScreen(Rect(0, 0, width, height),
                   Rect(0, 0, width, amountVisible), 255);
}

// ------------------------------------------------- [ BottomToTopDrawer ]

int BottomToTopDrawer::getMaxSize(GraphicsSystem& gs)
{
  return gs.screenSize().height();
}

// -----------------------------------------------------------------------

void BottomToTopDrawer::scrollOn(GraphicsSystem& graphics,
                                 ScrollSquashSlideBaseEffect& effect,
                                 int amountVisible, int width, int height)
{
   effect.srcSurface().
     renderToScreen(Rect(0, 0, width, amountVisible),
                    Rect(0, height - amountVisible, width, height), 255);
}

// -----------------------------------------------------------------------

void BottomToTopDrawer::scrollOff(GraphicsSystem& graphics, 
                                  ScrollSquashSlideBaseEffect& effect, 
                                  int amountVisible, int width, int height)
{
   effect.dstSurface().
     renderToScreen(Rect(0, amountVisible, width, height),
                    Rect(0, 0, width, height - amountVisible), 255);
}

// -----------------------------------------------------------------------

void BottomToTopDrawer::squashOn(GraphicsSystem& graphics,
                                 ScrollSquashSlideBaseEffect& effect,
                                 int amountVisible, int width, int height)
{
   effect.srcSurface().
     renderToScreen(Rect(0, 0, width, height),
                    Rect(0, height - amountVisible, width, height), 255);
}

// -----------------------------------------------------------------------

void BottomToTopDrawer::squashOff(GraphicsSystem& graphics,
                                  ScrollSquashSlideBaseEffect& effect, 
                                  int amountVisible, int width, int height)
{
  effect.dstSurface().
    renderToScreen(Rect(0, 0, width, height),
                   Rect(0, 0, width, height - amountVisible), 255);
}

// ------------------------------------------------- [ LeftToRightDrawer ]

int LeftToRightDrawer::getMaxSize(GraphicsSystem& gs)
{
  return gs.screenSize().width();
}

// -----------------------------------------------------------------------

void LeftToRightDrawer::scrollOn(GraphicsSystem& graphics,
                                 ScrollSquashSlideBaseEffect& effect, 
                                 int amountVisible, int width, int height)
{
  effect.srcSurface().
    renderToScreen(Rect(width - amountVisible, 0, width, height),
                   Rect(0, 0, amountVisible, height), 255);
}

// -----------------------------------------------------------------------

void LeftToRightDrawer::scrollOff(GraphicsSystem& graphics,
                                  ScrollSquashSlideBaseEffect& effect, 
                                  int amountVisible, int width, int height)
{
  effect.dstSurface().
    renderToScreen(Rect(0, 0, width - amountVisible, height),
                   Rect(amountVisible, 0, width, height), 255);
}

// -----------------------------------------------------------------------

void LeftToRightDrawer::squashOn(GraphicsSystem& graphics,
                                 ScrollSquashSlideBaseEffect& effect, 
                                 int amountVisible, int width, int height)
{
  effect.srcSurface().
    renderToScreen(Rect(0, 0, width, height), 
                   Rect(0, 0, amountVisible, height), 255);
}

// -----------------------------------------------------------------------

void LeftToRightDrawer::squashOff(GraphicsSystem& graphics,
                                  ScrollSquashSlideBaseEffect& effect, 
                                  int amountVisible, int width, int height)
{
  effect.dstSurface().
    renderToScreen(Rect(0, 0, width, height),
                   Rect(amountVisible, 0, width, height), 255);
}

// ------------------------------------------------- [ RightToLeftDrawer ]

int RightToLeftDrawer::getMaxSize(GraphicsSystem& gs)
{
  return gs.screenSize().width();
}

// -----------------------------------------------------------------------

void RightToLeftDrawer::scrollOff(GraphicsSystem& graphics,
                                  ScrollSquashSlideBaseEffect& effect,
                                  int amountVisible, int width, int height)
{
  effect.dstSurface().
    renderToScreen(Rect(amountVisible, 0, width, height),
                   Rect(0, 0, width - amountVisible, height), 255);
}

// -----------------------------------------------------------------------

void RightToLeftDrawer::scrollOn(GraphicsSystem& graphics,
                                 ScrollSquashSlideBaseEffect& effect, 
                                 int amountVisible, int width, int height)
{
  effect.srcSurface().
    renderToScreen(Rect(0, 0, amountVisible, height),
                   Rect(width - amountVisible, 0, width, height), 255);
}

// -----------------------------------------------------------------------

void RightToLeftDrawer::squashOff(GraphicsSystem& graphics,
                                  ScrollSquashSlideBaseEffect& effect,
                                  int amountVisible, int width, int height)
{
  effect.dstSurface().
    renderToScreen(Rect(0, 0, width, height),
                   Rect(0, 0, width - amountVisible, height), 255);
}

// -----------------------------------------------------------------------

void RightToLeftDrawer::squashOn(GraphicsSystem& graphics,
                                 ScrollSquashSlideBaseEffect& effect, 
                                 int amountVisible, int width, int height)
{
  effect.srcSurface().
    renderToScreen(Rect(0, 0, width, height),
                   Rect(width - amountVisible, 0, width, height), 255);  
}

// -----------------------------------------------------------------------
// Effect Type Interface
// -----------------------------------------------------------------------

ScrollSquashSlideEffectTypeBase::~ScrollSquashSlideEffectTypeBase()
{}

// -----------------------------------------------------------------------

void ScrollOnScrollOff::composeEffectsFor(
  GraphicsSystem& system,                                
  ScrollSquashSlideBaseEffect& effect, 
  ScrollSquashSlideDrawer& drawer,
  int amountVisible)
{
  Size s = system.screenSize();
  drawer.scrollOn(system, effect, amountVisible, s.width(), s.height());
  drawer.scrollOff(system, effect, amountVisible, s.width(), s.height());
}

// -----------------------------------------------------------------------

void ScrollOnSquashOff::composeEffectsFor(
  GraphicsSystem& system, 
  ScrollSquashSlideBaseEffect& effect, 
  ScrollSquashSlideDrawer& drawer,
  int amountVisible)
{
  Size s = system.screenSize();
  drawer.scrollOn(system, effect, amountVisible, s.width(), s.height());
  drawer.squashOff(system, effect, amountVisible, s.width(), s.height());
}

// -----------------------------------------------------------------------

void SquashOnScrollOff::composeEffectsFor(
  GraphicsSystem& system,
  ScrollSquashSlideBaseEffect& effect, 
  ScrollSquashSlideDrawer& drawer,
  int amountVisible)
{
  Size s = system.screenSize();
  drawer.squashOn(system, effect, amountVisible, s.width(), s.height());
  drawer.scrollOff(system, effect, amountVisible, s.width(), s.height());
}

// -----------------------------------------------------------------------

void SquashOnSquashOff::composeEffectsFor(
  GraphicsSystem& system, 
  ScrollSquashSlideBaseEffect& effect, 
  ScrollSquashSlideDrawer& drawer,
  int amountVisible)
{
  Size s = system.screenSize();
  drawer.squashOn(system, effect, amountVisible, s.width(), s.height());
  drawer.squashOff(system, effect, amountVisible, s.width(), s.height());
}

// -----------------------------------------------------------------------

void SlideOn::composeEffectsFor(
  GraphicsSystem& system,
  ScrollSquashSlideBaseEffect& effect, 
  ScrollSquashSlideDrawer& drawer,
  int amountVisible)
{
  Size s = system.screenSize();
  Rect screenRect = system.screenRect();

  // Draw the old image
  effect.dstSurface().
    renderToScreen(screenRect, screenRect, 255);

  drawer.scrollOn(system, effect, amountVisible, s.width(), s.height());
}

// -----------------------------------------------------------------------

void SlideOff::composeEffectsFor(
  GraphicsSystem& system,
  ScrollSquashSlideBaseEffect& effect, 
  ScrollSquashSlideDrawer& drawer,
  int amountVisible)
{
  Size s = system.screenSize();
  Rect screenRect = system.screenRect();

  effect.srcSurface().
    renderToScreen(screenRect, screenRect, 255);

  drawer.scrollOff(system, effect, amountVisible, s.width(), s.height());
}

// -----------------------------------------------------------------------
