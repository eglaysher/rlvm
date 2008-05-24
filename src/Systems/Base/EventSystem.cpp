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

#include "Systems/Base/EventSystem.hpp"
#include "Systems/Base/FrameCounter.hpp"
#include "Utilities.h"
#include "libReallive/gameexe.h"

#include <iostream>

using namespace std;
using namespace boost;

// -----------------------------------------------------------------------
// EventSystemGlobals
// -----------------------------------------------------------------------

EventSystemGlobals::EventSystemGlobals()
  : generic1(false), generic2(false)
{}

// -----------------------------------------------------------------------

EventSystemGlobals::EventSystemGlobals(Gameexe& gexe)
  : generic1(gexe("INIT_ORIGINALSETING1_MOD").to_int(0)), 
    generic2(gexe("INIT_ORIGINALSETING2_MOD").to_int(0))
{}
 
// -----------------------------------------------------------------------
// EventSystem
// -----------------------------------------------------------------------
EventSystem::EventSystem(Gameexe& gexe) 
  : m_numberOfRealtimeTasks(0), m_numberOfNiceAfterEachTaskItems(0),
    m_globals(gexe)
{
}

// -----------------------------------------------------------------------

EventSystem::~EventSystem()
{}

// -----------------------------------------------------------------------

void EventSystem::addEventHandler(EventHandler* handler)
{
  Handlers::iterator it = 
    std::find(m_eventHandlers.begin(), m_eventHandlers.end(), handler);

  if(it == m_eventHandlers.end())
    m_eventHandlers.push_back(handler);
}

// -----------------------------------------------------------------------

void EventSystem::removeEventHandler(EventHandler* handler)
{
  Handlers::iterator it = 
    std::find(m_eventHandlers.begin(), m_eventHandlers.end(), handler);

  if(it != m_eventHandlers.end())
    m_eventHandlers.erase(it);
}

// -----------------------------------------------------------------------

void EventSystem::addMouseListener(MouseListener* listener)
{
  MouseListeners::iterator it = 
    std::find(m_mouseListeners.begin(), m_mouseListeners.end(), listener);

  if(it == m_mouseListeners.end())
    m_mouseListeners.push_back(listener);
}

// -----------------------------------------------------------------------

void EventSystem::removeMouseListener(MouseListener* listener)
{
  MouseListeners::iterator it = 
    std::find(m_mouseListeners.begin(), m_mouseListeners.end(), listener);

  if(it != m_mouseListeners.end())
    m_mouseListeners.erase(it);
}

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
    throw rlvm::Exception("Trying to get an uninitialized frame counter!");

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

void EventSystem::checkLayerAndCounter(int layer, int frameCounter)
{
  if(layer < 0 || layer > 1)
    throw rlvm::Exception("Illegal frame counter layer!");

  if(frameCounter < 0 || frameCounter > 255)
    throw rlvm::Exception("Frame Counter index out of range!");
}

// -----------------------------------------------------------------------

