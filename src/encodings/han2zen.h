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
// -----------------------------------------------------------------------

#ifndef SRC_ENCODINGS_HAN2ZEN_H_
#define SRC_ENCODINGS_HAN2ZEN_H_

#include <string>

// Converts half-width ASCII and katakana characters to their full-width
// equivalents in a CP932 string. This is a no-op when |transformation| is
// anything other than 0.
std::string hantozen_cp932(const std::string& string, int transformation);

// Converts full-width ASCII and katakana characters to their half-width
// equivalents in a CP932 string. This is a no-op when |transformation| is
// anything other than 0.
std::string zentohan_cp932(const std::string& string, int transformation);

#endif  // SRC_ENCODINGS_HAN2ZEN_H_
