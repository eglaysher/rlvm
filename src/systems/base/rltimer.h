// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006 Elliot Glaysher
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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
//
// -----------------------------------------------------------------------

#ifndef SRC_SYSTEMS_BASE_RLTIMER_H_
#define SRC_SYSTEMS_BASE_RLTIMER_H_

class EventSystem;

// "Timers are simple counters: when initialized, they start counting from 0,
// and can be queried at any point to return the number of milliseconds that
// have elapsed since their initialization.
//
// "RealLive provides two sets of 255 timers, the basic counters (カウント) and
// the extended counters (ＥＸカウント); the difference between them is unclear,
// other than that different sets of functions operate on each. Of the functions
// documented below, those with an `Ex' infix operate on the extended counters,
// and those without operate on the basic counters.
//
// "The number of the timer to use, counter, is optional in all these functions;
// in all cases, if it is not given, it will default to 0.
//
// "All timers run continuously; they cannot be stopped. The implementation
// appears to be based on the time since the interpreter was started, as this is
// what you get if you query a timer that has not been initialized to any other
// value. “Setting” a timer merely stores the time at which the timer was set,
// and future queries subtract this from the time since the interpreter was
// started and then add the value to which the timer was initialized."
class RLTimer {
 public:
  RLTimer();
  ~RLTimer();

  // Returns the current value of this frame counter. This value represents the
  // number of milliseconds since the timer was initialized.
  unsigned int Read(EventSystem& events);

  void Set(EventSystem& events, unsigned int value = 0);

 private:
  unsigned int time_at_last_set_;
};

#endif  // SRC_SYSTEMS_BASE_RLTIMER_H_
