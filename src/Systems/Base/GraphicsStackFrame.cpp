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

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

#include "Systems/Base/GraphicsStackFrame.hpp"

// -----------------------------------------------------------------------
// GraphicsStackFrame
// -----------------------------------------------------------------------
GraphicsStackFrame::GraphicsStackFrame()
  : command_name_(""), has_filename_(false), has_source_dc_(false),
    has_source_coordinates_(false), has_target_dc_(false),
    has_target_coordinates_(false), has_rgb_(false), has_opacity_(false),
    has_mask_(false)
{}

// -----------------------------------------------------------------------

GraphicsStackFrame::GraphicsStackFrame(const std::string& name)
  : command_name_(name), has_filename_(false), has_source_dc_(false),
    has_source_coordinates_(false), has_target_dc_(false),
    has_target_coordinates_(false), has_rgb_(false), has_opacity_(false),
    has_mask_(false)
{}

// -----------------------------------------------------------------------

GraphicsStackFrame::~GraphicsStackFrame() {}

// -----------------------------------------------------------------------

GraphicsStackFrame& GraphicsStackFrame::setFilename(const std::string& filename)
{
  has_filename_ = true;
  file_name_ = filename;
  return *this;
}

// -----------------------------------------------------------------------

GraphicsStackFrame& GraphicsStackFrame::setSourceDC(int in)
{
  has_source_dc_ = true;
  source_dc_ = in;
  return *this;
}

// -----------------------------------------------------------------------

GraphicsStackFrame& GraphicsStackFrame::setSourceCoordinates(
  const Rect& src_rect)
{
  has_source_coordinates_ = true;
  source_rect_ = src_rect;
  return *this;
}

// -----------------------------------------------------------------------

GraphicsStackFrame& GraphicsStackFrame::setTargetDC(int in)
{
  has_target_dc_ = true;
  target_dc_ = in;
  return *this;
}

// -----------------------------------------------------------------------

GraphicsStackFrame& GraphicsStackFrame::setTargetCoordinates(
  const Point& target_point)
{
  has_target_coordinates_ = true;
  target_point_ = target_point;
  return *this;
}

// -----------------------------------------------------------------------

GraphicsStackFrame& GraphicsStackFrame::setRGB(int r, int g, int b)
{
  has_rgb_ = true;
  r_ = r;
  g_ = g;
  b_ = b;
  return *this;
}

// -----------------------------------------------------------------------

GraphicsStackFrame& GraphicsStackFrame::setOpacity(int in)
{
  has_opacity_ = true;
  opacity_ = in;
  return *this;
}

// -----------------------------------------------------------------------

GraphicsStackFrame& GraphicsStackFrame::setMask(bool in)
{
  has_mask_ = true;
  mask_ = in;
  return *this;
}

// -----------------------------------------------------------------------

std::ostream& operator<<(std::ostream& os, const GraphicsStackFrame& frame) {
  os << "{command=" << frame.name();

  if (frame.hasFilename())
    os << ", filename=" << frame.filename();
  if (frame.hasSourceDC())
    os << ", srcDC=" << frame.sourceDC();
  if (frame.hasSourceCoordinates())
    os << ", srcRect=" << frame.sourceRect();
  if (frame.hasTargetDC())
    os << ", dstDC=" << frame.targetDC();
  if (frame.hasTargetCoordinates())
    os << ", dstRect=" << frame.targetPoint();
  if (frame.hasRGB())
    os << ", rgb=[" << frame.r() << "," << frame.g() << "," << frame.b() << "]";
  if (frame.hasOpacity())
    os << ", opacity=" << frame.opacity();
  if (frame.hasMask())
    os << ", mask=" << frame.mask();

  os << "}";

  return os;
}
