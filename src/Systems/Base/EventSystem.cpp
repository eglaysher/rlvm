
#include "Systems/Base/EventSystem.hpp"
#include "Systems/Base/FrameCounter.hpp"

#include "libReallive/defs.h"

using namespace boost;
using namespace libReallive;

// -----------------------------------------------------------------------

EventSystem::EventSystem() {}

// -----------------------------------------------------------------------

EventSystem::~EventSystem() {}

// -----------------------------------------------------------------------

void EventSystem::setFrameCounter(int frameCounter, FrameCounter* counter)
{
  if(frameCounter < 0 || frameCounter > 255)
    throw Error("Frame Counter index out of range!");

  m_frameCounters[frameCounter].reset(counter);
}

// -----------------------------------------------------------------------

FrameCounter& EventSystem::getFrameCounter(int frameCounter)
{
  if(frameCounter < 0 || frameCounter > 255)
    throw Error("Frame Counter index out of range!");

  scoped_ptr<FrameCounter>& counter = m_frameCounters[frameCounter];
  if(counter.get() == NULL)
    throw Error("Trying to get an uninitialized frame counter!");

  return *counter;
}
