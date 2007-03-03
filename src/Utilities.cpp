// -*-  indent-tabs-mode:nil; c-basic-offset:2  -*-

#include "Utilities.h"
#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/System.hpp"
#include "libReallive/gameexe.h"

#include <string>
#include <iostream>
#include <stack>
#include <cctype>
#include "boost/filesystem/operations.hpp"

using namespace std;

inline void uppercase(string& what)
{
  for (string::size_type i = 0; i < what.size(); ++i) 
    what[i] = toupper(what[i]);
}

string correctPathCase(const string& fileName)
{
#ifndef CASE_SENSITIVE_FILESYSTEM
  return fileName;
#else
  using namespace boost::filesystem;  
  path Path(fileName, native);
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

string findFile(RLMachine& machine, const string& fileName)
{
  // Hack until I do this correctly
//  cerr << "__GAMEPATH: " << machine.system().gameexe()("__GAMEPATH").to_string() << endl;
  string file = machine.system().gameexe()("__GAMEPATH").to_string() + "g00/" + fileName;
  file += ".g00";
  return correctPathCase(file);
}
