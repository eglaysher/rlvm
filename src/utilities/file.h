// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006, 2007 Elliot Glaysher
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

#ifndef SRC_UTILITIES_FILE_H_
#define SRC_UTILITIES_FILE_H_

#include <boost/filesystem.hpp>

#include <iosfwd>
#include <string>
#include <vector>

class Gameexe;
class RLMachine;
class System;

// On platforms with case-insensitive file systems, returns a copy of the input
// unchanged. On less tolerant platforms, returns a copy of the input with
// correct case, or the empty string if no solution could be found.
boost::filesystem::path CorrectPathCase(boost::filesystem::path Path);

// Reads the entire contents of a file into the passed in |data| and
// |size|. Returns true if there were no problems.
bool LoadFileData(const boost::filesystem::path& path,
                  std::unique_ptr<char[]>& fileData,
                  int& fileSize);

#endif  // SRC_UTILITIES_FILE_H_
