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


#ifndef __GraphicsStackFrame_hpp__
#define __GraphicsStackFrame_hpp__

#include "Systems/Base/Rect.hpp"

#include <string>
#include <boost/serialization/access.hpp>

// -----------------------------------------------------------------------

/**
 * Represents one frame in the graphics stack.
 *
 * @todo Move the serialization code into this class
 */
class GraphicsStackFrame
{
private:
  friend class boost::serialization::access;

  /// boost::serialization support
  template<class Archive>
  void serialize(Archive& ar, unsigned int version)
  {
    ar & m_commandName & m_hasFilename & m_fileName & m_hasSourceDC
      & m_sourceDC & m_hasSourceCoordinates & m_sourceRect
      & m_hasTargetDC & m_targetDC
      & m_hasTargetCoordinates & m_targetPoint
      & m_hasRGB & m_r & m_g & m_b & m_hasOpacity
      & m_opacity & m_hasMask & m_mask;
  }

  std::string m_commandName;

  bool m_hasFilename;
  std::string m_fileName;

  bool m_hasSourceDC;
  int m_sourceDC;

  bool m_hasSourceCoordinates;
  Rect m_sourceRect;

  bool m_hasTargetDC;
  int m_targetDC;

  bool m_hasTargetCoordinates;
  Point m_targetPoint;

  bool m_hasRGB;
  int m_r, m_g, m_b;

  bool m_hasOpacity;
  int m_opacity;

  bool m_hasMask;
  int m_mask;

public:
  GraphicsStackFrame();
  GraphicsStackFrame(const std::string& name);
  ~GraphicsStackFrame();

  const std::string& name() const { return m_commandName; }

  bool hasFilename() const { return m_hasFilename; }
  const std::string& filename() const { return m_fileName; }
  GraphicsStackFrame& setFilename(const std::string& filename);

  // source stuff
  bool hasSourceDC() const { return m_hasSourceDC; }
  int sourceDC() const { return m_sourceDC; }
  GraphicsStackFrame& setSourceDC(int in);

  bool hasSourceCoordinates() const { return m_hasSourceCoordinates; }
  Rect sourceRect() const { return m_sourceRect; }
  GraphicsStackFrame& setSourceCoordinates(const Rect& in);

  bool hasTargetDC() const { return m_hasTargetDC; }
  int targetDC() const { return m_targetDC; }
  GraphicsStackFrame& setTargetDC(int in);

  bool hasTargetCoordinates() const { return m_hasTargetCoordinates; }
  Point targetPoint() const { return m_targetPoint; }
  GraphicsStackFrame& setTargetCoordinates(const Point& targetPoint);

  bool hasRGB() const { return m_hasRGB; }
  int r() const { return m_r; }
  int g() const { return m_g; }
  int b() const { return m_b; }
  GraphicsStackFrame& setRGB(int r, int g, int b);

  bool hasOpacity() const { return m_hasOpacity; }
  int opacity() const { return m_opacity; }
  GraphicsStackFrame& setOpacity(int in);

  bool hasMask() const { return m_hasMask; }
  bool mask() const { return m_mask; }
  GraphicsStackFrame& setMask(bool in);
};	// end of class GraphicsStackFrame


#endif
