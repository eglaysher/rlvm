// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2007 Elliot Glaysher
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

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

#include "ZoomLongOperation.hpp"

#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/EventSystem.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/Surface.hpp"

#include <iostream>

using std::cerr;
using std::endl;

// -----------------------------------------------------------------------
// ZoomLongOperation
// -----------------------------------------------------------------------

ZoomLongOperation::ZoomLongOperation(
  RLMachine& machine,
  const boost::shared_ptr<Surface>& origSurface,
  const boost::shared_ptr<Surface>& srcSurface,
  const int fx, const int fy, const int fwidth, const int fheight, 
  const int tx, const int ty, const int twidth, const int theight,
  const int dx, const int dy, const int dwidth, 
  const int dheight,  const int time)
  : m_machine(machine), m_origSurface(origSurface), 
    m_srcSurface(srcSurface),
    m_fx(fx), m_fy(fy), m_fwidth(fwidth), m_fheight(fheight), 
    m_tx(tx), m_ty(ty), m_twidth(twidth), m_theight(theight),
    m_dx(dx), m_dy(dy), m_dwidth(dwidth), 
    m_dheight(dheight),  m_duration(time),
    m_startTime(machine.system().event().getTicks())
{
  m_machine.system().event().beginRealtimeTask();
}

// -----------------------------------------------------------------------

ZoomLongOperation::~ZoomLongOperation()
{
  m_machine.system().event().endRealtimeTask();
}

// -----------------------------------------------------------------------

bool ZoomLongOperation::operator()(RLMachine& machine)
{
  unsigned int time = machine.system().event().getTicks();
  unsigned int currentFrame = time - m_startTime;

  bool ctrlPressed = machine.system().event().ctrlPressed();

  if(currentFrame >= m_duration || ctrlPressed)
  {
    return true;
  }
  else
  {
    // Render to the screen
    GraphicsSystem& graphics = machine.system().graphics();
    graphics.beginFrame();

    // First blit the original dc0 to the screen
    m_origSurface->
      renderToScreen(0, 0, m_origSurface->width(), m_origSurface->height(),
                     0, 0, m_origSurface->width(), m_origSurface->height(),
                     255);

    // figure out the new coordinates for the zoom.
    float ratio = currentFrame / float(m_duration);
    int zX = m_fx + (ratio * (m_tx - m_fx));
    int zY = m_fy + (ratio * (m_ty - m_fy));
    int zWidth = m_fwidth + (ratio * (m_twidth - m_fwidth));
    int zHeight = m_fheight + (ratio * (m_theight - m_fheight));

    m_srcSurface->
      renderToScreen(zX, zY, zX + zWidth, zY + zHeight,
                     m_dx, m_dy, m_dx + m_dwidth, m_dy + m_dheight,
                     255);

    graphics.endFrame();
    return false;
  }
}



