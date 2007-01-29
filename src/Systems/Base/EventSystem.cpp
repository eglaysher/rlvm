
#include "Systems/Base/EventSystem.hpp"
#include "Systems/Base/FrameCounter.hpp"

#include "libReallive/defs.h"

#include <iostream>

using namespace std;
using namespace boost;
using namespace libReallive;

// -----------------------------------------------------------------------

EventSystem::EventSystem() : m_numberOfRealtimeTasks(0) {}

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

// -----------------------------------------------------------------------

bool EventSystem::frameCounterExists(int frameCounter)
{
  if(frameCounter < 0 || frameCounter > 255)
    throw Error("Frame Counter index out of range!");

  scoped_ptr<FrameCounter>& counter = m_frameCounters[frameCounter];
  return counter.get() != NULL;
}

// -----------------------------------------------------------------------

void EventSystem::beginRealtimeTask()
{
//  cerr << "EventSystem::beginRealtimeTask()" << endl;
  m_numberOfRealtimeTasks++;
}

// -----------------------------------------------------------------------

void EventSystem::endRealtimeTask()
{
//  cerr << "EventSystem::endRealtimeTask()" << endl;
  m_numberOfRealtimeTasks--;
}

// -----------------------------------------------------------------------

bool EventSystem::canBeNice()
{
  return m_numberOfRealtimeTasks == 0;
}
