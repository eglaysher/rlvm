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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
//
// -----------------------------------------------------------------------

#include "systems/base/surface.h"

#include "utilities/exception.h"

// -----------------------------------------------------------------------

Surface::Surface() {}

// -----------------------------------------------------------------------

Surface::~Surface() {}

// -----------------------------------------------------------------------

Rect Surface::GetRect() const { return Rect(Point(0, 0), GetSize()); }

// -----------------------------------------------------------------------

void Surface::Dump() {
  throw rlvm::Exception("Unimplemented function Surface::Dump()");
}

// -----------------------------------------------------------------------

int Surface::GetNumPatterns() const { return 1; }

// -----------------------------------------------------------------------

const Surface::GrpRect& Surface::GetPattern(int patt_no) const {
  static GrpRect rect;
  return rect;
}

// -----------------------------------------------------------------------

std::shared_ptr<Surface> Surface::ClipAsColorMask(const Rect& clip_rect,
                                                    int r,
                                                    int g,
                                                    int b) const {
  throw rlvm::Exception("Unimplemented function Surface::ClipAsColorMask()");
}
