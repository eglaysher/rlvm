
#include "Systems/Base/FrameCounter.hpp"
#include "Systems/Base/EventSystem.hpp"

#include <iostream>
using namespace std;

FrameCounter::FrameCounter(EventSystem& eventSystem, int frameMin,
                           int frameMax, int milliseconds)
  : m_value(frameMin), m_minValue(frameMin), m_maxValue(frameMax),
    m_isActive(true),
    m_changeInterval(float(milliseconds) / (frameMax - frameMin)),
    m_timeAtLastCheck(eventSystem.getTicks())
{
}
 
// -----------------------------------------------------------------------

int FrameCounter::readFrame(EventSystem& eventSystem) {
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
      m_value = m_maxValue;
      m_isActive = false;
    }
  }

  return m_value;
}
