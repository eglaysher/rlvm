// This file is part of RLVM, a RealLive virtual machine clone.
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

#include "Systems/Base/EventSystem.hpp"
#include "Systems/Base/FrameCounter.hpp"

#include "libReallive/defs.h"

#include <iostream>

using namespace std;
using namespace boost;
using namespace libReallive;

// -----------------------------------------------------------------------

EventSystem::EventSystem() 
  : m_numberOfRealtimeTasks(0), m_numberOfNiceAfterEachTaskItems(0)
{}

// -----------------------------------------------------------------------

EventSystem::~EventSystem()
{}

// -----------------------------------------------------------------------

void EventSystem::setFrameCounter(int layer, int frameCounter, FrameCounter* counter)
{
  checkLayerAndCounter(layer, frameCounter);
  m_frameCounters[layer][frameCounter].reset(counter);
}

// -----------------------------------------------------------------------

FrameCounter& EventSystem::getFrameCounter(int layer, int frameCounter)
{
  checkLayerAndCounter(layer, frameCounter);

  scoped_ptr<FrameCounter>& counter = m_frameCounters[layer][frameCounter];
  if(counter.get() == NULL)
    throw Error("Trying to get an uninitialized frame counter!");

  return *counter;
}

// -----------------------------------------------------------------------

bool EventSystem::frameCounterExists(int layer, int frameCounter)
{
  checkLayerAndCounter(layer, frameCounter);
  scoped_ptr<FrameCounter>& counter = m_frameCounters[layer][frameCounter];
  return counter.get() != NULL;
}

// -----------------------------------------------------------------------

void EventSystem::beginBeingNiceAfterEachPass()
{
  m_numberOfNiceAfterEachTaskItems++;
}

// -----------------------------------------------------------------------

void EventSystem::endBeingNiceAfterEachPass()
{
  m_numberOfNiceAfterEachTaskItems--;
}

// -----------------------------------------------------------------------

bool EventSystem::beNiceAfterEachPass()
{
  return m_numberOfNiceAfterEachTaskItems > 0;
}

// -----------------------------------------------------------------------

void EventSystem::beginRealtimeTask()
{
//  cerr << "EventSystem::beginRealtimeTask()" << endl;
  m_numberOfRealtimeTasks++;
}

// -----------------------------------------------------------------------

void EventSystem::endRealtimeTask()
{
//  cerr << "EventSystem::endRealtimeTask()" << endl;
  m_numberOfRealtimeTasks--;
}

// -----------------------------------------------------------------------

bool EventSystem::canBeNice()
{
  return m_numberOfRealtimeTasks == 0;
}

// -----------------------------------------------------------------------

void EventSystem::checkLayerAndCounter(int layer, int frameCounter)
{
  if(layer < 0 || layer > 1)
    throw Error("Illegal frame counter layer!");

  if(frameCounter < 0 || frameCounter > 255)
    throw Error("Frame Counter index out of range!");
}
