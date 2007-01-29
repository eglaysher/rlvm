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

Effect::Effect(RLMachine& machine, int width, int height, int time)
  : m_width(width), m_height(height), m_duration(time), 
    m_startTime(machine.system().event().getTicks()),
    m_machine(machine)
{
  m_machine.system().event().beginRealtimeTask();
}

// -----------------------------------------------------------------------

Effect::~Effect()
{
  m_machine.system().event().endRealtimeTask();
}

// -----------------------------------------------------------------------

/// @todo Riht now, the ctrl pressed behaviour *may* not match
///       RealLive exactly. Verify this.
bool Effect::operator()(RLMachine& machine)
{
  GraphicsSystem& graphics = machine.system().graphics();
  unsigned int time = machine.system().event().getTicks();
  unsigned int currentFrame = time - m_startTime;

  bool ctrlPressed = machine.system().event().ctrlPressed();

  if(currentFrame >= m_duration || ctrlPressed)
  {
    // Blit DC1 onto DC0, with full opacity, and end the operation
    graphics.getDC(1).blitToSurface(graphics.getDC(0),
                                    0, 0, m_width, m_height,
                                    0, 0, m_width, m_height, 255);
    return true;
  }
  else
  {
    // Render to the screen
    GraphicsSystem& graphics = machine.system().graphics();
    graphics.beginFrame();

    if(blitOriginalImage())
    {
      graphics.getDC(0).
        renderToScreen(0, 0, width(), height(),
                       0, 0, width(), height(),
                       255);
    }

    performEffectForTime(machine, currentFrame);

    graphics.endFrame();
    return false;
  }
}
