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
 * @file   WipeEffect.cpp
 * @author Elliot Glaysher
 * @date   Thu Nov  2 20:33:21 2006
 * 
 * @brief  Implements #SEL transition style #10, Wipe.
 * 
 * 
 */


#include "WipeEffect.hpp"
#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/GraphicsSystem.hpp"

#include <iostream>
#include <cmath>

using namespace std;

// -----------------------------------------------------------------------
// WipeEffect base class
// -----------------------------------------------------------------------

bool WipeEffect::blitOriginalImage() const
{
  return true; 
}

// -----------------------------------------------------------------------

/** 
 * Calculates the size of the interpolation and main polygons.
 *
 * There are 3 possible stages:
 * - [0, m_interpolationInPixels) - Draw only the
 *   transition. (sizeOfMainPolygon == 0, sizeOfInterpolation ==
 *   amountVisible)
 * - [m_interpolationInPixels, sizeOfScreen) - Draw both
 *   polygons. (sizeOfMainPolygon == amountVisible -
 *   m_interpolationInPixels, sizeOfInterpolation == amountVisible)
 * - [height, height + m_interpolationInPixels) - Draw both
 *   polygons, flooring the height of the transition to 
 * 
 * @param sizeOfInterpolation 
 * @param sizeOfMainPolygon 
 * @param sizeOfScreen Size of the area we are rendering to. Name is
 *                     intentionally ambigous so we can apply this to
 *                     both width and height.
 */
void WipeEffect::calculateSizes(int currentTime,
                                int& sizeOfInterpolation, 
                                int& sizeOfMainPolygon,
                                int sizeOfScreen)
{
  int amountVisible = (float(currentTime) / duration()) * 
    (sizeOfScreen + m_interpolationInPixels);
  if(amountVisible < m_interpolationInPixels)
  {
    sizeOfInterpolation = amountVisible;
    sizeOfMainPolygon = 0;
  }
  else if(amountVisible < sizeOfScreen)
  {
    sizeOfInterpolation = m_interpolationInPixels;
    sizeOfMainPolygon = amountVisible - m_interpolationInPixels;
  }
  else if(amountVisible < sizeOfScreen + m_interpolationInPixels)
  {
    sizeOfMainPolygon = amountVisible - m_interpolationInPixels;
    sizeOfInterpolation = sizeOfScreen - sizeOfMainPolygon;
  }
}

// -----------------------------------------------------------------------

WipeEffect::WipeEffect(RLMachine& machine, int width, 
                       int height, int time, 
                       int interpolation)
  : Effect(machine, width, height, time), 
    m_interpolation(interpolation),
    m_interpolationInPixels(0)
{
  if(m_interpolation)
    m_interpolationInPixels = pow(float(2), interpolation) * 2.5;
}

// -----------------------------------------------------------------------
// WipeTopToBottomEffect
// -----------------------------------------------------------------------

WipeTopToBottomEffect::WipeTopToBottomEffect(
  RLMachine& machine, int width, int height, int time, int interpolation)
  : WipeEffect(machine, width, height, time, interpolation)
{}

// -----------------------------------------------------------------------

void WipeTopToBottomEffect::performEffectForTime(RLMachine& machine, 
                                                 int currentTime)
{
  GraphicsSystem& graphics = machine.system().graphics();
  int sizeOfInterpolation, sizeOfMainPolygon;
  calculateSizes(currentTime, sizeOfInterpolation, sizeOfMainPolygon, height());

  if(sizeOfMainPolygon)
  {
    graphics.getDC(1).
      renderToScreen(0, height() - sizeOfMainPolygon - sizeOfInterpolation, 
                     width(), height() - sizeOfInterpolation, 
                     0, 0, width(), sizeOfMainPolygon,
                     255);
  }

  if(sizeOfInterpolation)
  {
    int opacity[4] = {255, 255, 0, 0};

    graphics.getDC(1).
      renderToScreen(0, height() - sizeOfInterpolation, 
                     width(), height(), 
                     0, sizeOfMainPolygon, width(), 
                     sizeOfMainPolygon + sizeOfInterpolation,
                     opacity);    
  }
}

