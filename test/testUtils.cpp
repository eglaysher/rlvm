// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2007 Elliot Glaysher
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

#include "testUtils.hpp"
#include <vector>
#include <boost/assign/list_of.hpp> // for 'list_of()'
#include <boost/filesystem/operations.hpp>
#include <stdexcept>

using boost::assign::list_of;
using std::string;
using std::vector;

namespace fs = boost::filesystem;

// -----------------------------------------------------------------------

const std::vector<std::string> testPaths =
  list_of("./")("./test/");

string locateTestCase(const string& baseName)
{
  for(vector<string>::const_iterator it = testPaths.begin();
      it != testPaths.end(); ++it)
  {
    string testName = *it + baseName;
    if(fs::exists(testName))
      return testName;
  }

  throw std::runtime_error("Could not locate data file in locateTestCase.");
}
