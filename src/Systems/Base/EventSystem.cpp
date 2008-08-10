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
  : globals_(gexe)
{
}

// -----------------------------------------------------------------------

EventSystem::~EventSystem()
{}

// -----------------------------------------------------------------------

void EventSystem::addEventHandler(EventHandler* handler)
{
  Handlers::iterator it =
    std::find(event_handlers_.begin(), event_handlers_.end(), handler);

  if(it == event_handlers_.end())
    event_handlers_.push_back(handler);
}

// -----------------------------------------------------------------------

void EventSystem::removeEventHandler(EventHandler* handler)
{
  Handlers::iterator it =
    std::find(event_handlers_.begin(), event_handlers_.end(), handler);

  if(it != event_handlers_.end())
    event_handlers_.erase(it);
}

// -----------------------------------------------------------------------

void EventSystem::addMouseListener(MouseListener* listener)
{
  MouseListeners::iterator it =
    std::find(mouse_listeners_.begin(), mouse_listeners_.end(), listener);

  if(it == mouse_listeners_.end())
    mouse_listeners_.push_back(listener);
}

// -----------------------------------------------------------------------

void EventSystem::removeMouseListener(MouseListener* listener)
{
  MouseListeners::iterator it =
    std::find(mouse_listeners_.begin(), mouse_listeners_.end(), listener);

  if(it != mouse_listeners_.end())
    mouse_listeners_.erase(it);
}

// -----------------------------------------------------------------------

void EventSystem::setFrameCounter(int layer, int frame_counter, FrameCounter* counter)
{
  checkLayerAndCounter(layer, frame_counter);
  frame_counters_[layer][frame_counter].reset(counter);
}

// -----------------------------------------------------------------------

FrameCounter& EventSystem::getFrameCounter(int layer, int frame_counter)
{
  checkLayerAndCounter(layer, frame_counter);

  scoped_ptr<FrameCounter>& counter = frame_counters_[layer][frame_counter];
  if(counter.get() == NULL)
    throw rlvm::Exception("Trying to get an uninitialized frame counter!");

  return *counter;
}

// -----------------------------------------------------------------------

bool EventSystem::frameCounterExists(int layer, int frame_counter)
{
  checkLayerAndCounter(layer, frame_counter);
  scoped_ptr<FrameCounter>& counter = frame_counters_[layer][frame_counter];
  return counter.get() != NULL;
}

// -----------------------------------------------------------------------

void EventSystem::checkLayerAndCounter(int layer, int frame_counter)
{
  if(layer < 0 || layer > 1)
    throw rlvm::Exception("Illegal frame counter layer!");

  if(frame_counter < 0 || frame_counter > 255)
    throw rlvm::Exception("Frame Counter index out of range!");
}

// -----------------------------------------------------------------------

