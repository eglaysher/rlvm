// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
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


#ifndef __GraphicsObjectOfFile_hpp__
#define __GraphicsObjectOfFile_hpp__

#include "Systems/Base/GraphicsObjectData.hpp"
#include <string>
#include <boost/shared_ptr.hpp>

class Surface;
class RLMachine;

// -----------------------------------------------------------------------

/** 
 * GraphicsObjectData class that encapsulates a G00 or ANM file.
 * 
 * GraphicsObjectOfFile is used for loading individual bitmaps into an
 * object. It has support for normal display, and also 
 *
 */
class GraphicsObjectOfFile : public GraphicsObjectData
{
private:
  /// The encapsulated surface to render
  boost::shared_ptr<Surface> m_surface;

  /// Number of miliseconds to spend on a single frame in the
  /// animation
  unsigned int m_frameTime;

  /// Current frame displayed (when animating)
  int m_currentFrame;

  /// While currentlyPlaying() is true, this variable is used to store
  /// the time when the frame was switched last
  unsigned int m_timeAtLastFrameChange;

  // Private constructor for cloning
  GraphicsObjectOfFile(const GraphicsObjectOfFile& obj);

protected:
  virtual void loopAnimation();

public:
  GraphicsObjectOfFile(GraphicsSystem& graphics, const std::string& filename);

  virtual void render(RLMachine& machine, GraphicsObject& rp);
  virtual int pixelWidth(RLMachine& machine, GraphicsObject& rp);
  virtual int pixelHeight(RLMachine& machine, GraphicsObject& rp);
  virtual GraphicsObjectData* clone() const;

  virtual void execute(RLMachine& machine);

  virtual bool isAnimation() const;
  virtual void playSet(RLMachine& machine, int set);
};

#endif
