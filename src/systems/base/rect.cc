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

#include "systems/base/rect.h"

#include <algorithm>
#include <ostream>

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
Rect Size::CenteredIn(const Rect& r) const {
  int half_r_width = r.width() / 2;
  int half_r_height = r.height() / 2;

  int half_width = width() / 2;
  int half_height = height() / 2;

  int new_x = r.x() + half_r_width - half_width;
  int new_y = r.y() + half_r_height - half_height;

  return Rect(Point(new_x, new_y), *this);
}

Size Size::SizeUnion(const Size& rhs) const {
  return Size(std::max(width_, rhs.width_), std::max(height_, rhs.height_));
}

std::ostream& operator<<(std::ostream& os, const Size& s) {
  os << "Size(" << s.width() << ", " << s.height() << ")";
  return os;
}

// -----------------------------------------------------------------------
// Rect
// -----------------------------------------------------------------------
bool Rect::Contains(const Point& loc) {
  return loc.x() >= x() && loc.x() < x2() && loc.y() >= y() && loc.y() < y2();
}

bool Rect::Intersects(const Rect& rhs) const {
  return !(x() > rhs.x2() || x2() < rhs.x() || y() > rhs.y2() ||
           y2() < rhs.y());
}

Rect Rect::Intersection(const Rect& rhs) const {
  if (Intersects(rhs)) {
    return Rect::GRP(std::max(x(), rhs.x()),
                     std::max(y(), rhs.y()),
                     std::min(x2(), rhs.x2()),
                     std::min(y2(), rhs.y2()));
  }

  return Rect();
}

Rect Rect::RectUnion(const Rect& rhs) const {
  if (is_empty()) {
    return rhs;
  } else if (rhs.is_empty()) {
    return *this;
  } else {
    return Rect::GRP(std::min(x(), rhs.x()),
                     std::min(y(), rhs.y()),
                     std::max(x2(), rhs.x2()),
                     std::max(y2(), rhs.y2()));
  }
}

Rect Rect::GetInsetRectangle(const Rect& rhs) const {
  Size p = rhs.origin() - origin();
  return Rect(Point(p.width(), p.height()), rhs.size());
}

Rect Rect::ApplyInset(const Rect& inset) const {
  Point p = origin() + inset.origin();
  return Rect(p, inset.size());
}

std::ostream& operator<<(std::ostream& os, const Rect& r) {
  os << "Rect(" << r.x() << ", " << r.y() << ", " << r.size() << ")";
  return os;
}
