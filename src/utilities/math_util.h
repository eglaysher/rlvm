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

#ifndef SRC_UTILITIES_MATH_UTIL_H_
#define SRC_UTILITIES_MATH_UTIL_H_

// Interpolates between |start| and |end|. Returns a percentage of |amount|.
int Interpolate(int start, int current, int end, int amount, int mod);

// Interpolates a value between |start_val| and |end_val|.
int InterpolateBetween(int start, int current, int end,
                       int start_val, int end_val, int mod);

#endif  // SRC_UTILITIES_MATH_UTIL_H_
