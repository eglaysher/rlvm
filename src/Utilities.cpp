
#include "Utilities.h"
#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/System.hpp"
#include "libReallive/gameexe.h"

#include <string>
#include <iostream>

using namespace std;

string findFile(RLMachine& machine, const std::string& fileName)
{
  // Hack until I do this correctly
//  cerr << "__GAMEPATH: " << machine.system().gameexe()("__GAMEPATH").to_string() << endl;
  string file = machine.system().gameexe()("__GAMEPATH").to_string() + "/g00/" + fileName;
  file += ".g00";
  return file;
}
