// This file is part of RLVM, a RealLive virutal machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006 El Riot
//  
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
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


#include "Systems/Base/FrameCounter.hpp"
#include "Systems/Base/EventSystem.hpp"

#include <cmath>
#include <iostream>
using namespace std;

// -----------------------------------------------------------------------
// Frame Counter Base Class
// -----------------------------------------------------------------------

FrameCounter::FrameCounter(EventSystem& eventSystem, int frameMin,
                           int frameMax, int milliseconds)
  : m_eventSystem(eventSystem), m_value(frameMin), m_minValue(frameMin), 
    m_maxValue(frameMax), m_isActive(true), 
    m_timeAtStart(eventSystem.getTicks()),
    m_totalTime(milliseconds)
{
  beginTimer(eventSystem);
}

// -----------------------------------------------------------------------

FrameCounter::~FrameCounter()
{
  if(m_isActive)
    endTimer(m_eventSystem);
}

// -----------------------------------------------------------------------

void FrameCounter::beginTimer(EventSystem& eventSystem)
{
  eventSystem.beginRealtimeTask();
  m_isActive = true;
}

// -----------------------------------------------------------------------

void FrameCounter::endTimer(EventSystem& eventSystem)
{
  if(m_isActive)
    eventSystem.endRealtimeTask();

  m_isActive = false;
}

// -----------------------------------------------------------------------
// Simpe Frame Counter
// -----------------------------------------------------------------------

SimpleFrameCounter::SimpleFrameCounter(
  EventSystem& es, int frameMin, int frameMax, int milliseconds)
  : FrameCounter(es, frameMin, frameMax, milliseconds),
    m_changeInterval((unsigned int)(float(milliseconds) / (frameMax - frameMin))),
    m_timeAtLastCheck(es.getTicks())
{
}

// -----------------------------------------------------------------------

int SimpleFrameCounter::readFrame(EventSystem& eventSystem) {
  if(m_isActive)
  {
    unsigned int currentTime = eventSystem.getTicks();
    float currentRate = pow(float(currentTime - m_timeAtStart) / m_totalTime, 2);
    if(currentRate >= 1.0f)
    {
      m_value = m_maxValue;
      endTimer(eventSystem);
    }
    else
      m_value = m_minValue + ((m_maxValue - m_minValue) * currentRate);


//     unsigned int currentTime = eventSystem.getTicks();
//     unsigned int msElapsed = currentTime - m_timeAtLastCheck;
//     unsigned int timeRemainder = msElapsed % m_changeInterval;
//     unsigned int numTicks = msElapsed / m_changeInterval;

//     // Update the value 
//     m_value += numTicks;

//     // Set the last time checked to the currentTime minus the
//     // remainder of ms that don't get counted in the ticks incremented
//     // this time.
//     m_timeAtLastCheck = currentTime - timeRemainder;

//     if(m_value >= m_maxValue)
//     {
//       m_value = m_maxValue;
//       endTimer(eventSystem);
//     }
  }

  return m_value;
}


// -----------------------------------------------------------------------
// Loop Frame Counter
// -----------------------------------------------------------------------

LoopFrameCounter::LoopFrameCounter(
  EventSystem& es, int frameMin, int frameMax, int milliseconds)
  : FrameCounter(es, frameMin, frameMax, milliseconds),
    m_changeInterval((unsigned int)(float(milliseconds) / (frameMax - frameMin))),
    m_timeAtLastCheck(es.getTicks())
{
}

// -----------------------------------------------------------------------

