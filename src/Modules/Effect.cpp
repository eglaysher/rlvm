
#include "Modules/Effect.hpp"  
#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/EventSystem.hpp"

#include <iostream>

using namespace std;

// -----------------------------------------------------------------------

Effect::Effect(RLMachine& machine, int x, int y, int width, 
               int height, int dx, int dy, int time)
  : m_x(x), m_y(y), m_width(width), m_height(height), m_dx(dx), 
    m_dy(dy), m_duration(time), 
    m_startTime(machine.system().event().getTicks()),
    m_dc1(machine.system().graphics().getDC(1)),
    m_originalDC0(machine.system().graphics().getDC(0).clone())
{
  
}

// -----------------------------------------------------------------------

bool Effect::operator()(RLMachine& machine)
{
  GraphicsSystem& graphics = machine.system().graphics();
  unsigned int time = machine.system().event().getTicks();
  unsigned int currentFrame = time - m_startTime;

  cout << currentFrame << "/" << m_duration << endl;

  // Blit the portional of the original DC0 image onto DC0, so we
  // make sure we have the correct original state. We do this when we
  // are instructed to. We also do it ALWAYS on the last frame.
  if(blitOriginalImage() || currentFrame >= m_duration)
  {
    graphics.blitSurfaceToDC(*m_originalDC0, 0,
                             m_x, m_y, m_width, m_height,
                             m_dx, m_dy, m_width, m_height);
  }

  if(currentFrame < m_duration)
  {
    performEffectForTime(machine, currentFrame);
    return false;
  }
  else
  {
    // Blit DC1 onto DC0, with full opacity, and end the operation
    graphics.blitSurfaceToDC(dc1(), 0,
                             m_x, m_y, m_width, m_height,
                             m_dx, m_dy, m_width, m_height);
    return true;
  }
}
