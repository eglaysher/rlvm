// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006 Elliot Glaysher
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

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

/**
 * @file   BlindEffect.cpp
 * @author Elliot Glaysher
 * @date   Tue Feb 27 21:11:04 2007
 * 
 * @brief  Implements \#SEL transition style \#10, Blind.
 */

#include "Modules/BlindEffect.hpp"
#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/Surface.hpp"

#include <iostream>
#include <boost/bind.hpp>

using std::cerr;
using std::endl;
using boost::bind;
using boost::function;
using boost::ref;

// -----------------------------------------------------------------------
// BlindEffect base class
// -----------------------------------------------------------------------

bool BlindEffect::blitOriginalImage() const
{
  return true;
}

// -----------------------------------------------------------------------


BlindEffect::BlindEffect(RLMachine& machine, boost::shared_ptr<Surface> src,
                         boost::shared_ptr<Surface> dst,
                         int width, int height, int time, 
                         int blindSize)
  : Effect(machine, src, dst, width, height, time), 
    m_blindSize(blindSize)
{
}

// -----------------------------------------------------------------------

BlindEffect::~BlindEffect()
{}

// -----------------------------------------------------------------------

void BlindEffect::computeGrowing(RLMachine& machine, int maxSize,
                                 int currentTime)
{
  GraphicsSystem& graphics = machine.system().graphics();
  int numBlinds = height() / blindSize() + 1;
  int rowsToDisplay = int((float(currentTime) / duration()) * 
                          (blindSize() + numBlinds));

  for(int currentBlind = 0; currentBlind < numBlinds; ++currentBlind)
  {
    if(currentBlind <= rowsToDisplay)
    {
      int currentlyDisplayed = currentBlind - rowsToDisplay;
      if(currentlyDisplayed > blindSize())
        currentlyDisplayed = blindSize();

      int polygonStart = currentBlind * blindSize();

      renderPolygon(polygonStart, polygonStart + currentlyDisplayed);
    }
  }
}

// -----------------------------------------------------------------------

void BlindEffect::computeDecreasing(RLMachine& machine, int maxSize, 
                                    int currentTime)
{
  GraphicsSystem& graphics = machine.system().graphics();
  int numBlinds = maxSize / blindSize() + 1;
  int rowsToDisplay = int((float(currentTime) / duration()) * (blindSize() + numBlinds));

  for(int currentBlind = numBlinds; currentBlind >= 0; --currentBlind)
  {
    if((numBlinds - currentBlind) < rowsToDisplay)
    {
      int currentlyDisplayed = numBlinds - currentBlind - rowsToDisplay;
      if(currentlyDisplayed > blindSize())
        currentlyDisplayed = blindSize();

      int bottomOfPolygon = currentBlind * blindSize();

      renderPolygon(bottomOfPolygon, bottomOfPolygon - currentlyDisplayed);
    }
  }
}

// -----------------------------------------------------------------------
// BlindTopToBottomEffect
// -----------------------------------------------------------------------

BlindTopToBottomEffect::BlindTopToBottomEffect(
  RLMachine& machine, boost::shared_ptr<Surface> src,
  boost::shared_ptr<Surface> dst, 
  int width, int height, int time, int blindSize)
  : BlindEffect(machine, src, dst, width, height, time, 
                blindSize)
{}

// -----------------------------------------------------------------------

void BlindTopToBottomEffect::performEffectForTime(RLMachine& machine, 
                                                  int currentTime)
{
  computeGrowing(machine, height(), currentTime);
}

// -----------------------------------------------------------------------

void BlindTopToBottomEffect::renderPolygon(int polyStart, int polyEnd)
{
  srcSurface().renderToScreen(0, polyStart, width(), polyEnd,
                              0, polyStart, width(), polyEnd,
                              255);
}

// -----------------------------------------------------------------------
// BlindBottomToTopEffect
// -----------------------------------------------------------------------

BlindBottomToTopEffect::BlindBottomToTopEffect(
  RLMachine& machine, boost::shared_ptr<Surface> src,
  boost::shared_ptr<Surface> dst, 
  int width, int height, int time, int blindSize)
  : BlindEffect(machine, src, dst, width, height, time, 
                blindSize)
{}

// -----------------------------------------------------------------------

void BlindBottomToTopEffect::performEffectForTime(RLMachine& machine, 
                                                  int currentTime)
{
  computeDecreasing(machine, height(), currentTime);
}

// -----------------------------------------------------------------------

void BlindBottomToTopEffect::renderPolygon(int polyStart, int polyEnd)
{
  // Render polygon
  srcSurface().
    renderToScreen(0, polyEnd, width(), polyStart,
                   0, polyEnd, width(), polyStart,
                   255);      
}

// -----------------------------------------------------------------------
// BlindLeftToRightEffect
// -----------------------------------------------------------------------

BlindLeftToRightEffect::BlindLeftToRightEffect(
  RLMachine& machine, boost::shared_ptr<Surface> src,
  boost::shared_ptr<Surface> dst, 
  int width, int height, int time, int blindSize)
  : BlindEffect(machine, src, dst, width, height, time, 
                blindSize)
{
}

// -----------------------------------------------------------------------

void BlindLeftToRightEffect::performEffectForTime(RLMachine& machine, 
                                                  int currentTime)
{
  computeGrowing(machine, width(), currentTime);
}

// -----------------------------------------------------------------------

void BlindLeftToRightEffect::renderPolygon(int polyStart, int polyEnd)
{
  srcSurface().renderToScreen(polyStart, 0, polyEnd, height(),
                              polyStart, 0, polyEnd, height(),
                              255);
}

// -----------------------------------------------------------------------
// BlindRightToLeftEffect
// -----------------------------------------------------------------------

BlindRightToLeftEffect::BlindRightToLeftEffect(
  RLMachine& machine, boost::shared_ptr<Surface> src,
  boost::shared_ptr<Surface> dst, 
  int width, int height, int time, int blindSize)
  : BlindEffect(machine, src, dst, width, height, time, 
                blindSize)
{}

// -----------------------------------------------------------------------

void BlindRightToLeftEffect::performEffectForTime(RLMachine& machine, 
                                                  int currentTime)
{
  computeDecreasing(machine, width(), currentTime);
}

// -----------------------------------------------------------------------

void BlindRightToLeftEffect::renderPolygon(int polyStart, int polyEnd)
{
      srcSurface().
        renderToScreen(polyEnd, 0,
                       polyStart, height(),
                       polyEnd, 0,
                       polyStart, height(),
                       255);      
}
