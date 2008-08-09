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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// -----------------------------------------------------------------------

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

#include "Systems/Base/FrameCounter.hpp"
#include "Systems/Base/EventSystem.hpp"

#include "Utilities.h"

#include <cmath>
#include <iostream>
using namespace std;

// -----------------------------------------------------------------------
// Frame Counter Base Class
// -----------------------------------------------------------------------

FrameCounter::FrameCounter(EventSystem& eventSystem, int frameMin,
                           int frameMax, int milliseconds)
  : event_system_(eventSystem), value_(frameMin), min_value_(frameMin),
    max_value_(frameMax), is_active_(true),
    time_at_start_(eventSystem.getTicks()),
    total_time_(milliseconds)
{
  beginTimer(eventSystem);
}

// -----------------------------------------------------------------------

FrameCounter::~FrameCounter()
{
  if(is_active_)
    endTimer(event_system_);
}

// -----------------------------------------------------------------------

void FrameCounter::beginTimer(EventSystem& eventSystem)
{
  is_active_ = true;
}

// -----------------------------------------------------------------------

void FrameCounter::endTimer(EventSystem& eventSystem)
{
  is_active_ = false;
}

// -----------------------------------------------------------------------

bool FrameCounter::checkIfFinished(float newValue)
{
  if(max_value_ > min_value_)
  {
    return newValue >= max_value_;
  }
  else
  {
    return newValue <= max_value_;
  }
}

// -----------------------------------------------------------------------

void FrameCounter::updateTimeValue(float numTicks)
{
  // Update the value
  if(max_value_ > min_value_)
    value_ += numTicks;
  else
    value_ -= numTicks;
}

// -----------------------------------------------------------------------

int FrameCounter::readNormalFrameWithChangeInterval(
  EventSystem& eventSystem, float changeInterval,
  float& timeAtLastCheck)
{
  if(is_active_)
  {
    unsigned int currentTime = eventSystem.getTicks();
    float msElapsed = currentTime - timeAtLastCheck;
    float numTicks = msElapsed / changeInterval;

    updateTimeValue(numTicks);

    // Set the last time checked to the currentTime minus the
    // remainder of ms that don't get counted in the ticks incremented
    // this time.
    timeAtLastCheck = currentTime;

    if(checkIfFinished(value_))
    {
      finished(eventSystem);
    }
  }

  return int(value_);
}

// -----------------------------------------------------------------------

void FrameCounter::finished(EventSystem& eventSystem)
{
  value_ = max_value_;
  endTimer(eventSystem);
}

// -----------------------------------------------------------------------
// Simpe Frame Counter
// -----------------------------------------------------------------------

SimpleFrameCounter::SimpleFrameCounter(
  EventSystem& es, int frameMin, int frameMax, int milliseconds)
  : FrameCounter(es, frameMin, frameMax, milliseconds),
    time_at_last_check_(es.getTicks())
{
  change_interval_ = float(milliseconds) / abs(frameMax - frameMin);
}

// -----------------------------------------------------------------------

int SimpleFrameCounter::readFrame(EventSystem& eventSystem)
{
  return readNormalFrameWithChangeInterval(eventSystem, change_interval_,
                                           time_at_last_check_);
}

// -----------------------------------------------------------------------
// Loop Frame Counter
// -----------------------------------------------------------------------

LoopFrameCounter::LoopFrameCounter(
  EventSystem& es, int frameMin, int frameMax, int milliseconds)
  : FrameCounter(es, frameMin, frameMax, milliseconds),
    time_at_last_check_(es.getTicks())
{
  change_interval_ = float(milliseconds) / abs(frameMax - frameMin);
}

// -----------------------------------------------------------------------

int LoopFrameCounter::readFrame(EventSystem& eventSystem)
{
  return readNormalFrameWithChangeInterval(eventSystem, change_interval_,
                                           time_at_last_check_);
}

// -----------------------------------------------------------------------

void LoopFrameCounter::finished(EventSystem& eventSystem)
{
  // Don't end the timer, simply reset it
  value_ = min_value_;
}

// -----------------------------------------------------------------------
// Turn Frame Counter
// -----------------------------------------------------------------------
TurnFrameCounter::TurnFrameCounter(
  EventSystem& es, int frameMin, int frameMax, int milliseconds)
  : FrameCounter(es, frameMin, frameMax, milliseconds),
    time_at_last_check_(es.getTicks())
{
  change_interval_ = int(float(milliseconds) / abs(frameMax - frameMin));
  going_forward_ = frameMax >= frameMin;
}

// -----------------------------------------------------------------------

/// @bug This has all the bugs of the old implementation.
int TurnFrameCounter::readFrame(EventSystem& eventSystem)
{
  cerr << "BIG WARNING: TurnFrameCounter::readFrame DOESN'T DO THE SAFE "
       << " THING LIKE ALL OTHER FRAME COUNTERS. FIXME." << endl;

  if(is_active_)
  {
    unsigned int currentTime = eventSystem.getTicks();
    unsigned int msElapsed = currentTime - time_at_last_check_;
    unsigned int timeRemainder = msElapsed % change_interval_;
    unsigned int numTicks = msElapsed / change_interval_;

    // Update the value
    if(going_forward_)
      value_ += numTicks;
    else
      value_ -= numTicks;

    // Set the last time checked to the currentTime minus the
    // remainder of ms that don't get counted in the ticks incremented
    // this time.
    time_at_last_check_ = currentTime - timeRemainder;

    if(value_ >= max_value_)
    {
      int difference = int(value_ - max_value_);
      value_ = max_value_ - difference;
      going_forward_ = false;
    }
    else if(value_ < min_value_)
    {
      int difference = int(min_value_ - value_);
      value_ = min_value_ + difference;
      going_forward_ = true;
    }
  }

  return int(value_);
}

// -----------------------------------------------------------------------
// Accelerating Frame Counter
// -----------------------------------------------------------------------
AcceleratingFrameCounter::AcceleratingFrameCounter(
  EventSystem& es, int frameMin, int frameMax, int milliseconds)
  : FrameCounter(es, frameMin, frameMax, milliseconds),
    start_time_(es.getTicks()),
    time_at_last_check_(start_time_)
{}

// -----------------------------------------------------------------------

int AcceleratingFrameCounter::readFrame(EventSystem& eventSystem)
{
  if(is_active_)
  {
    float baseInterval = float(total_time_) / abs(max_value_ - min_value_);
    float curTime = (eventSystem.getTicks() - start_time_) / float(total_time_);
    float interval = (1.1f - curTime*0.2f) * baseInterval;

    return readNormalFrameWithChangeInterval(eventSystem, interval,
                                             time_at_last_check_);
  }

  return int(value_);
}

// -----------------------------------------------------------------------
// Decelerating Frame Counter
// -----------------------------------------------------------------------
DeceleratingFrameCounter::DeceleratingFrameCounter(
  EventSystem& es, int frameMin, int frameMax, int milliseconds)
  : FrameCounter(es, frameMin, frameMax, milliseconds),
    start_time_(es.getTicks()),
    time_at_last_check_(start_time_)
{}

// -----------------------------------------------------------------------

int DeceleratingFrameCounter::readFrame(EventSystem& eventSystem)
{
  if(is_active_)
  {
    float baseInterval = float(total_time_) / abs(max_value_ - min_value_);
    float curTime = (eventSystem.getTicks() - start_time_) / float(total_time_);
    float interval = (0.9f + curTime*0.2f) * baseInterval;

    return readNormalFrameWithChangeInterval(eventSystem, interval,
                                             time_at_last_check_);
  }

  return int(value_);
}

// -----------------------------------------------------------------------

