// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2008 Elliot Glaysher
//  
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
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

#include "Systems/Base/MouseCursor.hpp"
#include "Systems/Base/Surface.hpp"

const int CURSOR_X_OFFSET = 8;
const int CURSOR_Y_OFFSET = 8;
const int CURSOR_SIZE = 32;
const int HOTSPOTMASK_X_OFFSET = 8;
const int HOTSPOTMASK_Y_OFFSET = 48;

using namespace std;

// -----------------------------------------------------------------------
// MouseCursor (public)
// -----------------------------------------------------------------------
MouseCursor::MouseCursor(const boost::shared_ptr<Surface>& cursorSurface)
  : m_cursorSurface(cursorSurface)
{
  findHotspot();

  int alphaR, alphaG, alphaB;
  cursorSurface->getDCPixel(0, 0, alphaR, alphaG, alphaB);

  m_cursorSurface = 
    cursorSurface->clipAsColorMask(
      CURSOR_X_OFFSET, CURSOR_Y_OFFSET,
      CURSOR_X_OFFSET + CURSOR_SIZE, CURSOR_Y_OFFSET + CURSOR_SIZE,
      alphaR, alphaG, alphaB);
}

// -----------------------------------------------------------------------

MouseCursor::~MouseCursor() {}

// -----------------------------------------------------------------------

void MouseCursor::renderHotspotAt(RLMachine& machine, 
                                  const Point& mouseLocation)
{
  Point render = getTopLeftForHotspotAt(mouseLocation);

  m_cursorSurface->renderToScreen(
    0, 0, CURSOR_SIZE, CURSOR_SIZE,
    render.x(), render.y(), render.x() + CURSOR_SIZE, render.y() + CURSOR_SIZE);
}

// -----------------------------------------------------------------------

Point MouseCursor::getTopLeftForHotspotAt(const Point& mouseLocation)
{
  return mouseLocation - m_hotspotLocation;
}

// -----------------------------------------------------------------------
// MouseCursor (private)
// -----------------------------------------------------------------------

void MouseCursor::findHotspot()
{
  int r, g, b;

  for(int x = HOTSPOTMASK_X_OFFSET; x < HOTSPOTMASK_X_OFFSET + CURSOR_SIZE;
      ++x)
  {
    for(int y = HOTSPOTMASK_Y_OFFSET; y < HOTSPOTMASK_Y_OFFSET + CURSOR_SIZE;
        ++y)
    {
      m_cursorSurface->getDCPixel(x, y, r, g, b);

      if(r == 255 && g == 255 && b == 255)
      {
        m_hotspotLocation = 
          Point(x - HOTSPOTMASK_X_OFFSET, y - HOTSPOTMASK_Y_OFFSET);
        break;
      }
    }
  }
}
