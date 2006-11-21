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
 * @file   ScrollOnScrollOff.cpp
 * @author Elliot Glaysher
 * @date   Thu Nov  2 20:33:21 2006
 * @ingroup TransitionEffects
 * @brief  Implements a bunch of \#SEL transitions.
 */

#include "ScrollOnScrollOff.hpp"
#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/GraphicsSystem.hpp"

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
  m_effectType->composeEffectsFor(graphics, *m_drawer, amountVisible);
}

// -----------------------------------------------------------------------

ScrollSquashSlideBaseEffect::ScrollSquashSlideBaseEffect(
  RLMachine& machine,
  ScrollSquashSlideDrawer* drawer,
  ScrollSquashSlideEffectTypeBase* effectType,
  int width, int height, int time)
  : Effect(machine, width, height, time), 
    m_drawer(drawer), m_effectType(effectType)
{
}

// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
// Direction Interface
// -----------------------------------------------------------------------

// ------------------------------------------------- [ TopToBottomDrawer ]

int TopToBottomDrawer::getMaxSize(GraphicsSystem& gs)
{
  return gs.screenHeight();
}

// -----------------------------------------------------------------------

void TopToBottomDrawer::scrollOff(GraphicsSystem& graphics,
                                  int amountVisible, int width, int height)
{
   graphics.getDC(0).
     renderToScreen(0, 0, width, height - amountVisible,
                    0, amountVisible, width, height, 255);
}

// -----------------------------------------------------------------------

void TopToBottomDrawer::scrollOn(GraphicsSystem& graphics,
                                 int amountVisible, int width, int height)
{
  graphics.getDC(1).
    renderToScreen(0, height - amountVisible, width, height,
                   0, 0, width, amountVisible, 255);
}

// -----------------------------------------------------------------------

void TopToBottomDrawer::squashOff(GraphicsSystem& graphics,
                                  int amountVisible, int width, int height)
{
  graphics.getDC(0).
    renderToScreen(0, 0, width, height,
                   0, amountVisible, width, height, 255);
}

// -----------------------------------------------------------------------

void TopToBottomDrawer::squashOn(GraphicsSystem& graphics,
                                 int amountVisible, int width, int height)
{
  graphics.getDC(1).
    renderToScreen(0, 0, width, height,
                   0, 0, width, amountVisible, 255);
}

// ------------------------------------------------- [ BottomToTopDrawer ]

int BottomToTopDrawer::getMaxSize(GraphicsSystem& gs)
{
  return gs.screenHeight();
}

// -----------------------------------------------------------------------

void BottomToTopDrawer::scrollOn(GraphicsSystem& graphics,
                                 int amountVisible, int width, int height)
{
   graphics.getDC(1).
     renderToScreen(0, 0, width, amountVisible,
                    0, height - amountVisible, width, height, 255);
}

// -----------------------------------------------------------------------

void BottomToTopDrawer::scrollOff(GraphicsSystem& graphics, 
                                  int amountVisible, int width, int height)
{
   graphics.getDC(0).
     renderToScreen(0, amountVisible, width, height,
                    0, 0, width, height - amountVisible, 255);
}

// -----------------------------------------------------------------------

void BottomToTopDrawer::squashOn(GraphicsSystem& graphics,
                                 int amountVisible, int width, int height)
{
   graphics.getDC(1).
     renderToScreen(0, 0, width, height,
                    0, height - amountVisible, width, height, 255);
}

// -----------------------------------------------------------------------

void BottomToTopDrawer::squashOff(GraphicsSystem& graphics, 
                                  int amountVisible, int width, int height)
{
  graphics.getDC(0).
    renderToScreen(0, 0, width, height,
                   0, 0, width, height - amountVisible, 255);
}

// ------------------------------------------------- [ LeftToRightDrawer ]

int LeftToRightDrawer::getMaxSize(GraphicsSystem& gs)
{
  return gs.screenWidth();
}

// -----------------------------------------------------------------------

void LeftToRightDrawer::scrollOn(GraphicsSystem& graphics, 
                                 int amountVisible, int width, int height)
{
  graphics.getDC(1).
    renderToScreen(width - amountVisible, 0, width, height,
                   0, 0, amountVisible, height, 255);
}

// -----------------------------------------------------------------------

void LeftToRightDrawer::scrollOff(GraphicsSystem& graphics, 
                                  int amountVisible, int width, int height)
{
  graphics.getDC(0).
    renderToScreen(0, 0, width - amountVisible, height,
                   amountVisible, 0, width, height, 255);
}

