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
 * @file   SlideEffect.cpp
 * @author Elliot Glaysher
 * @date   Thu Nov  2 20:33:21 2006
 * 
 * @brief  Implements #SEL transition style #15, Slide.
 */

#include "SlideEffect.hpp"
#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/GraphicsSystem.hpp"

#include <iostream>
#include <cmath>

using namespace std;

// -----------------------------------------------------------------------
// SlideEffect base class
// -----------------------------------------------------------------------

bool SlideEffect::blitOriginalImage() const
{
  return false; 
}

// -----------------------------------------------------------------------

int SlideEffect::calculateAmountVisible(int currentTime, int screenSize)
{
  return int((float(currentTime) / duration()) * screenSize);
}

// -----------------------------------------------------------------------

SlideEffect::SlideEffect(RLMachine& machine, int x, int y, int width, 
                         int height, int dx, int dy, int time)
  : Effect(machine, x, y, width, height, dx, dy, time)
{
}

// -----------------------------------------------------------------------
// SlideTopToBottomEffect
// -----------------------------------------------------------------------

SlideTopToBottomEffect::SlideTopToBottomEffect(
  RLMachine& machine, int x, int y, int width, int height, int dx, 
  int dy, int time)
  : SlideEffect(machine, x, y, width, height, dx, dy, time)
{}

// -----------------------------------------------------------------------

void SlideTopToBottomEffect::performEffectForTime(RLMachine& machine, 
                                                 int currentTime)
{
  GraphicsSystem& graphics = machine.system().graphics();
  int amountVisible = calculateAmountVisible(currentTime, height());
  
   graphics.getDC(0).
     renderToScreen(x(), y(), 
                    x() + width(), y() + height() - amountVisible,
                    dx(), dy() + amountVisible, 
                    dx() + width(), dy() + height(),
                    255);
  graphics.getDC(1).
    renderToScreen(x(), y() + height() - amountVisible,
                   x() + width(), y() + height(),
                   dx(), dy(), 
                   dx() + width(), dy() + amountVisible,
                   255);
}

// -----------------------------------------------------------------------
// SlideBottomToTopEffect
// -----------------------------------------------------------------------

SlideBottomToTopEffect::SlideBottomToTopEffect(
  RLMachine& machine, int x, int y, int width, int height, int dx, 
  int dy, int time)
  : SlideEffect(machine, x, y, width, height, dx, dy, time)
{}

// -----------------------------------------------------------------------

void SlideBottomToTopEffect::performEffectForTime(RLMachine& machine,
                                                 int currentTime)
{
  GraphicsSystem& graphics = machine.system().graphics();
  int amountVisible = calculateAmountVisible(currentTime, height());
  
   graphics.getDC(0).
     renderToScreen(x(), y() + amountVisible, 
                    x() + width(), y() + height(),
                    dx(), dy(),
                    dx() + width(), dy() + height() - amountVisible,
                    255);
   graphics.getDC(1).
     renderToScreen(x(), y(),
                    x() + width(), y() + amountVisible,
                    dx(), dy() + height() - amountVisible, 
                    dx() + width(), dy() + height(),
                    255);
}

// -----------------------------------------------------------------------
// SlideFromLeftToRightEffect
// -----------------------------------------------------------------------

SlideLeftToRightEffect::SlideLeftToRightEffect(
  RLMachine& machine, int x, int y, int width, int height, int dx, 
  int dy, int time)
  : SlideEffect(machine, x,y, width, height, dx, dy, time)
{}

// -----------------------------------------------------------------------

void SlideLeftToRightEffect::performEffectForTime(RLMachine& machine,
                                                  int currentTime)
{
  GraphicsSystem& graphics = machine.system().graphics();
  int amountVisible = calculateAmountVisible(currentTime, width());
  
  graphics.getDC(0).
    renderToScreen(x(), y(), 
                   x() + width() - amountVisible, y() + height(),
                   dx() + amountVisible, dy(),
                   dx() + width(), dy() + height(),
                   255);
  graphics.getDC(1).
    renderToScreen(x() + width() - amountVisible, y(),
                   x() + width(), y() + height(),
                   dx(), dy(),
                   dx() + amountVisible, dy() + height(),
                   255);
}

// -----------------------------------------------------------------------
// SlideFromRightToLeftEffect
// -----------------------------------------------------------------------

SlideRightToLeftEffect::SlideRightToLeftEffect(
  RLMachine& machine, int x, int y, int width, int height, int dx, 
  int dy, int time)
  : SlideEffect(machine, x, y, width, height, dx, dy, time)
{}

// -----------------------------------------------------------------------

void SlideRightToLeftEffect::performEffectForTime(RLMachine& machine,
                                                 int currentTime)
{
  GraphicsSystem& graphics = machine.system().graphics();
  int amountVisible = calculateAmountVisible(currentTime, width());

  graphics.getDC(0).
    renderToScreen(x() + amountVisible, y(),
                   x() + width(), y() + height(),
                   dx(), dy(),
                   dx() + width() - amountVisible, dy() + height(),
                   255);
  graphics.getDC(1).
    renderToScreen(x(), y(),
                   x() + amountVisible, y() + height(),
                   dx() + width() - amountVisible, dy() + height(),
                   dx() + width(), dy() + height(),
                   255);
}
