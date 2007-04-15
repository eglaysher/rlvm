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

#include <stdexcept>

#include "Utilities.h"
#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/SystemError.hpp"
#include "libReallive/gameexe.h"

#include <string>
#include <iostream>
#include <stack>
#include <cctype>
#include "boost/filesystem/operations.hpp"

using namespace std;

// -----------------------------------------------------------------------

inline void uppercase(string& what)
{
  for (string::size_type i = 0; i < what.size(); ++i) 
    what[i] = toupper(what[i]);
}

// -----------------------------------------------------------------------

string correctPathCase(const string& fileName)
{
  using namespace boost::filesystem;  
  path Path(fileName);

#ifndef CASE_SENSITIVE_FILESYSTEM
  if(!exists(Path)) return "";
  return fileName;
#else
  // If the path is OK as it stands, do nothing.
  if (exists(Path)) return fileName;
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
    }
    else {
      // If not, search for a suitable candidate.
      uppercase(elt);
      directory_iterator end;
      bool found = false;
      for (directory_iterator dir(Path); dir != end; ++dir) {
        string uleaf = dir->leaf();
        uppercase(uleaf);
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

std::string findFontFile(const std::string& fileName)
{
  using namespace boost::filesystem;  
  path home(getenv("HOME"));
  return (home / fileName).string();
}

// -----------------------------------------------------------------------

/**
 * @todo This function is a hack and needs to be completely rewritten
 *       to use the \#FOLDNAME table in the Gameexe.ini file.
 */
string findFile(RLMachine& machine, const string& fileName)
{
  // Hack until I do this correctly
  string gamepath = machine.system().gameexe()("__GAMEPATH").to_string();

  // First search for this file as a g00
  string file = gamepath + "g00/" + fileName + ".g00";
  string correctFile = correctPathCase(file);
  if(correctFile == "")
  {
    // Then try PDT.
    file = gamepath + "pdt/" + fileName + ".pdt";
    correctFile = correctPathCase(file);
  }

  return correctFile;
}

// -----------------------------------------------------------------------

std::vector<int> getSELEffect(RLMachine& machine, int selNum)
{
  Gameexe& gexe = machine.system().gameexe();
  vector<int> selEffect;

  if(gexe("SEL", selNum).exists())
  {
    selEffect = gexe("SEL", selNum).to_intVector();
    grpToRecCoordinates(selEffect[0], selEffect[1], 
                        selEffect[2], selEffect[3]);
  }
  else if(gexe("SELR", selNum).exists())
    selEffect = gexe("SELR", selNum).to_intVector();
  else
  {
    ostringstream oss;
    oss << "Could not find either #SEL." << selNum << " or #SELR."
        << selNum;
    throw SystemError(oss.str());
  }

  return selEffect;
}

// -----------------------------------------------------------------------

void getScreenSize(Gameexe& gameexe, int& width, int& height)
{
  int graphicsMode = gameexe("SCREENSIZE_MOD");
  if(graphicsMode == 0)
  {
    width = 640;
    height = 480;
  }
  else if(graphicsMode == 1)
  {
    width = 800;
    height = 600;
  }
  else
  {
    ostringstream oss;
    oss << "Illegal #SCREENSIZE_MOD value: " << graphicsMode << endl;
    throw SystemError(oss.str());
  }
}

// -----------------------------------------------------------------------
// rlvm::Exception
// -----------------------------------------------------------------------

namespace rlvm {

const char* Exception::what() const throw()
{
  return description.c_str(); 
}

// -----------------------------------------------------------------------

Exception::Exception(std::string what) 
  : description(what) 
{}

// -----------------------------------------------------------------------

Exception::~Exception() throw() {}

}


