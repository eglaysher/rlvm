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

#include <cmath>
#include <iostream>

using namespace std;

// -----------------------------------------------------------------------
// Frame Counter Base Class
// -----------------------------------------------------------------------

FrameCounter::FrameCounter(EventSystem& event_system, int frame_min,
                           int frame_max, int milliseconds)
  : event_system_(event_system), value_(frame_min), min_value_(frame_min),
    max_value_(frame_max), is_active_(true),
    time_at_start_(event_system.getTicks()),
    total_time_(milliseconds)
{
  beginTimer();
}

// -----------------------------------------------------------------------

FrameCounter::~FrameCounter()
{
  if(is_active_)
    endTimer();
}

// -----------------------------------------------------------------------

void FrameCounter::beginTimer()
{
  is_active_ = true;
}

// -----------------------------------------------------------------------

void FrameCounter::endTimer()
{
  is_active_ = false;
}

// -----------------------------------------------------------------------

bool FrameCounter::isActive() {
  // Read the counter and ignore the result so is_active_ can be updated...
  readFrame();

  return is_active_;
}

// -----------------------------------------------------------------------

bool FrameCounter::checkIfFinished(float new_value)
{
  if(max_value_ > min_value_)
  {
    return new_value >= max_value_;
  }
  else
  {
    return new_value <= max_value_;
  }
}

// -----------------------------------------------------------------------

void FrameCounter::updateTimeValue(float num_ticks)
{
  // Update the value
  if(max_value_ > min_value_)
    value_ += num_ticks;
  else
    value_ -= num_ticks;
}

// -----------------------------------------------------------------------

int FrameCounter::readNormalFrameWithChangeInterval(
  float change_interval, float& time_at_last_check)
{
  if(is_active_)
  {
    unsigned int current_time = event_system_.getTicks();
    float ms_elapsed = current_time - time_at_last_check;
    float num_ticks = ms_elapsed / change_interval;

    updateTimeValue(num_ticks);

    // Set the last time checked to the current_time minus the
    // remainder of ms that don't get counted in the ticks incremented
    // this time.
    time_at_last_check = current_time;

    if(checkIfFinished(value_))
    {
      finished();
    }
  }

  return int(value_);
}

// -----------------------------------------------------------------------

void FrameCounter::finished()
{
  value_ = max_value_;
  endTimer();
}

// -----------------------------------------------------------------------
// Simpe Frame Counter
// -----------------------------------------------------------------------

SimpleFrameCounter::SimpleFrameCounter(
  EventSystem& es, int frame_min, int frame_max, int milliseconds)
  : FrameCounter(es, frame_min, frame_max, milliseconds),
    time_at_last_check_(es.getTicks())
{
  change_interval_ = float(milliseconds) / abs(frame_max - frame_min);
}

// -----------------------------------------------------------------------

int SimpleFrameCounter::readFrame()
{
  return readNormalFrameWithChangeInterval(change_interval_,
                                           time_at_last_check_);
}

// -----------------------------------------------------------------------
// Loop Frame Counter
// -----------------------------------------------------------------------

LoopFrameCounter::LoopFrameCounter(
  EventSystem& es, int frame_min, int frame_max, int milliseconds)
  : FrameCounter(es, frame_min, frame_max, milliseconds),
    time_at_last_check_(es.getTicks())
{
  change_interval_ = float(milliseconds) / abs(frame_max - frame_min);
}

// -----------------------------------------------------------------------

int LoopFrameCounter::readFrame()
{
  return readNormalFrameWithChangeInterval(change_interval_,
                                           time_at_last_check_);
}

// -----------------------------------------------------------------------

void LoopFrameCounter::finished()
{
  // Don't end the timer, simply reset it
  value_ = min_value_;
}

// -----------------------------------------------------------------------
// Turn Frame Counter
// -----------------------------------------------------------------------
TurnFrameCounter::TurnFrameCounter(
  EventSystem& es, int frame_min, int frame_max, int milliseconds)
  : FrameCounter(es, frame_min, frame_max, milliseconds),
    time_at_last_check_(es.getTicks())
{
  change_interval_ = int(float(milliseconds) / abs(frame_max - frame_min));
  going_forward_ = frame_max >= frame_min;
}

// -----------------------------------------------------------------------

/// @bug This has all the bugs of the old implementation.
int TurnFrameCounter::readFrame()
{
  cerr << "BIG WARNING: TurnFrameCounter::read_frame DOESN'T DO THE SAFE "
       << " THING LIKE ALL OTHER FRAME COUNTERS. FIXME." << endl;

  if(is_active_)
  {
    unsigned int current_time = event_system_.getTicks();
    unsigned int ms_elapsed = current_time - time_at_last_check_;
    unsigned int time_remainder = ms_elapsed % change_interval_;
    unsigned int num_ticks = ms_elapsed / change_interval_;

    // Update the value
    if(going_forward_)
      value_ += num_ticks;
    else
      value_ -= num_ticks;

    // Set the last time checked to the current_time minus the
    // remainder of ms that don't get counted in the ticks incremented
    // this time.
    time_at_last_check_ = current_time - time_remainder;

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
  EventSystem& es, int frame_min, int frame_max, int milliseconds)
  : FrameCounter(es, frame_min, frame_max, milliseconds),
    start_time_(es.getTicks()),
    time_at_last_check_(start_time_)
{}

// -----------------------------------------------------------------------

int AcceleratingFrameCounter::readFrame()
{
  if(is_active_)
  {
    float base_interval = float(total_time_) / abs(max_value_ - min_value_);
    float cur_time = (event_system_.getTicks() - start_time_) / float(total_time_);
    float interval = (1.1f - cur_time*0.2f) * base_interval;

    return readNormalFrameWithChangeInterval(interval, time_at_last_check_);
  }

  return int(value_);
}

// -----------------------------------------------------------------------
// Decelerating Frame Counter
// -----------------------------------------------------------------------
DeceleratingFrameCounter::DeceleratingFrameCounter(
  EventSystem& es, int frame_min, int frame_max, int milliseconds)
  : FrameCounter(es, frame_min, frame_max, milliseconds),
    start_time_(es.getTicks()),
    time_at_last_check_(start_time_)
{}

// -----------------------------------------------------------------------

int DeceleratingFrameCounter::readFrame()
{
  if(is_active_)
  {
    float base_interval = float(total_time_) / abs(max_value_ - min_value_);
    float cur_time = (event_system_.getTicks() - start_time_) / float(total_time_);
    float interval = (0.9f + cur_time*0.2f) * base_interval;

    return readNormalFrameWithChangeInterval(interval, time_at_last_check_);
  }

  return int(value_);
}

// -----------------------------------------------------------------------

