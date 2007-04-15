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
  GraphicsSystem& graphics = machine.system().graphics();
  int numBlinds = height() / blindSize() + 1;
  int rowsToDisplay = int((float(currentTime) / duration()) * (blindSize() + numBlinds));

  for(int currentBlind = 0; currentBlind < numBlinds; ++currentBlind)
  {
    if(currentBlind <= rowsToDisplay)
    {
      int currentlyDisplayed = currentBlind - rowsToDisplay;
      if(currentlyDisplayed > blindSize())
        currentlyDisplayed = blindSize();

      int polygonYStart = currentBlind * blindSize();

      // Render polygon
      srcSurface().
        renderToScreen(0, polygonYStart,
                       width(), polygonYStart + currentlyDisplayed,
                       0, polygonYStart,
                       width(), polygonYStart + currentlyDisplayed,
                       255);
    }
  }
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
  GraphicsSystem& graphics = machine.system().graphics();
  int numBlinds = height() / blindSize() + 1;
  int rowsToDisplay = int((float(currentTime) / duration()) * (blindSize() + numBlinds));

  for(int currentBlind = numBlinds; currentBlind >= 0; --currentBlind)
  {
    if((numBlinds - currentBlind) < rowsToDisplay)
    {
      int currentlyDisplayed = numBlinds - currentBlind - rowsToDisplay;
      if(currentlyDisplayed > blindSize())
        currentlyDisplayed = blindSize();

      int bottomOfPolygon = currentBlind * blindSize();

      // Render polygon
      srcSurface().
        renderToScreen(0, bottomOfPolygon - currentlyDisplayed,
                       width(), bottomOfPolygon,
                       0, bottomOfPolygon - currentlyDisplayed,
                       width(), bottomOfPolygon,
                       255);      
    }
  }
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
{}

// -----------------------------------------------------------------------

/**
 * @bug Doesn't always cover screen properly.
 */
void BlindLeftToRightEffect::performEffectForTime(RLMachine& machine, 
                                                  int currentTime)
{
  GraphicsSystem& graphics = machine.system().graphics();
  int numBlinds = width() / blindSize() + 1;
  int rowsToDisplay = int((float(currentTime) / duration()) * (blindSize() + numBlinds));

  for(int currentBlind = 0; currentBlind < numBlinds; ++currentBlind)
  {
    if(currentBlind <= rowsToDisplay)
    {
      int currentlyDisplayed = currentBlind - rowsToDisplay;
      if(currentlyDisplayed > blindSize())
        currentlyDisplayed = blindSize();

      int polygonXStart = currentBlind * blindSize();

      // Render polygon
      srcSurface().
        renderToScreen(polygonXStart, 0,
                       polygonXStart + currentlyDisplayed, height(),
                       polygonXStart, 0,
                       polygonXStart + currentlyDisplayed, height(),
                       255);
    }
  }
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
  GraphicsSystem& graphics = machine.system().graphics();
  int numBlinds = width() / blindSize() + 1;
  int rowsToDisplay = int((float(currentTime) / duration()) * (blindSize() + numBlinds));

  for(int currentBlind = numBlinds; currentBlind >= 0; --currentBlind)
  {
    if((numBlinds - currentBlind) < rowsToDisplay)
    {
      int currentlyDisplayed = numBlinds - currentBlind - rowsToDisplay;
      if(currentlyDisplayed > blindSize())
        currentlyDisplayed = blindSize();

      int rightOfPolygon = currentBlind * blindSize();

      // Render polygon
      srcSurface().
        renderToScreen(rightOfPolygon - currentlyDisplayed, 0,
                       rightOfPolygon, height(),
                       rightOfPolygon - currentlyDisplayed, 0,
                       rightOfPolygon, height(),
                       255);      
    }
  }
}
