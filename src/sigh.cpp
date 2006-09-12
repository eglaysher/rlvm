#include "reallive.h"
#include "RLMachine.hpp"
#include "Module_Jmp.hpp"
#include "Module_Sys.hpp"
#include "Module_Str.hpp"
#include "Module_Mem.hpp"

#include <iostream>

using namespace std;

int main(int argc, char* argv[])
{
  srand(time(NULL));

  try {
    Reallive::Archive arc(argv[1]);
    RLMachine rlmachine(arc);

    // Attatch the modules for some commands
    rlmachine.attatchModule(new JmpModule);
    rlmachine.attatchModule(new SysModule);
    rlmachine.attatchModule(new StrModule);
    rlmachine.attatchModule(new MemModule);

    while(!rlmachine.halted()) {
      rlmachine.executeNextInstruction();
    }
  }
  catch (Reallive::Error& b) {
    printf("Fatal error: %s\n", b.what());
    return 1;
  }
}
