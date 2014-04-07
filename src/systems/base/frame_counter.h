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

#ifndef SRC_SYSTEMS_BASE_FRAME_COUNTER_H_
#define SRC_SYSTEMS_BASE_FRAME_COUNTER_H_

class EventSystem;

// Frame counter used by RealLive code to ensure events happen at a constant
// speed. Constant to all System implementations since it is implemented in
// terms of operations on System classes.a
//
// See section "5.13.3 Frame counters" of the RLdev specification for more
// details.
class FrameCounter {
 public:
  FrameCounter(EventSystem& es, int frame_min, int frame_max, int milliseconds);

  virtual ~FrameCounter();

  // Returns the current value of this frame counter, a value between
  virtual int ReadFrame() = 0;

  // The converse is setting the value, which should be done after the
  // frame counter has been turned off.
  void set_value(int value) { value_ = value; }

  // When a timer starts, we need to tell the EventSystem that we now
  // have a near realtime event going on and to stop being nice to the
  // operating system.
  void BeginTimer();

  // When a timer ends, there's no need to be so harsh on the
  // system. Tell the event_system that we no longer require near
  // realtime event handling.
  void EndTimer();

  bool IsActive();
  void set_active(bool active) { is_active_ = active; }

  bool CheckIfFinished(float new_value);
  void UpdateTimeValue(float num_ticks);

 protected:
  // Implementation of ReadFrame(). Called by most subclasses with their own
  // data members.
  int ReadNormalFrameWithChangeInterval(float change_interval,
                                        float& time_at_last_check);

  // Called from read_normal_frame_with_change_interval when finished. This
  // method can be overloaded to control what happens when the timer
  // has reached its end.
  virtual void Finished();

  EventSystem& event_system_;

  float value_;
  int min_value_;
  int max_value_;
  bool is_active_;

  unsigned int time_at_start_;
  unsigned int total_time_;
};

// Simple frame counter that counts from frame_min to frame_max.
class SimpleFrameCounter : public FrameCounter {
 public:
  SimpleFrameCounter(EventSystem& es,
                     int frame_min,
                     int frame_max,
                     int milliseconds);
  virtual ~SimpleFrameCounter();

  virtual int ReadFrame() override;

 private:
  float change_interval_;
  float time_at_last_check_;
};

// Loop frame counter that counts from frame_min to frame_max, starting over at
// frame_min.
class LoopFrameCounter : public FrameCounter {
 public:
  LoopFrameCounter(EventSystem& es,
                   int frame_min,
                   int frame_max,
                   int milliseconds);
  virtual ~LoopFrameCounter();

  virtual int ReadFrame() override;
  virtual void Finished() override;

 private:
  float change_interval_;
  float time_at_last_check_;
};

// Turn frame counter that counts from frame_min to frame_max and then counts
// back down to frame_min.
class TurnFrameCounter : public FrameCounter {
 public:
  TurnFrameCounter(EventSystem& es,
                   int frame_min,
                   int frame_max,
                   int milliseconds);
  virtual ~TurnFrameCounter();

  virtual int ReadFrame() override;

 private:
  bool going_forward_;
  unsigned int change_interval_;
  unsigned int time_at_last_check_;
};

// Frame counter that counts from frame_min to frame_max, speeding up as it
// goes.
class AcceleratingFrameCounter : public FrameCounter {
 public:
  AcceleratingFrameCounter(EventSystem& es,
                           int frame_min,
                           int frame_max,
                           int milliseconds);
  virtual ~AcceleratingFrameCounter();

  virtual int ReadFrame() override;

 private:
  unsigned int start_time_;
  float time_at_last_check_;
};

// Frame counter that counts from frame_min to frame_max, slowing down as it
// goes.
class DeceleratingFrameCounter : public FrameCounter {
 public:
  DeceleratingFrameCounter(EventSystem& es,
                           int frame_min,
                           int frame_max,
                           int milliseconds);
  virtual ~DeceleratingFrameCounter();

  virtual int ReadFrame() override;

 private:
  unsigned int start_time_;
  float time_at_last_check_;
};

#endif  // SRC_SYSTEMS_BASE_FRAME_COUNTER_H_
