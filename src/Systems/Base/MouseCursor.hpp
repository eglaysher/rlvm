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

#ifndef __MouseCursor_hpp__
#define __MouseCursor_hpp__

#include <boost/shared_ptr.hpp>

class Surface;
class RLMachine;

/**
 * Represents a mouse cursor on screen.
 */
class MouseCursor
{
public:
  MouseCursor(const boost::shared_ptr<Surface>& cursorSurface);
  ~MouseCursor();

  /**
   * Renders the cursor to the screen, taking the hotspot offset into
   * account.
   */
  void renderHotspotAt(RLMachine& machine, int x, int y);

private:
  /// Sets m_hotspot[XY] to the white pixel in the 
  void findHotspot();

  /// The raw image read from the PDT.
  boost::shared_ptr<Surface> m_cursorSurface;
  
  /// The hotspot location.
  int m_hotspotX, m_hotspotY;
};	// end of class MouseCursor


#endif
