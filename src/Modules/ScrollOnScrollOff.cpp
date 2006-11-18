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
 * 
 * @brief  Implements #SEL transition style #15, ScrollOnScrollOff.
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

bool ScrollOnScrollOff::blitOriginalImage() const
{
  return false; 
}

// -----------------------------------------------------------------------

int ScrollOnScrollOff::calculateAmountVisible(int currentTime, int screenSize)
{
  return int((float(currentTime) / duration()) * screenSize);
}

// -----------------------------------------------------------------------

ScrollOnScrollOff::ScrollOnScrollOff(RLMachine& machine, int width, 
                         int height, int time)
  : Effect(machine, width, height, time)
{
}

// -----------------------------------------------------------------------
// ScrollOnScrollOffTopToBottomEffect
// -----------------------------------------------------------------------

ScrollOnScrollOffTopToBottomEffect::ScrollOnScrollOffTopToBottomEffect(
  RLMachine& machine, int width, int height, int time)
  : ScrollOnScrollOff(machine, width, height, time)
{}

// -----------------------------------------------------------------------

void ScrollOnScrollOffTopToBottomEffect::performEffectForTime(RLMachine& machine, 
                                                 int currentTime)
{
  GraphicsSystem& graphics = machine.system().graphics();
  int amountVisible = calculateAmountVisible(currentTime, height());
  
   graphics.getDC(0).
     renderToScreen(0, 0, 
                    width(), height() - amountVisible,
                    0, amountVisible, 
                    width(), height(),
                    255);
  graphics.getDC(1).
    renderToScreen(0, height() - amountVisible,
                   width(), height(),
                   0, 0, 
                   width(), amountVisible,
                   255);
}

// -----------------------------------------------------------------------
// ScrollOnScrollOffBottomToTopEffect
// -----------------------------------------------------------------------

ScrollOnScrollOffBottomToTopEffect::ScrollOnScrollOffBottomToTopEffect(
  RLMachine& machine, int width, int height, int time)
  : ScrollOnScrollOff(machine, width, height, time)
{}

// -----------------------------------------------------------------------

void ScrollOnScrollOffBottomToTopEffect::performEffectForTime(RLMachine& machine,
                                                 int currentTime)
{
  GraphicsSystem& graphics = machine.system().graphics();
  int amountVisible = calculateAmountVisible(currentTime, height());
  
   graphics.getDC(0).
     renderToScreen(0, amountVisible, 
                    width(), height(),
                    0, 0,
                    width(), height() - amountVisible,
                    255);
   graphics.getDC(1).
     renderToScreen(0, 0,
                    width(), amountVisible,
                    0, height() - amountVisible, 
                    width(), height(),
                    255);
}

// -----------------------------------------------------------------------
// ScrollOnScrollOffFromLeftToRightEffect
// -----------------------------------------------------------------------

ScrollOnScrollOffLeftToRightEffect::ScrollOnScrollOffLeftToRightEffect(
  RLMachine& machine, int width, int height, int time)
  : ScrollOnScrollOff(machine, width, height, time)
{}

// -----------------------------------------------------------------------

void ScrollOnScrollOffLeftToRightEffect::performEffectForTime(RLMachine& machine,
                                                  int currentTime)
{
  GraphicsSystem& graphics = machine.system().graphics();
  int amountVisible = calculateAmountVisible(currentTime, width());
  
  graphics.getDC(0).
    renderToScreen(0, 0, 
                   width() - amountVisible, height(),
                   amountVisible, 0,
                   width(), height(),
                   255);
  graphics.getDC(1).
    renderToScreen(width() - amountVisible, 0,
                   width(), height(),
                   0, 0,
                   amountVisible, height(),
                   255);
}

// -----------------------------------------------------------------------
// ScrollOnScrollOffFromRightToLeftEffect
// -----------------------------------------------------------------------

ScrollOnScrollOffRightToLeftEffect::ScrollOnScrollOffRightToLeftEffect(
  RLMachine& machine, int width, int height, int time)
  : ScrollOnScrollOff(machine, width, height, time)
{}

// -----------------------------------------------------------------------

void ScrollOnScrollOffRightToLeftEffect::performEffectForTime(RLMachine& machine,
                                                 int currentTime)
{
  GraphicsSystem& graphics = machine.system().graphics();
  int amountVisible = calculateAmountVisible(currentTime, width());

  graphics.getDC(0).
    renderToScreen(amountVisible, 0,
                   width(), height(),
                   0, 0,
                   width() - amountVisible, height(),
                   255);
  graphics.getDC(1).
    renderToScreen(0, 0,
                   amountVisible, height(),
                   width() - amountVisible, height(),
                   width(), height(),
                   255);
}
