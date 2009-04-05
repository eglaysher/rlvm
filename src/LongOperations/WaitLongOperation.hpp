// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2009 Elliot Glaysher
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
// -----------------------------------------------------------------------

#ifndef __WaitLongOperation_hpp__
#define __WaitLongOperation_hpp__

#include <boost/function.hpp>

#include "MachineBase/LongOperation.hpp"

class Point;

/// Pauses interpretation, waiting for input, an event or a user click.
class WaitLongOperation : public LongOperation {
 public:
  WaitLongOperation(RLMachine& machine);
  ~WaitLongOperation();

  /// This instance should wait time milliseconds and then return.
  void waitMilliseconds(unsigned int time);

  /// Whether we should exit this long operation on a click.
  void breakOnClicks();

  /// Checks |function| on every LongOperation invocation to see if we are
  /// finished. |function| should return true if we are done.
  void breakOnEvent(const boost::function<bool()>& function);

  /// Whether we write out the location of a mouse click. Implies that we're
  /// breaking on mouse click.
  void saveClickLocation(IntReferenceIterator x, IntReferenceIterator y);

  void mouseMotion(const Point&);

  // Overridden from EventListener:
  virtual bool mouseButtonStateChanged(MouseButton mouseButton, bool pressed);
  virtual bool keyStateChanged(KeyCode keyCode, bool pressed);

  void recordMouseCursorPosition();

  // Overridden from LongOperation:
  bool operator()(RLMachine& machine);

 private:
  RLMachine& machine_;

  bool wait_until_target_time_;
  unsigned int target_time_;

  bool break_on_clicks_;
  int button_pressed_;

  bool break_on_event_;
  boost::function<bool()> event_function_;

  bool break_on_ctrl_pressed_;
  bool ctrl_pressed_;

  bool mouse_moved_;

  bool save_click_location_;
  IntReferenceIterator x_;
  IntReferenceIterator y_;
};

#endif
