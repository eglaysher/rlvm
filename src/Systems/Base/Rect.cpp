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

#include "Systems/Base/Rect.hpp"

#include <algorithm>
#include <iostream>

// -----------------------------------------------------------------------
// Point
// -----------------------------------------------------------------------
std::ostream& operator<<(std::ostream& os, const Point& p) {
  os << "Point(" << p.x() << ", " << p.y() << ")";
  return os;
}

// -----------------------------------------------------------------------
// Size
// -----------------------------------------------------------------------
Size Size::sizeUnion(const Size& rhs) const {
  return Size(std::max(width_, rhs.width_),
              std::max(height_, rhs.height_));
}

std::ostream& operator<<(std::ostream& os, const Size& s) {
  os << "Size(" << s.width() << ", " << s.height() << ")";
  return os;
}

// -----------------------------------------------------------------------
// Rect
// -----------------------------------------------------------------------
bool Rect::contains(const Point& loc) {
  return loc.x() >= x() && loc.x() < x2() && loc.y() >= y() && loc.y() < y2();
}

std::ostream& operator<<(std::ostream& os, const Rect& r) {
  os << "Rect(" << r.x() << ", " << r.y() << ", " << r.size() << ")";
  return os;
}
