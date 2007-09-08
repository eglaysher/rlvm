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


#ifndef __GraphicsStackFrame_hpp__
#define __GraphicsStackFrame_hpp__

#include <string>

// -----------------------------------------------------------------------

namespace Json {
class Value;
}

// -----------------------------------------------------------------------

/**
 * Represents one frame in the graphics stack. 
 */
class GraphicsStackFrame
{
private:
  std::string m_commandName;

  bool m_hasFilename;
  std::string m_fileName;

  bool m_hasTargetDC;
  int m_targetDC;

  bool m_hasTargetCoordinates;


  bool m_hasOpacity;
  int m_opacity;

public: 
  GraphicsStackFrame();
  GraphicsStackFrame(const Json::Value& frame);
  GraphicsStackFrame(const std::string& name);
  ~GraphicsStackFrame();

  const std::string& name() const { return m_commandName; }

  bool hasFilename() const { return m_hasFilename; }
  const std::string& filename() const { return m_fileName; }
  GraphicsStackFrame& setFilename(const std::string& filename);

  bool hasTargetDC() const { return m_hasTargetDC; }
  int targetDC() const { return m_targetDC; }
  GraphicsStackFrame& setTargetDC(int in);

  bool hasOpacity() const { return m_hasOpacity; }
  int opacity() const { return m_opacity; }
  GraphicsStackFrame& setOpacity(int in);

  void serializeTo(Json::Value& frame);
};	// end of class GraphicsStackFrame


#endif
