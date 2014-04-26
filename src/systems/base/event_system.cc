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

#include "systems/base/event_system.h"

#include "libreallive/gameexe.h"
#include "machine/long_operation.h"
#include "machine/rlmachine.h"
#include "systems/base/event_listener.h"
#include "systems/base/frame_counter.h"
#include "utilities/exception.h"

// -----------------------------------------------------------------------
// EventSystemGlobals
// -----------------------------------------------------------------------
EventSystemGlobals::EventSystemGlobals()
    : generic1_(false), generic2_(false) {
}

EventSystemGlobals::EventSystemGlobals(Gameexe& gexe)
    : generic1_(gexe("INIT_ORIGINALSETING1_MOD").ToInt(0)),
      generic2_(gexe("INIT_ORIGINALSETING2_MOD").ToInt(0)) {}

// -----------------------------------------------------------------------
// EventSystem
// -----------------------------------------------------------------------
EventSystem::EventSystem(Gameexe& gexe) : globals_(gexe) {}

EventSystem::~EventSystem() {}

RLTimer& EventSystem::GetTimer(int layer, int counter) {
  if (layer >= 2)
    throw rlvm::Exception("Invalid layer in EventSystem::GetTimer.");
  if (counter >= 255)
    throw rlvm::Exception("Invalid counter in EventSystem::GetTimer.");
  return timers_[layer][counter];
}

void EventSystem::SetFrameCounter(int layer,
                                  int frame_counter,
                                  FrameCounter* counter) {
  CheckLayerAndCounter(layer, frame_counter);
  frame_counters_[layer][frame_counter].reset(counter);
}

FrameCounter& EventSystem::GetFrameCounter(int layer, int frame_counter) {
  CheckLayerAndCounter(layer, frame_counter);

  std::unique_ptr<FrameCounter>& counter =
      frame_counters_[layer][frame_counter];
  if (counter.get() == NULL)
    throw rlvm::Exception("Trying to get an uninitialized frame counter!");

  return *counter;
}

bool EventSystem::FrameCounterExists(int layer, int frame_counter) {
  CheckLayerAndCounter(layer, frame_counter);
  std::unique_ptr<FrameCounter>& counter =
      frame_counters_[layer][frame_counter];
  return counter.get() != NULL;
}

void EventSystem::AddMouseListener(EventListener* listener) {
  event_listeners_.insert(listener);
}

void EventSystem::RemoveMouseListener(EventListener* listener) {
  event_listeners_.erase(listener);
}

void EventSystem::DispatchEvent(
    RLMachine& machine,
    const std::function<bool(EventListener&)>& event) {
  // In addition to the handled variable, we need to add break statements to
  // the loops since |event| can be any arbitrary code and may modify listeners
  // or handlers. (i.e., System::showSyscomMenu)
  bool handled = false;

  // Give the mostly passive listeners first shot at handling this event
  EventListeners::iterator listenerIt = listeners_begin();
  for (; !handled && listenerIt != listeners_end(); ++listenerIt) {
    if (event(**listenerIt)) {
      handled = true;
      break;
    }
  }

  // Try to pass the event on to the top of the call stack.
  std::shared_ptr<LongOperation> current_op = machine.CurrentLongOperation();
  if (current_op)
    event(*current_op);
}

void EventSystem::BroadcastEvent(
    RLMachine& machine,
    const std::function<void(EventListener&)>& event) {
  for (EventListener* listener : event_listeners_)
    event(*listener);

  std::shared_ptr<LongOperation> current_op = machine.CurrentLongOperation();
  if (current_op)
    event(*current_op);
}

void EventSystem::CheckLayerAndCounter(int layer, int frame_counter) {
  if (layer < 0 || layer > 1)
    throw rlvm::Exception("Illegal frame counter layer!");

  if (frame_counter < 0 || frame_counter > 255)
    throw rlvm::Exception("Frame Counter index out of range!");
}
