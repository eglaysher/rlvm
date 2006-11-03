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

/**
 * @file   Effect.cpp
 * @author Elliot Glaysher
 * @date   Thu Nov  2 20:35:54 2006
 * 
 * @brief  Base LongOperation for all transition effects on DCs.
 */

#include "Modules/Effect.hpp"  
#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/System.hpp"
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
