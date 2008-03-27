// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006, 2007 Elliot Glaysher
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
 * @file   WipeEffect.cpp
 * @author Elliot Glaysher
 * @date   Thu Nov  2 20:33:21 2006
 * 
 * @ingroup TransitionEffects
 * @brief  Implements \#SEL transition style \#10, Wipe.
 */


#include "Effects/WipeEffect.hpp"
#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/Surface.hpp"

#include <iostream>
#include <cmath>

#include <boost/shared_ptr.hpp>

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
 * @param[in] currentTime The current number of ms since the Effect started
 * @param[out] sizeOfInterpolation Size of the interpolation
 * @param[out] sizeOfMainPolygon Size of the main polygon
 * @param[in] sizeOfScreen Size of the area we are rendering to. Name is
 *                     intentionally ambigous so we can apply this to
 *                     both width and height.
 */
void WipeEffect::calculateSizes(int currentTime,
                                int& sizeOfInterpolation, 
                                int& sizeOfMainPolygon,
                                int sizeOfScreen)
{
  int amountVisible = int((float(currentTime) / duration()) * 
                          (sizeOfScreen + m_interpolationInPixels));
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

WipeEffect::WipeEffect(RLMachine& machine, boost::shared_ptr<Surface> src,
                       boost::shared_ptr<Surface> dst,
                       int width, int height, int time, 
                       int interpolation)
  : Effect(machine, src, dst, width, height, time), 
    m_interpolation(interpolation),
    m_interpolationInPixels(0)
{
  if(m_interpolation)
    m_interpolationInPixels = int(pow(float(2), interpolation) * 2.5);
}

// -----------------------------------------------------------------------
// WipeTopToBottomEffect
// -----------------------------------------------------------------------

WipeTopToBottomEffect::WipeTopToBottomEffect(
  RLMachine& machine, boost::shared_ptr<Surface> src,
  boost::shared_ptr<Surface> dst, 
  int width, int height, int time, 
  int interpolation)
  : WipeEffect(machine, src, dst, width, height, time, interpolation)
{}

// -----------------------------------------------------------------------

void WipeTopToBottomEffect::performEffectForTime(RLMachine& machine, 
                                                 int currentTime)
{
  int sizeOfInterpolation, sizeOfMainPolygon;
  calculateSizes(currentTime, sizeOfInterpolation, sizeOfMainPolygon, height());

  if(sizeOfMainPolygon)
  {
    srcSurface().
      renderToScreen(0, 0,
                     width(), sizeOfMainPolygon,
                     0, 0, width(), sizeOfMainPolygon,
                     255);
  }

  if(sizeOfInterpolation)
  {
    int opacity[4] = {255, 255, 0, 0};

    srcSurface().
      renderToScreen(0, sizeOfMainPolygon,
                     width(), sizeOfMainPolygon + sizeOfInterpolation,
                     0, sizeOfMainPolygon, width(), 
                     sizeOfMainPolygon + sizeOfInterpolation,
                     opacity);    
  }
}

// -----------------------------------------------------------------------
// WipeBottomToTopEffect
// -----------------------------------------------------------------------

WipeBottomToTopEffect::WipeBottomToTopEffect(
  RLMachine& machine,  boost::shared_ptr<Surface> src,
  boost::shared_ptr<Surface> dst, 
  int width, int height, int time, 
  int interpolation)
  : WipeEffect(machine, src, dst, width, height, time, interpolation)
{}

// -----------------------------------------------------------------------

void WipeBottomToTopEffect::performEffectForTime(RLMachine& machine,
                                                 int currentTime)
{
  int sizeOfInterpolation, sizeOfMainPolygon;
  calculateSizes(currentTime, sizeOfInterpolation, 
                 sizeOfMainPolygon, height());

  // Render the sliding on frame
  if(sizeOfMainPolygon)
  {
    srcSurface().
      renderToScreen(0, height() - sizeOfMainPolygon,
                     width(), height(),
                     0, height() - sizeOfMainPolygon, 
                     width(), height(),
                     255);
  }

  if(sizeOfInterpolation)
  {
    int opacity[4] = {0, 0, 255, 255};
    srcSurface().
      renderToScreen(0, height() - sizeOfMainPolygon - sizeOfInterpolation,
                     width(), height() - sizeOfMainPolygon,
                     0, height() - sizeOfMainPolygon - sizeOfInterpolation,
                     width(), height() - sizeOfMainPolygon,
                     opacity);
  }
}

// -----------------------------------------------------------------------
// WipeFromLeftToRightEffect
// -----------------------------------------------------------------------

WipeLeftToRightEffect::WipeLeftToRightEffect(
  RLMachine& machine,  boost::shared_ptr<Surface> src,
  boost::shared_ptr<Surface> dst,
  int width, int height, int time, 
  int interpolation)
  : WipeEffect(machine, src, dst, width, height, time, interpolation)
{}

// -----------------------------------------------------------------------

void WipeLeftToRightEffect::performEffectForTime(RLMachine& machine,
                                                  int currentTime)
{
  int sizeOfInterpolation, sizeOfMainPolygon;
  calculateSizes(currentTime, sizeOfInterpolation, sizeOfMainPolygon, width());


  // CONTINUE FIXING THE WIPES HERE!

  if(sizeOfMainPolygon)
  {
    srcSurface().
      renderToScreen(0, 0, 
                     sizeOfMainPolygon, height(), 
                     0, 0, sizeOfMainPolygon, height(),
                     255);
  }

  if(sizeOfInterpolation)
  {
    int opacity[4] = {255, 0, 0, 255};
    srcSurface().
      renderToScreen(sizeOfMainPolygon, 0, 
                     sizeOfMainPolygon + sizeOfInterpolation, height(), 
                     sizeOfMainPolygon, 0, 
                     sizeOfMainPolygon + sizeOfInterpolation, height(),
                     opacity);
  }
}

// -----------------------------------------------------------------------
// WipeFromRightToLeftEffect
// -----------------------------------------------------------------------

WipeRightToLeftEffect::WipeRightToLeftEffect(
  RLMachine& machine, boost::shared_ptr<Surface> src,
  boost::shared_ptr<Surface> dst,
  int width, int height, int time,
  int interpolation)
  : WipeEffect(machine, src, dst, width, height, time, interpolation)
{}

// -----------------------------------------------------------------------

void WipeRightToLeftEffect::performEffectForTime(RLMachine& machine,
                                                 int currentTime)
{
  int sizeOfInterpolation, sizeOfMainPolygon;
  calculateSizes(currentTime, sizeOfInterpolation, sizeOfMainPolygon, width());

  if(sizeOfMainPolygon)
  {
    srcSurface().
      renderToScreen(width() - sizeOfMainPolygon, 0, 
                     width(), height(), 
                     width() - sizeOfMainPolygon, 0,
                     width(), height(),
                     255);
  }

  if(sizeOfInterpolation)
  {
    int opacity[4] = {0, 255, 255, 0};
    srcSurface().
      renderToScreen(width() - sizeOfInterpolation - sizeOfMainPolygon, 0,
                     width() - sizeOfMainPolygon, height(),
                     width() - sizeOfInterpolation - sizeOfMainPolygon, 0,
                     width() - sizeOfMainPolygon, height(),
                     opacity);
  }
}
