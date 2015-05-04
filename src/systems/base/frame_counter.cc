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

#include "systems/base/frame_counter.h"

#include <cmath>
#include <iostream>

#include "systems/base/event_system.h"

// -----------------------------------------------------------------------
// Frame Counter Base Class
// -----------------------------------------------------------------------

FrameCounter::FrameCounter(EventSystem& event_system,
                           int frame_min,
                           int frame_max,
                           int milliseconds)
    : event_system_(event_system),
      value_(frame_min),
      min_value_(frame_min),
      max_value_(frame_max),
      is_active_(true),
      time_at_start_(event_system.GetTicks()),
      total_time_(milliseconds) {
  BeginTimer();

  if (milliseconds == 0) {
    // Prevent us from dividing by zero (OH SHI-)
    value_ = frame_max;
    is_active_ = false;
  }
}

FrameCounter::~FrameCounter() {
  if (is_active_)
    EndTimer();
}

void FrameCounter::BeginTimer() { is_active_ = true; }

void FrameCounter::EndTimer() { is_active_ = false; }

bool FrameCounter::IsActive() {
  // Read the counter and ignore the result so is_active_ can be updated...
  ReadFrame();

  return is_active_;
}

bool FrameCounter::CheckIfFinished(float new_value) {
  if (max_value_ > min_value_) {
    return new_value >= max_value_;
  } else {
    return new_value <= max_value_;
  }
}

void FrameCounter::UpdateTimeValue(float num_ticks) {
  // Update the value
  if (max_value_ > min_value_)
    value_ += num_ticks;
  else
    value_ -= num_ticks;
}

int FrameCounter::ReadNormalFrameWithChangeInterval(float change_interval,
                                                    float& time_at_last_check) {
  if (is_active_) {
    unsigned int current_time = event_system_.GetTicks();
    float ms_elapsed = current_time - time_at_last_check;
    float num_ticks = ms_elapsed / change_interval;

    UpdateTimeValue(num_ticks);

    // Set the last time checked to the current_time minus the
    // remainder of ms that don't get counted in the ticks incremented
    // this time.
    time_at_last_check = current_time;

    if (CheckIfFinished(value_)) {
      Finished();
    }
  }

  return int(value_);
}

void FrameCounter::Finished() {
  value_ = max_value_;
  EndTimer();
}

// -----------------------------------------------------------------------
// Simple Frame Counter
// -----------------------------------------------------------------------

SimpleFrameCounter::SimpleFrameCounter(EventSystem& es,
                                       int frame_min,
                                       int frame_max,
                                       int milliseconds)
    : FrameCounter(es, frame_min, frame_max, milliseconds),
      time_at_last_check_(es.GetTicks()) {
  change_interval_ = float(milliseconds) / abs(frame_max - frame_min);
}

SimpleFrameCounter::~SimpleFrameCounter() {}

int SimpleFrameCounter::ReadFrame() {
  return ReadNormalFrameWithChangeInterval(change_interval_,
                                           time_at_last_check_);
}

// -----------------------------------------------------------------------
// Loop Frame Counter
// -----------------------------------------------------------------------

LoopFrameCounter::LoopFrameCounter(EventSystem& es,
                                   int frame_min,
                                   int frame_max,
                                   int milliseconds)
    : FrameCounter(es, frame_min, frame_max, milliseconds),
      time_at_last_check_(es.GetTicks()) {
  change_interval_ = float(milliseconds) / abs(frame_max - frame_min);
}

LoopFrameCounter::~LoopFrameCounter() {}

int LoopFrameCounter::ReadFrame() {
  return ReadNormalFrameWithChangeInterval(change_interval_,
                                           time_at_last_check_);
}

void LoopFrameCounter::Finished() {
  // Don't end the timer, simply reset it
  value_ = min_value_;
}

// -----------------------------------------------------------------------
// Turn Frame Counter
// -----------------------------------------------------------------------
TurnFrameCounter::TurnFrameCounter(EventSystem& es,
                                   int frame_min,
                                   int frame_max,
                                   int milliseconds)
    : FrameCounter(es, frame_min, frame_max, milliseconds),
      time_at_last_check_(es.GetTicks()) {
  change_interval_ = int(float(milliseconds) / abs(frame_max - frame_min));
  going_forward_ = frame_max >= frame_min;
}

TurnFrameCounter::~TurnFrameCounter() {}

// @bug This has all the bugs of the old implementation.
int TurnFrameCounter::ReadFrame() {
  std::cerr << "BIG WARNING: TurnFrameCounter::read_frame DOESN'T DO THE SAFE "
            << " THING LIKE ALL OTHER FRAME COUNTERS. FIXME." << std::endl;

  if (is_active_) {
    unsigned int current_time = event_system_.GetTicks();
    unsigned int ms_elapsed = current_time - time_at_last_check_;
    unsigned int time_remainder = ms_elapsed % change_interval_;
    unsigned int num_ticks = ms_elapsed / change_interval_;

    // Update the value
    if (going_forward_)
      value_ += num_ticks;
    else
      value_ -= num_ticks;

    // Set the last time checked to the current_time minus the
    // remainder of ms that don't get counted in the ticks incremented
    // this time.
    time_at_last_check_ = current_time - time_remainder;

    if (value_ >= max_value_) {
      int difference = int(value_ - max_value_);
      value_ = max_value_ - difference;
      going_forward_ = false;
    } else if (value_ < min_value_) {
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
AcceleratingFrameCounter::AcceleratingFrameCounter(EventSystem& es,
                                                   int frame_min,
                                                   int frame_max,
                                                   int milliseconds)
    : FrameCounter(es, frame_min, frame_max, milliseconds),
      start_time_(es.GetTicks()),
      time_at_last_check_(start_time_) {}

AcceleratingFrameCounter::~AcceleratingFrameCounter() {}

int AcceleratingFrameCounter::ReadFrame() {
  if (is_active_) {
    float base_interval = float(total_time_) / abs(max_value_ - min_value_);
    float cur_time =
        (event_system_.GetTicks() - start_time_) / float(total_time_);
    float interval = (1.1f - cur_time * 0.2f) * base_interval;

    return ReadNormalFrameWithChangeInterval(interval, time_at_last_check_);
  }

  return int(value_);
}

// -----------------------------------------------------------------------
// Decelerating Frame Counter
// -----------------------------------------------------------------------
DeceleratingFrameCounter::DeceleratingFrameCounter(EventSystem& es,
                                                   int frame_min,
                                                   int frame_max,
                                                   int milliseconds)
    : FrameCounter(es, frame_min, frame_max, milliseconds),
      start_time_(es.GetTicks()),
      time_at_last_check_(start_time_) {}

DeceleratingFrameCounter::~DeceleratingFrameCounter() {}

int DeceleratingFrameCounter::ReadFrame() {
  if (is_active_) {
    float base_interval = float(total_time_) / abs(max_value_ - min_value_);
    float cur_time =
        (event_system_.GetTicks() - start_time_) / float(total_time_);
    float interval = (0.9f + cur_time * 0.2f) * base_interval;

    return ReadNormalFrameWithChangeInterval(interval, time_at_last_check_);
  }

  return int(value_);
}
