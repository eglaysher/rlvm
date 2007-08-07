// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006, 2007 Elliot Glaysher
//  
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
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

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

#include "libReallive/gameexe.h"
#include "Systems/Base/System.hpp"
#include "Systems/Base/SystemError.hpp"

#include <boost/bind.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>

#include <algorithm>
#include <string>
#include <iostream>

using namespace std;
using boost::bind;

namespace fs = boost::filesystem;

// -----------------------------------------------------------------------

void System::addPath(GameexeInterpretObject gio)
{
  string gamepath = gameexe()("__GAMEPATH").to_string();
  gamepath += gio.to_string();
  cachedSearchPaths.push_back(gamepath);
}

// -----------------------------------------------------------------------

const std::vector<std::string>& System::getSearchPaths()
{
  if(cachedSearchPaths.size() == 0)
  {
    Gameexe& gexe = gameexe();
    
    // This *can't* be rewritten as a for_each + bind because of the
    // forwarding problem. See
    // http://www.boost.org/libs/bind/bind.html#Limitations.
    GameexeFilteringIterator it = gexe.filtering_begin("FOLDNAME");
    GameexeFilteringIterator end = gexe.filtering_end();
    for(; it != end; ++it)
      addPath(*it);
  }

  return cachedSearchPaths;
}

// -----------------------------------------------------------------------

boost::filesystem::path System::getHomeDirectory()
{
  string drive, home;
  if((home = getenv("HOME")) != "")
  {
	// UN*X like home directory
	return fs::path(home);
  }
  else if((drive = getenv("HOMEDRIVE")) != "" &&
		  (home  = getenv("HOMEPATH")) != "")
  {
	// Windows.
	return fs::path(drive) / fs::path(home);
  }
  else if((home = getenv("USERPROFILE")) != "")
  {
	// Windows?
	return fs::path(home);
  }
  else
  {
	throw SystemError("Could not find location of home directory.");
  }
}

// -----------------------------------------------------------------------

boost::filesystem::path System::gameSaveDirectory()
{
  Gameexe& gexe = gameexe();

  fs::path baseDir = getHomeDirectory() / ".rlvm" / gexe("REGNAME").to_string();
  fs::create_directories(baseDir);

  return baseDir;
}