// -----------------------------------------------------------------------
// WipeBottomToTopEffect
// -----------------------------------------------------------------------

WipeBottomToTopEffect::WipeBottomToTopEffect(
  RLMachine& machine, int width, int height, int time, int interpolation)
  : WipeEffect(machine, width, height, time, interpolation)
{}

// -----------------------------------------------------------------------

void WipeBottomToTopEffect::performEffectForTime(RLMachine& machine,
                                                 int currentTime)
{
  GraphicsSystem& graphics = machine.system().graphics();

  int sizeOfInterpolation, sizeOfMainPolygon;
  calculateSizes(currentTime, sizeOfInterpolation, 
                 sizeOfMainPolygon, height());

  // Render the sliding on frame
  if(sizeOfMainPolygon)
  {
    graphics.getDC(1).
      renderToScreen(0, sizeOfInterpolation, 
                     width(), sizeOfInterpolation + sizeOfMainPolygon, 
                     0, height() - sizeOfMainPolygon, 
                     width(), height(),
                     255);
  }

  if(sizeOfInterpolation)
  {
    int opacity[4] = {0, 0, 255, 255};
    graphics.getDC(1).
      renderToScreen(0, 0,
                     width(), sizeOfInterpolation,
                     0, height() - sizeOfMainPolygon - sizeOfInterpolation,
                     width(), height() - sizeOfMainPolygon,
                     opacity);
  }
}

// -----------------------------------------------------------------------
// WipeFromLeftToRightEffect
// -----------------------------------------------------------------------

WipeLeftToRightEffect::WipeLeftToRightEffect(
  RLMachine& machine, int width, int height, int time, int interpolation)
  : WipeEffect(machine, width, height, time, interpolation)
{}

// -----------------------------------------------------------------------

void WipeLeftToRightEffect::performEffectForTime(RLMachine& machine,
                                                  int currentTime)
{
  GraphicsSystem& graphics = machine.system().graphics();
  int sizeOfInterpolation, sizeOfMainPolygon;
  calculateSizes(currentTime, sizeOfInterpolation, sizeOfMainPolygon, width());

  if(sizeOfMainPolygon)
  {
    graphics.getDC(1).
      renderToScreen(width() - sizeOfInterpolation - sizeOfMainPolygon, 0, 
                     width() - sizeOfInterpolation, height(), 
                     0, 0, sizeOfMainPolygon, height(),
                     255);
  }

  if(sizeOfInterpolation)
  {
    int opacity[4] = {255, 0, 0, 255};
    graphics.getDC(1).
      renderToScreen(width() - sizeOfInterpolation, 0, 
                     width(), height(), 
                     sizeOfMainPolygon, 0, 
                     sizeOfMainPolygon + sizeOfInterpolation, height(),
                     opacity);
  }
}

// -----------------------------------------------------------------------
// WipeFromRightToLeftEffect
// -----------------------------------------------------------------------

WipeRightToLeftEffect::WipeRightToLeftEffect(
  RLMachine& machine, int width, int height, int time, int interpolation)
  : WipeEffect(machine, width, height, time, interpolation)
{}

// -----------------------------------------------------------------------

void WipeRightToLeftEffect::performEffectForTime(RLMachine& machine,
                                                 int currentTime)
{
  GraphicsSystem& graphics = machine.system().graphics();
  int sizeOfInterpolation, sizeOfMainPolygon;
  calculateSizes(currentTime, sizeOfInterpolation, sizeOfMainPolygon, width());

  if(sizeOfMainPolygon)
  {
    // broken
    graphics.getDC(1).
      renderToScreen(sizeOfInterpolation, 0, 
                     sizeOfInterpolation + sizeOfMainPolygon, height(), 
                     width() - sizeOfMainPolygon, 0,
                     width(), height(),
                     255);
  }

  if(sizeOfInterpolation)
  {
    int opacity[4] = {0, 255, 255, 0};
    graphics.getDC(1).
      renderToScreen(0, 0, 
                     sizeOfInterpolation, height(), 
                     width() - sizeOfInterpolation - sizeOfMainPolygon, 0,
                     width() - sizeOfMainPolygon, height(),
                     opacity);
  }
}
