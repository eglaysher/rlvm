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

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

#include <stdexcept>

#include "Utilities.h"
#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/SystemError.hpp"
#include "libReallive/gameexe.h"

#include <fstream>
#include <string>
#include <iostream>
#include <iterator>
#include <stack>
#include <cctype>
#include "boost/filesystem/operations.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/assign/list_of.hpp> // for 'list_of()'

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

fs::path correctPathCase(fs::path Path)
{
  using namespace boost::filesystem;  
 
#ifndef CASE_SENSITIVE_FILESYSTEM
  if(!exists(Path)) return path();
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
    }
    else {
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

fs::path findFontFile(RLMachine& machine)
{
  return findFontFile(machine.system().gameexe(), "msgothic.ttc");
}

// -----------------------------------------------------------------------

fs::path findFontFile(Gameexe& gexe, const std::string& fileName)
{
  // HACK: If the user has overridden the __GAMEFONT, use it instead.
  if(gexe.exists("__GAMEFONT"))
  {
    std::string gamefontstr = gexe("__GAMEFONT");
    fs::path gameFont = fs::path(gamefontstr);
    if(fs::exists(gameFont))
      return gameFont;
  }

  // HACK: Look for the font in the game
  if(gexe.exists("__GAMEPATH"))
  {
    std::string gamepath = gexe("__GAMEPATH");
    fs::path gamePathFont = fs::path(gamepath) / fileName;
    if(fs::exists(gamePathFont))
      return gamePathFont;
  }

  char* homeptr = getenv("HOME");
  char* rootptr = getenv("SYSTEMROOT");
  fs::path home;
  if (homeptr != 0) {
    home = homeptr;
  }
  else if (rootptr != 0) {
    home = rootptr;
    home /= "Fonts";
  }

  fs::path filePath = home / fileName;

  if(fs::exists(filePath))
    return filePath;
  else
    return fs::path();
}

// -----------------------------------------------------------------------

const std::vector<std::string> ALL_FILETYPES = 
  list_of("g00")("pdt")("anm")("gan");
const std::vector<std::string> IMAGE_FILETYPES = 
  list_of("g00")("pdt");
const std::vector<std::string> GAN_FILETYPES =
  list_of("gan");

/**
 * @todo This function is a hack and needs to be completely rewritten
 *       to use the \#FOLDNAME table in the Gameexe.ini file.
 */
boost::filesystem::path findFile(RLMachine& machine, 
								 const std::string& fileName,
							     const vector<string>& extensions)
{
  using namespace boost;

  // Hack to get around fileNames like "REALNAME?010", where we only
  // want REALNAME.
  string newName = 
    string(fileName.begin(), find(fileName.begin(), fileName.end(), '?'));

  // Iterate across the search paths in the order they were specified.
  const vector<boost::filesystem::path>& blah = machine.system().getSearchPaths();
  for(vector<boost::filesystem::path>::const_iterator it = blah.begin(); it != blah.end(); ++it)
  {
    for(vector<string>::const_iterator ext = extensions.begin();
        ext != extensions.end(); ++ext)
    {
	  string fileWithExt = newName + "." + *ext;
	  fs::path path = *it / fileWithExt;
	  fs::path correctFile = correctPathCase(path);
      if(!correctFile.empty())
      {
        return correctFile;
      }
    }
  }

  ostringstream oss;
  oss << "Could not open file: \"" << newName << "\" looking for types: {";
  copy(extensions.begin(), extensions.end(), 
       ostream_iterator<string>(oss, ","));
  oss << "}";

  throw rlvm::Exception(oss.str());
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

void clamp(float& var, float min, float max)
{
  if(var < min)
    var = min;
  else if(var > max)
    var = max;
}

// -----------------------------------------------------------------------

bool loadFileData(ifstream& ifs, scoped_array<char>& anmData, int& fileSize)
{
  ifs.seekg(0, ios::end);
  fileSize = ifs.tellg();  
  ifs.seekg(0, ios::beg);

  anmData.reset(new char[fileSize]);
  ifs.read(anmData.get(), fileSize);

  return !ifs.good();
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

// -----------------------------------------------------------------------

UnimplementedOpcode::UnimplementedOpcode(
  const std::string& funName,
  int modtype, int module, int opcode, int overload)
  : Exception("")
{
  ostringstream oss;
  oss << "Undefined opcode \"" << funName << "\", opcode<" << modtype
      << ":" << module << ":" << opcode << ", " << overload << ">";
  description = oss.str();
}

// -----------------------------------------------------------------------

UnimplementedOpcode::UnimplementedOpcode(
  int modtype, int module, int opcode, int overload)
  : Exception("")
{
  ostringstream oss;
  oss << "Undefined opcode<" << modtype << ":" << module << ":" 
      << opcode << ", " << overload << ">";
  description = oss.str();
}

// -----------------------------------------------------------------------

}
