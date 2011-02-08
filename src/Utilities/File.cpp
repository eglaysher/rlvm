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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// -----------------------------------------------------------------------

#include "Utilities/File.hpp"

#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost/assign/list_of.hpp>  // for 'list_of()'
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <cctype>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#include <vector>

#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/SystemError.hpp"
#include "Utilities/Exception.hpp"

using boost::to_upper;
using boost::scoped_array;
using boost::assign::list_of;
using std::stack;
using std::cerr;
using std::endl;
using std::ostringstream;
using std::string;
using std::vector;
using std::ifstream;
using std::ios;
using std::ostream_iterator;

namespace fs = boost::filesystem;

// -----------------------------------------------------------------------

fs::path correctPathCase(fs::path Path) {
  using namespace boost::filesystem;

#ifndef CASE_SENSITIVE_FILESYSTEM
  if (!exists(Path))
    return path();
  return Path;
#else
  // If the path is OK as it stands, do nothing.
  if (exists(Path)) return Path;
  // If the path doesn't seem to be OK, track backwards through it
  // looking for the point at which the problem first arises.  Path
  // will contain the parts of the path that exist on the current
  // filesystem, and pathElts will contain the parts after that point,
  // which may have incorrect case.
  stack<string> pathElts;
  while (!Path.empty() && !exists(Path)) {
    pathElts.push(Path.leaf());
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
    string elt(pathElts.top());
    pathElts.pop();
    // Does this element exist?
    if (exists(Path/elt) && (!needDir || is_directory(Path/elt))) {
      // If so, use it.
      Path /= elt;
    } else {
      // If not, search for a suitable candidate.
      to_upper(elt);
      directory_iterator end;
      bool found = false;
      for (directory_iterator dir(Path); dir != end; ++dir) {
        string uleaf = dir->leaf();
        to_upper(uleaf);
        if (uleaf == elt && (!needDir || is_directory(*dir))) {
          Path /= dir->leaf();
          found = true;
          break;
        }
      }
      if (!found) return "";
    }
  }
  return Path.string();
#endif
}

// -----------------------------------------------------------------------

bool loadFileData(const boost::filesystem::path& path,
                  scoped_array<char>& fileData,
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

