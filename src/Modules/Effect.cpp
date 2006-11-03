
#include "Modules/Effect.hpp"  
#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/EventSystem.hpp"

#include <iostream>

using namespace std;

// -----------------------------------------------------------------------

Effect::Effect(RLMachine& machine, int x, int y, int width, 
               int height, int dx, int dy, int time)
  : m_x(x), m_y(y), m_width(width + 1), m_height(height + 1), m_dx(dx), 
    m_dy(dy), m_duration(time), 
    m_startTime(machine.system().event().getTicks())
{
  
}

// -----------------------------------------------------------------------

bool Effect::operator()(RLMachine& machine)
{
  GraphicsSystem& graphics = machine.system().graphics();
  unsigned int time = machine.system().event().getTicks();
  unsigned int currentFrame = time - m_startTime;

//  cout << currentFrame << "/" << m_duration << endl;

  if(currentFrame < m_duration)
  {
    performEffectForTime(machine, currentFrame);
    return false;
  }
  else
  {
    // Blit DC1 onto DC0, with full opacity, and end the operation
    graphics.getDC(1).blitToSurface(graphics.getDC(0),
                                    m_x, m_y, m_width, m_height,
                                    m_dx, m_dy, m_width, m_height, 255);
    return true;
  }
}
