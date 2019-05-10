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

#ifndef SRC_SYSTEMS_BASE_EVENT_SYSTEM_H_
#define SRC_SYSTEMS_BASE_EVENT_SYSTEM_H_

#include <functional>
#include <memory>
#include <queue>
#include <set>

#include "systems/base/rltimer.h"
#include "systems/base/rect.h"

class RLMachine;

class Gameexe;
class FrameCounter;
class EventListener;

// -----------------------------------------------------------------------

struct EventSystemGlobals {
  EventSystemGlobals();
  explicit EventSystemGlobals(Gameexe& gexe);

  // The two generic values that the reallive game has control over
  // with the Generic1 and Generic2 functions.
  int generic1_;
  int generic2_;

  // boost::serialization support
  template <class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar& generic1_ & generic2_;
  }
};

// -----------------------------------------------------------------------

// Generalization of an event system. Reallive's event model is a bit
// weird; interpreted code will check the state of certain keyboard
// modifiers, with functions such as CtrlPressed() or ShiftPressed().
//
// So what's the solution? Have two different event systems side by
// side. One is exposed to Reallive and mimics what RealLive bytecode
// expects. The other is based on event handlers and is sane.
class EventSystem {
 public:
  explicit EventSystem(Gameexe& gexe);
  virtual ~EventSystem();

  EventSystemGlobals& globals() { return globals_; }

  RLTimer& GetTimer(int layer, int counter);

  // Frame Counters
  //
  // "Frame counters are designed to make it simple to ensure events happen at a
  // constant speed regardless of the host system's specifications. Once a frame
  // counter has been initialized, it will count from one arbitrary number to
  // another, over a given length of time. The counter can be queried at any
  // point to get its current value."
  //
  // Valid values for layer are 0 and 1. Valid values for frame_counter are 0
  // through 255.
  void SetFrameCounter(int layer, int frame_counter, FrameCounter* counter);
  FrameCounter& GetFrameCounter(int layer, int frame_counter);
  bool FrameCounterExists(int layer, int frame_counter);

  // Keyboard and Mouse Input (Event Listener style)
  //
  // rlvm event handling works by registering objects that received input
  // notifications from the EventSystem. These objects are EventListeners,
  // which passively listen for input and have a first chance grab at any click
  // or keypress.
  //
  // If no EventListener claims the event, then we try to reinterpret the top
  // of the RLMachine callstack as an EventListener. Otherwise, the events
  // are handled RealLive style (see below).
  void AddMouseListener(EventListener* listener);
  void RemoveMouseListener(EventListener* listener);

  // Generic values
  //
  // These values should have, from the beginning, been placed somewhere
  // else. They will remain here till the end of time for save game file
  // compatibility, though.
  //
  // "RealLive provides two generic settings to permit games using the standard
  // system command menu to include custom options in it. The meaning of each
  // generic flag is left up to the programmer. Valid values are 0 to 4."
  void set_generic1(const int in) { globals_.generic1_ = in; }
  int generic1() const { return globals_.generic1_; }
  void set_generic2(const int in) { globals_.generic2_ = in; }
  int generic2() const { return globals_.generic2_; }

  // Run once per cycle through the game loop to process events.
  virtual void ExecuteEventSystem(RLMachine& machine) = 0;

  // Returns the number of milliseconds since the program
  // started. Used for timing things.
  virtual unsigned int GetTicks() const = 0;

  // Idles the program for a certain amount of time in milliseconds.
  virtual void Wait(unsigned int milliseconds) const = 0;

  // Keyboard and Mouse Input (Reallive style)
  //
  // RealLive applications poll for input, with all the problems that sort of
  // event handling has. We therefore provide an interface for polling.
  //
  // Don't use it. This interface is provided for RealLive
  // bytecode. EventListeners should be used within rlvm code, instead.

  // Returns whether shift is currently pressed.
  virtual bool ShiftPressed() const = 0;

  // Returns whether ctrl has been pressed since the last invocation of
  // ctrlPresesd().
  virtual bool CtrlPressed() const = 0;

  // Returns the current cursor hotspot.
  virtual Point GetCursorPos() = 0;

  // Gets the location of the mouse cursor and the button states.
  //
  // The following values are used to indicate a button's status:
  // - 0 if unpressed
  // - 1 if being pressed
  // - 2 if pressed and released.
  virtual void GetCursorPos(Point& position, int& button1, int& button2) = 0;

  // Resets the state of the mouse buttons.
  virtual void FlushMouseClicks() = 0;

  // Returns the time in ticks of the last mouse movement.
  virtual unsigned int TimeOfLastMouseMove() = 0;

  // Testing
  //
  // Allows test systems like lua_rlvm to inject mouse movement and clicks.
  virtual void InjectMouseMovement(RLMachine& machine, const Point& loc) = 0;
  virtual void InjectMouseDown(RLMachine& machine) = 0;
  virtual void InjectMouseUp(RLMachine& machine) = 0;

 protected:
  typedef std::set<EventListener*> EventListeners;

  EventListeners::iterator listeners_begin() {
    return event_listeners_.begin();
  }
  EventListeners::iterator listeners_end() { return event_listeners_.end(); }

  // Calls a EventListener member function on all event listeners, and then
  // event handlers, stopping when an object says they handled it.
  void DispatchEvent(RLMachine& machine,
                     const std::function<bool(EventListener&)>& event);
  void BroadcastEvent(RLMachine& machine,
                      const std::function<void(EventListener&)>& event);

 private:
  // Helper function that verifies input
  void CheckLayerAndCounter(int layer, int counter);

  std::unique_ptr<FrameCounter> frame_counters_[255][2];
  RLTimer timers_[255][2];

  EventListeners event_listeners_;

  EventSystemGlobals globals_;
};

#endif  // SRC_SYSTEMS_BASE_EVENT_SYSTEM_H_
