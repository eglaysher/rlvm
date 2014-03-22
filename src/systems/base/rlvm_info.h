// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2009 Elliot Glaysher
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

#ifndef SRC_SYSTEMS_BASE_RLVM_INFO_H_
#define SRC_SYSTEMS_BASE_RLVM_INFO_H_

#include <string>

// Data struct used to pass data to display on the Interpreter menu.
struct RlvmInfo {
  std::string game_name;     // The second part of the #REGNAME key.
  std::string game_brand;    // The first part of the #REGNAME key.
  std::string game_version;  // The #VERSION_STR key.
  std::string game_path;     // Top level game directory.

  std::string rlvm_version;  // rlvm's version as a string.

  bool rlbabel_loaded;
  int text_transformation;
};

#endif  // SRC_SYSTEMS_BASE_RLVM_INFO_H_