int LoopFrameCounter::readFrame(EventSystem& eventSystem) {
  if(m_isActive)
  {
    unsigned int currentTime = eventSystem.getTicks();
    unsigned int msElapsed = currentTime - m_timeAtLastCheck;
    unsigned int timeRemainder = msElapsed % m_changeInterval;
    unsigned int numTicks = msElapsed / m_changeInterval;

    // Update the value 
    m_value += numTicks;

    // Set the last time checked to the currentTime minus the
    // remainder of ms that don't get counted in the ticks incremented
    // this time.
    m_timeAtLastCheck = currentTime - timeRemainder;

    if(m_value > m_maxValue)
    {
      // Don't end the timer, simply reset it
      m_value = m_minValue;
    }
  }

  return m_value;
}

// -----------------------------------------------------------------------
// Turn Frame Counter
// -----------------------------------------------------------------------
TurnFrameCounter::TurnFrameCounter(
  EventSystem& es, int frameMin, int frameMax, int milliseconds)
  : FrameCounter(es, frameMin, frameMax, milliseconds),
    m_goingForward(true),
    m_changeInterval((unsigned int)(float(milliseconds) / (frameMax - frameMin))),
    m_timeAtLastCheck(es.getTicks())
{
}

// -----------------------------------------------------------------------

int TurnFrameCounter::readFrame(EventSystem& eventSystem) {
  if(m_isActive)
  {
    unsigned int currentTime = eventSystem.getTicks();
    unsigned int msElapsed = currentTime - m_timeAtLastCheck;
    unsigned int timeRemainder = msElapsed % m_changeInterval;
    unsigned int numTicks = msElapsed / m_changeInterval;

    // Update the value
    if(m_goingForward)
      m_value += numTicks;
    else
      m_value -= numTicks;

    // Set the last time checked to the currentTime minus the
    // remainder of ms that don't get counted in the ticks incremented
    // this time.
    m_timeAtLastCheck = currentTime - timeRemainder;

    if(m_value >= m_maxValue)
    {
      int difference = m_maxValue - m_value;
      m_value = m_maxValue - difference;
      m_goingForward = false;
    }
    else if(m_value < m_minValue)
    {
      int difference = m_minValue - m_value;
      m_value = m_minValue + difference;
      m_goingForward = true;
    }
  }

  return m_value;
}


// -----------------------------------------------------------------------
// Accelerating Frame Counter
// -----------------------------------------------------------------------
AcceleratingFrameCounter::AcceleratingFrameCounter(
  EventSystem& es, int frameMin, int frameMax, int milliseconds)
  : FrameCounter(es, frameMin, frameMax, milliseconds)
{
}

// -----------------------------------------------------------------------

int AcceleratingFrameCounter::readFrame(EventSystem& eventSystem) {
  if(m_isActive)
  {
    unsigned int currentTime = eventSystem.getTicks();
    float currentRate = pow(float(currentTime - m_timeAtStart) / m_totalTime, 2);
    m_value = m_minValue + ((m_maxValue - m_minValue) * currentRate);

    if(m_value >= m_maxValue)
    {
      m_value = m_maxValue;
      endTimer(eventSystem);
    }
  }

  return m_value;
}

// -----------------------------------------------------------------------


// -----------------------------------------------------------------------
// Decelerating Frame Counter
// -----------------------------------------------------------------------
DeceleratingFrameCounter::DeceleratingFrameCounter(
  EventSystem& es, int frameMin, int frameMax, int milliseconds)
  : FrameCounter(es, frameMin, frameMax, milliseconds)
{
}

// -----------------------------------------------------------------------

/**
 * @bug Sometimes this screws up, will turn around and go in the total
 *      other direction.
 */
int DeceleratingFrameCounter::readFrame(EventSystem& eventSystem) {
  if(m_isActive)
  {
    unsigned int currentTime = eventSystem.getTicks();
    float currentRate = 1 - pow((float(currentTime - m_timeAtStart) / m_totalTime) - 1, 2);
    m_value = m_minValue + ((m_maxValue - m_minValue) * currentRate);

    if(m_value >= m_maxValue)
    {
      m_value = m_maxValue;
      endTimer(eventSystem);
    }
  }

  return m_value;
}

// -----------------------------------------------------------------------

