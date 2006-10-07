#include "reallive.h"
#include "RLMachine.hpp"
#include "Module_Jmp.hpp"
#include "Module_Sys.hpp"
#include "Module_Str.hpp"
#include "Module_Mem.hpp"
#include "Module_Grp.hpp"
#include "gameexe.h"

#include "SDLSystem.hpp"
#include "SDLGraphicsSystem.hpp"

#include <iostream>

using namespace std;

/**
 * @mainpage RLVM, a Reallive virtual machine clone
 *
 * @section Introduction
 *
 * RLVM is a clone of the official Reallive virtual machine, produced
 * by VisualArt's KK, meant to provide Linux and Macintosh users with
 * a compatible, portable interpreter for Realive games.
 *
 * RLVM would not exist if it weren't for the help of Haeleth, and,
 * indirectly, Jagarl, both of who have done amazing jobs documenting
 * the fine details of the RealLive system, along with doing all the
 * really hard reverse engineering work that I'd rather not do.
 *
 * @section Table of Contents
 * 
 * The documentation is divided into the following sections.
 *
 * - @subpage dataIntoMachine "Reading SEEN files into the Machine"
 * - @subpage virtualMachine "The Virtual Machine and its support classes"
 */

// -----------------------------------------------------------------------

/**
 * @page dataIntoMachine Reading SEEN files into the Machine
 *
 * @section Overview
 *
 * There are several classes that represent the SEEN.TXT file in
 * memory; most of this code is stolen from Haeleth.
 */

// -----------------------------------------------------------------------

/**
 * @page virtualMachine The Virtual Machine and its support classes
 *
 * @section Overview 
 *
 * To begin, we have our main virtual machine class
 * RLMachine. RLMachine is responsible for owning the passed in
 * 
 */

int main(int argc, char* argv[])
{
  srand(time(NULL));

  try {
//    Gameexe gamexex(argv[1]);

    // Initialize the sdlSystem
    SDLSystem sdlSystem;

    libReallive::Archive arc(argv[1]);
    RLMachine rlmachine(arc);

    // Attatch the modules for some commands
    rlmachine.attatchModule(new JmpModule);
    rlmachine.attatchModule(new SysModule);
    rlmachine.attatchModule(new StrModule);
    rlmachine.attatchModule(new MemModule);
    rlmachine.attatchModule(new GrpModule);

    while(!rlmachine.halted()) {
      rlmachine.executeNextInstruction();
    }
  }
  catch (libReallive::Error& b) {
    printf("Fatal error: %s\n", b.what());
    return 1;
  }
}