// -----------------------------------------------------------------------

void LeftToRightDrawer::squashOn(GraphicsSystem& graphics, 
                                 int amountVisible, int width, int height)
{
  graphics.getDC(1).
    renderToScreen(0, 0, width, height, 
                   0, 0, amountVisible, height, 255);
}

// -----------------------------------------------------------------------

void LeftToRightDrawer::squashOff(GraphicsSystem& graphics, 
                                  int amountVisible, int width, int height)
{
  graphics.getDC(0).
    renderToScreen(0, 0, width, height,
                   amountVisible, 0, width, height, 255);
}

// ------------------------------------------------- [ RightToLeftDrawer ]

int RightToLeftDrawer::getMaxSize(GraphicsSystem& gs)
{
  return gs.screenWidth();
}

// -----------------------------------------------------------------------

void RightToLeftDrawer::scrollOff(GraphicsSystem& graphics,
                                  int amountVisible, int width, int height)
{
  graphics.getDC(0).
    renderToScreen(amountVisible, 0, width, height,
                   0, 0, width - amountVisible, height, 255);
}

// -----------------------------------------------------------------------

void RightToLeftDrawer::scrollOn(GraphicsSystem& graphics, 
                                 int amountVisible, int width, int height)
{
  graphics.getDC(1).
    renderToScreen(0, 0, amountVisible, height,
                   width - amountVisible, 0, width, height, 255);
}

// -----------------------------------------------------------------------

void RightToLeftDrawer::squashOff(GraphicsSystem& graphics,
                                  int amountVisible, int width, int height)
{
  graphics.getDC(0).
    renderToScreen(0, 0, width, height,
                   0, 0, width - amountVisible, height, 255);
}

// -----------------------------------------------------------------------

void RightToLeftDrawer::squashOn(GraphicsSystem& graphics, 
                                 int amountVisible, int width, int height)
{
  graphics.getDC(1).
    renderToScreen(0, 0, width, height,
                   width - amountVisible, 0, width, height, 255);  
}

// -----------------------------------------------------------------------
// Effect Type Interface
// -----------------------------------------------------------------------

void ScrollOnScrollOff::composeEffectsFor(
  GraphicsSystem& system, ScrollSquashSlideDrawer& drawer,
  int amountVisible)
{
  drawer.scrollOn(system, amountVisible, system.screenWidth(), system.screenHeight());
  drawer.scrollOff(system, amountVisible, system.screenWidth(), system.screenHeight());
}

// -----------------------------------------------------------------------

void ScrollOnSquashOff::composeEffectsFor(
  GraphicsSystem& system, ScrollSquashSlideDrawer& drawer,
  int amountVisible)
{
  drawer.scrollOn(system, amountVisible, system.screenWidth(), system.screenHeight());
  drawer.squashOff(system, amountVisible, system.screenWidth(), system.screenHeight());
}

// -----------------------------------------------------------------------

void SquashOnScrollOff::composeEffectsFor(
  GraphicsSystem& system, ScrollSquashSlideDrawer& drawer,
  int amountVisible)
{
  drawer.squashOn(system, amountVisible, system.screenWidth(), system.screenHeight());
  drawer.scrollOff(system, amountVisible, system.screenWidth(), system.screenHeight());
}

// -----------------------------------------------------------------------

void SquashOnSquashOff::composeEffectsFor(
  GraphicsSystem& system, ScrollSquashSlideDrawer& drawer,
  int amountVisible)
{
  drawer.squashOn(system, amountVisible, system.screenWidth(), system.screenHeight());
  drawer.squashOff(system, amountVisible, system.screenWidth(), system.screenHeight());
}

// -----------------------------------------------------------------------

void SlideOn::composeEffectsFor(
  GraphicsSystem& system, ScrollSquashSlideDrawer& drawer,
  int amountVisible)
{
  int width = system.screenWidth();
  int height = system.screenHeight();

  // Draw the old image
  system.getDC(0).
    renderToScreen(0, 0, width, height, 0, 0, width, height, 255);

  drawer.scrollOn(system, amountVisible, width, height);
}

// -----------------------------------------------------------------------

void SlideOff::composeEffectsFor(
  GraphicsSystem& system, ScrollSquashSlideDrawer& drawer,
  int amountVisible)
{
  int width = system.screenWidth();
  int height = system.screenHeight();

  // Draw the old image
  system.getDC(1).
    renderToScreen(0, 0, width, height, 0, 0, width, height, 255);

  drawer.scrollOff(system, amountVisible, width, height);
}

// -----------------------------------------------------------------------
