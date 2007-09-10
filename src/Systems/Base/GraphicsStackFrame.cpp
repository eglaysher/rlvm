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
  : m_commandName(""), m_hasFilename(false), m_hasSourceDC(false),
    m_hasSourceCoordinates(false), m_hasTargetDC(false),
    m_hasTargetCoordinates(false), m_hasRGB(false), m_hasOpacity(false),
    m_hasMask(false)
{}

// -----------------------------------------------------------------------

GraphicsStackFrame::GraphicsStackFrame(const Json::Value& frame) 
  : m_commandName(""), m_hasFilename(false), m_hasSourceDC(false),
    m_hasSourceCoordinates(false), m_hasTargetDC(false),
    m_hasTargetCoordinates(false), m_hasRGB(false), m_hasOpacity(false),
    m_hasMask(false)
{
  m_commandName = frame["command"].asString();

  if(frame.isMember("filename"))
    setFilename(frame["filename"].asString());

  if(frame.isMember("sourceDC"))
    setSourceDC(frame["sourceDC"].asInt());

  if(frame.isMember("sourceCoordinates"))
  {
    const Json::Value& src = frame["sourceCoordinates"];
    setSourceCoordinates(src[0u].asInt(), src[1u].asInt(), src[2u].asInt(),
                         src[3u].asInt());
  }

  if(frame.isMember("targetDC"))
    setTargetDC(frame["targetDC"].asInt());

  if(frame.isMember("targetCoordinates"))
  {
    const Json::Value& src = frame["targetCoordinates"];
    setTargetCoordinates(src[0u].asInt(), src[1u].asInt(), src[2u].asInt(),
                         src[3u].asInt());
  }

  if(frame.isMember("rgb"))
  {
    const Json::Value& rgb = frame["rgb"];
    setRGB(rgb[0u].asInt(), rgb[1u].asInt(), rgb[2u].asInt());
  }

  if(frame.isMember("opacity"))
    setOpacity(frame["opacity"].asInt());

  if(frame.isMember("mask"))
    setMask(frame["mask"].asInt());
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

GraphicsStackFrame& GraphicsStackFrame::setSourceDC(int in)
{
  m_hasSourceDC = true;
  m_sourceDC = in;
  return *this;
}

// -----------------------------------------------------------------------

GraphicsStackFrame& GraphicsStackFrame::setSourceCoordinates(
  int x1, int y1, int x2, int y2)
{
  m_hasSourceCoordinates = true;
  m_sourceX = x1;
  m_sourceY = y1;
  m_sourceX2 = x2;
  m_sourceY2 = y2;
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

GraphicsStackFrame& GraphicsStackFrame::setTargetCoordinates(
  int x1, int y1, int x2, int y2)
{
  m_hasTargetCoordinates = true;
  m_targetX = x1;
  m_targetY = y1;
  m_targetX2 = x2;
  m_targetY2 = y2;
  return *this;
}

// -----------------------------------------------------------------------

GraphicsStackFrame& GraphicsStackFrame::setRGB(int r, int g, int b)
{
  m_hasRGB = true;
  m_r = r;
  m_g = g; 
  m_b = b;
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

GraphicsStackFrame& GraphicsStackFrame::setMask(bool in)
{
  m_hasMask = true;
  m_mask = in;
  return *this;
}

// -----------------------------------------------------------------------

void GraphicsStackFrame::serializeTo(Json::Value& frame)
{
  frame["command"] = m_commandName;

  if(m_hasFilename)
    frame["filename"] = m_fileName;

  if(m_hasSourceDC)
    frame["sourceDC"] = m_sourceDC;

  if(m_hasSourceCoordinates)
  {
    Json::Value source(Json::arrayValue);
    source.push_back(m_sourceX);
    source.push_back(m_sourceY);
    source.push_back(m_sourceX2);
    source.push_back(m_sourceY2);

    frame["sourceCoordinates"] = source;
  }

  if(m_hasTargetDC)
    frame["targetDC"] = m_targetDC;

  if(m_hasTargetCoordinates)
  {
    Json::Value target(Json::arrayValue);
    target.push_back(m_targetX);
    target.push_back(m_targetY);
    target.push_back(m_targetX2);
    target.push_back(m_targetY2);

    frame["targetCoordinates"] = target;
  }

  if(m_hasRGB)
  {
    Json::Value rgb(Json::arrayValue);
    rgb.push_back(m_r);
    rgb.push_back(m_g);
    rgb.push_back(m_b);

    frame["rgb"] = rgb;
  }

  if(m_hasOpacity)
    frame["opacity"] = m_opacity;

  if(m_hasMask)
    frame["mask"] = m_opacity;
}
