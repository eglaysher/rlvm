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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
//
// -----------------------------------------------------------------------

#include "systems/base/colour.h"

#include <ostream>
#include <vector>

// -----------------------------------------------------------------------
// RGBColour
// -----------------------------------------------------------------------
RGBColour::RGBColour(const std::vector<int>& colour)
    : r_(colour.at(0)), g_(colour.at(1)), b_(colour.at(2)) {}

bool RGBColour::operator==(const RGBColour& rhs) const {
  return r_ == rhs.r_ && g_ == rhs.g_ && b_ == rhs.b_;
}

bool RGBColour::operator!=(const RGBColour& rhs) const {
  return !operator==(rhs);
}

std::ostream& operator<<(std::ostream& os, const RGBColour& rgb) {
  os << "RGB(" << rgb.r() << ", " << rgb.g() << ", " << rgb.b() << ")";

  return os;
}

// -----------------------------------------------------------------------
// RGBAColour
// -----------------------------------------------------------------------
RGBAColour::RGBAColour(const std::vector<int>& colour)
    : rgb_(colour), alpha_(255) {}

bool RGBAColour::operator==(const RGBAColour& rhs) const {
  return rgb_ == rhs.rgb_ && alpha_ == rhs.alpha_;
}

bool RGBAColour::operator!=(const RGBAColour& rhs) const {
  return !operator==(rhs);
}

std::ostream& operator<<(std::ostream& os, const RGBAColour& rgba) {
  os << "RGBA(" << rgba.r() << ", " << rgba.g() << ", " << rgba.b() << ", "
     << rgba.a() << ")";

  return os;
}
