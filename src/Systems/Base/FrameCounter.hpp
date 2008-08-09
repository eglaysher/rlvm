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

#ifndef __FrameCounter_hpp__
#define __FrameCounter_hpp__

class EventSystem;

/**
 * Frame counter used by RealLive code to ensure events happen at a
 * constant speed. Constant to all System implementations since it is
 * implemented in terms of operations on System classes.a
 *
 * See section "5.13.3 Frame counters" of the RLdev specification for
 * more details.
 */
class FrameCounter
{
public:
  FrameCounter(EventSystem& es, int frameMin, int frameMax, int milliseconds);

  virtual ~FrameCounter();

  /**
   * Returns the current value of this frame counter, a value between
   *
   * @return
   */
  virtual int readFrame(EventSystem& eventSystem) = 0;

  /**
   * The converse is setting the value, which should be done after the
   * frame counter has been turned off.
   *
   * @param value New value for the frame counter
   */
  void setValue(int value) { value_ = value; }

  /**
   * When a timer starts, we need to tell the EventSystem that we now
   * have a near realtime event going on and to stop being nice to the
   * operating system.
   *
   * @see endTimer
   */
  void beginTimer(EventSystem& eventSystem);

  /**
   * When a timer ends, there's no need to be so harsh on the
   * system. Tell the eventSystem that we no longer require near
   * realtime event handling.
   *
   * @see beginTimer
   */
  void endTimer(EventSystem& eventSystem);

  bool isActive() const { return is_active_; }
  void setActive(bool active) { is_active_ = active; }

  bool checkIfFinished(float newValue);
  void updateTimeValue(float numTicks);


  int readNormalFrameWithChangeInterval(
    EventSystem& eventSystem, float changeInterval,
    float& timeAtLastCheck);

  /**
   * Called from readNormalFrameWithChangeInterval when finished. This
   * method can be overloaded to control what happens when the timer
   * has reached its end.
   */
  virtual void finished(EventSystem& eventSystem);

// Give these accessors later?
protected:
  EventSystem& event_system_;

  float value_;
  int min_value_;
  int max_value_;
  bool is_active_;

  unsigned int time_at_start_;
  unsigned int total_time_;
};

// -----------------------------------------------------------------------

/**
 * Simple frame counter that counts from frameMin to frameMax.
 */
class SimpleFrameCounter : public FrameCounter
{
public:
  SimpleFrameCounter(EventSystem& es, int frameMin, int frameMax,
                     int milliseconds);

  virtual int readFrame(EventSystem& eventSystem);

private:
  float change_interval_;
  float time_at_last_check_;
};

// -----------------------------------------------------------------------

/**
 * Loop frame counter that counts from frameMin to frameMax, starting over at
 * frameMin.
 */
class LoopFrameCounter : public FrameCounter
{
public:
  LoopFrameCounter(EventSystem& es, int frameMin, int frameMax,
                   int milliseconds);

  virtual int readFrame(EventSystem& eventSystem);
  virtual void finished(EventSystem& eventSystem);

private:
  float change_interval_;
  float time_at_last_check_;
};

// -----------------------------------------------------------------------

/**
 * Turn frame counter that counts from frameMin to frameMax and then counts back
 * down to frameMin.
 */
class TurnFrameCounter : public FrameCounter
{
public:
  TurnFrameCounter(EventSystem& es, int frameMin, int frameMax,
                   int milliseconds);

  virtual int readFrame(EventSystem& eventSystem);

private:
  bool going_forward_;
  unsigned int change_interval_;
  unsigned int time_at_last_check_;
};

// -----------------------------------------------------------------------

/**
 * Frame counter that counts from frameMin to frameMax, speeding up as it goes.
 */
class AcceleratingFrameCounter : public FrameCounter
{
private:
  unsigned int start_time_;
  float time_at_last_check_;

public:
  AcceleratingFrameCounter(EventSystem& es, int frameMin, int frameMax,
                           int milliseconds);

  virtual int readFrame(EventSystem& eventSystem);
};

// -----------------------------------------------------------------------


/**
 * Frame counter that counts from frameMin to frameMax, slowing down as it goes.
 */
class DeceleratingFrameCounter : public FrameCounter
{
private:
  unsigned int start_time_;
  float time_at_last_check_;

public:
  DeceleratingFrameCounter(EventSystem& es, int frameMin, int frameMax,
                           int milliseconds);

  virtual int readFrame(EventSystem& eventSystem);
};


#endif
