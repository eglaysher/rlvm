// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2013 Elliot Glaysher
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
// -----------------------------------------------------------------------

#include "utilities/math_util.h"

#include <cmath>

#include "utilities/exception.h"

int Interpolate(int start, int current, int end, int amount, int mod) {
  double percentage = double(current - start) / double(end - start);

  if (mod == 0) {
    return percentage * amount;
  } else if (mod == 1) {
    double log_percentage = std::log(percentage + 1) / std::log(2);
    return (amount - ((1 - log_percentage) * amount));
  } else if (mod == 2) {
    double log_percentage = std::log(percentage + 1) / std::log(2);
    return (log_percentage * amount);
  } else {
    throw rlvm::Exception("Invalid mod in Interpolate");
  }
}

int InterpolateBetween(int start, int current, int end,
                       int start_val, int end_val, int mod) {
  int to_add = end_val - start_val;
  return start_val + Interpolate(start, current, end, to_add, mod);
}
