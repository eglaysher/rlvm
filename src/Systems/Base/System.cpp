// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006 Elliot Glaysher
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

#include <boost/bind.hpp>

#include <algorithm>
#include <string>
#include <iostream>

using namespace std;
using boost::bind;

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
