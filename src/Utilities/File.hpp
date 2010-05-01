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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// -----------------------------------------------------------------------

#ifndef SRC_UTILITIES_FILE_HPP_
#define SRC_UTILITIES_FILE_HPP_

#include <string>
#include <vector>
#include <iosfwd>

#include <boost/scoped_array.hpp>
#include <boost/filesystem.hpp>

class Gameexe;
class RLMachine;
class System;

// On platforms with case-insensitive file systems, returns a copy of the input
// unchanged. On less tolerant platforms, returns a copy of the input with
// correct case, or the empty string if no solution could be found.
boost::filesystem::path correctPathCase(boost::filesystem::path Path);

// File type constants.
//
// These constant, externed vectors are passed as parameters to
// findFile to control which file types are searched for. Defaults to
// all.
extern const std::vector<std::string> ALL_FILETYPES;
extern const std::vector<std::string> IMAGE_FILETYPES;
extern const std::vector<std::string> PDT_IMAGE_FILETYPES;
extern const std::vector<std::string> GAN_FILETYPES;
extern const std::vector<std::string> ANM_FILETYPES;
extern const std::vector<std::string> HIK_FILETYPES;
extern const std::vector<std::string> SOUND_FILETYPES;
extern const std::vector<std::string> KOE_ARCHIVE_FILETYPES;
extern const std::vector<std::string> KOE_LOOSE_FILETYPES;


// Returns the full path to a g00 file for the basename of the file, or empty()
// if file not found.
boost::filesystem::path findFile(
    System& system,
    const std::string& fileName,
    const std::vector<std::string>& extensions = ALL_FILETYPES);

// Reads the entire contents of a file into the passed in |data| and
// |size|. Returns true if there were no problems.
bool loadFileData(const boost::filesystem::path& path,
                  boost::scoped_array<char>& fileData,
                  int& fileSize);

#endif  // SRC_UTILITIES_FILE_HPP_
