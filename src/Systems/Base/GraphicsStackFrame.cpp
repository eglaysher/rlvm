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

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

#include "Systems/Base/GraphicsStackFrame.hpp"

#include "json/value.h"

// -----------------------------------------------------------------------

GraphicsStackFrame::GraphicsStackFrame() 
  : m_commandName(""), m_hasFilename(false), m_hasTargetDC(false),
    m_hasTargetCoordinates(false), m_hasOpacity(false)
{}

// -----------------------------------------------------------------------

GraphicsStackFrame::GraphicsStackFrame(const Json::Value& frame) 
  : m_commandName(""), m_hasFilename(false), m_hasTargetDC(false),
    m_hasTargetCoordinates(false), m_hasOpacity(false)
{
  m_commandName = frame["command"].asString();

  if(frame.isMember("filename"))
    setFilename(frame["filename"].asString());

  if(frame.isMember("targetDC"))
    setTargetDC(frame["targetDC"].asInt());

  if(frame.isMember("opacity"))
    setOpacity(frame["opacity"].asInt());
}

// -----------------------------------------------------------------------

GraphicsStackFrame::GraphicsStackFrame(const std::string& name)
  : m_commandName(name), m_hasFilename(false), m_hasTargetDC(false),
    m_hasTargetCoordinates(false), m_hasOpacity(false)
{}

// -----------------------------------------------------------------------

GraphicsStackFrame::~GraphicsStackFrame() {}

// -----------------------------------------------------------------------

GraphicsStackFrame& GraphicsStackFrame::setFilename(const std::string& filename) 
{
  m_hasFilename = true;
  m_fileName = filename;
  return *this;
}

// -----------------------------------------------------------------------

GraphicsStackFrame& GraphicsStackFrame::setTargetDC(int in)
{
  m_hasTargetDC = true;
  m_targetDC = in;
  return *this;
}

// -----------------------------------------------------------------------

GraphicsStackFrame& GraphicsStackFrame::setOpacity(int in)
{
  m_hasOpacity = true;
  m_opacity = in;
  return *this;
}

// -----------------------------------------------------------------------

void GraphicsStackFrame::serializeTo(Json::Value& frame)
{
  frame["command"] = m_commandName;

  if(m_hasFilename)
    frame["filename"] = m_fileName;

  if(m_hasTargetDC)
    frame["targetDC"] = m_targetDC;

  if(m_hasOpacity)
    frame["opacity"] = m_opacity;
}
