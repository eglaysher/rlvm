// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//

#include "Systems/Base/Rect.hpp"

// -----------------------------------------------------------------------
// Rect
// -----------------------------------------------------------------------
bool Rect::contains(const Point& loc) {
  return loc.x() >= x() && loc.x() < x2() && loc.y() >= y() && loc.y() < y2();
}
