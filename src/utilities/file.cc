// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006 Elliot Glaysher
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

#include "utilities/file.h"

#include <boost/algorithm/string.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iterator>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>

#include "systems/base/system.h"
#include "systems/base/system_error.h"
#include "utilities/exception.h"

using boost::to_upper;
using std::stack;
using std::ostringstream;
using std::string;
using std::ifstream;
using std::ios;

namespace fs = boost::filesystem;

// -----------------------------------------------------------------------

fs::path CorrectPathCase(fs::path Path) {
#ifndef CASE_SENSITIVE_FILESYSTEM
  if (!fs::exists(Path))
    return path();
  return Path;
#else
  // If the path is OK as it stands, do nothing.
  if (fs::exists(Path))
    return Path;
  // If the path doesn't seem to be OK, track backwards through it
  // looking for the point at which the problem first arises.  Path
  // will contain the parts of the path that exist on the current
  // filesystem, and pathElts will contain the parts after that point,
  // which may have incorrect case.
  std::stack<std::string> pathElts;
  while (!Path.empty() && !fs::exists(Path)) {
    pathElts.push(Path.filename().string());
    Path = Path.branch_path();
  }
  // Now proceed forwards through the possibly-incorrect elements.
  while (!pathElts.empty()) {
    // Does this element need to be a directory?
    // (If we are searching for /foo/bar/baz, and /foo contains a file
    // bar and a directory Bar, then we need to know which is the one
    // we're looking for.  This will still be unreliable if /foo
    // contains directories bar and Bar, but a full backtracking
    // search would be complicated; for now this should be adequate!)
    const bool needDir = pathElts.size() > 1;
    std::string elt(pathElts.top());
    pathElts.pop();
    // Does this element exist?
    if (exists(Path / elt) && (!needDir || is_directory(Path / elt))) {
      // If so, use it.
      Path /= elt;
    } else {
      // If not, search for a suitable candidate.
      to_upper(elt);
      fs::directory_iterator end;
      bool found = false;
      for (fs::directory_iterator dir(Path); dir != end; ++dir) {
        std::string uleaf = dir->path().filename().string();
        to_upper(uleaf);
        if (uleaf == elt && (!needDir || is_directory(*dir))) {
          Path /= dir->path().filename();
          found = true;
          break;
        }
      }
      if (!found)
        return "";
    }
  }
  return Path.string();
#endif
}

// -----------------------------------------------------------------------

bool LoadFileData(const boost::filesystem::path& path,
                  std::unique_ptr<char[]>& fileData,
                  int& fileSize) {
  fs::ifstream ifs(path, ifstream::in | ifstream::binary);
  if (!ifs) {
    ostringstream oss;
    oss << "Could not open file \"" << path << "\".";
    throw rlvm::Exception(oss.str());
  }

  ifs.seekg(0, ios::end);
  fileSize = ifs.tellg();
  ifs.seekg(0, ios::beg);

  fileData.reset(new char[fileSize]);
  ifs.read(fileData.get(), fileSize);

  return !ifs.good();
}
