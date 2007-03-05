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
 * @file   FadeEffect.cpp
 * @author Elliot Glaysher
 * @date   Tue Nov 14 20:58:10 2006
 * 
 * @brief  Fade transition Effect
 */

#include "FadeEffect.hpp"
#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/Surface.hpp"

#include <iostream>

using namespace std;

// -----------------------------------------------------------------------

bool FadeEffect::blitOriginalImage() const
{
  return true; 
}

// -----------------------------------------------------------------------

void FadeEffect::performEffectForTime(RLMachine& machine,
                                      int currentTime)
{
  // Blit the source image to the screen with the opacity
  int opacity = int((float(currentTime) / duration()) * 255);

  GraphicsSystem& graphics = machine.system().graphics();
  srcSurface().
    renderToScreen(0, 0, width(), height(), 
                   0, 0, width(), height(),
                   opacity);
}

// -----------------------------------------------------------------------

FadeEffect::FadeEffect(RLMachine& machine, boost::shared_ptr<Surface> src, 
                       boost::shared_ptr<Surface> dst,
                       int width, int height, 
                       int time)
  : Effect(machine, src, dst, width, height, time)
{
}
