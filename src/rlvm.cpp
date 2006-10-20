#include "libReallive/reallive.h"
#include "libReallive/gameexe.h"
#include "MachineBase/RLMachine.hpp"

#include "Modules/Module_Jmp.hpp"
#include "Modules/Module_Sys.hpp"
#include "Modules/Module_Str.hpp"
#include "Modules/Module_Mem.hpp"
#include "Modules/Module_Grp.hpp"
#include "Modules/Module_Msg.hpp"

#include "Systems/SDL/SDLSystem.hpp"
#include "Systems/SDL/SDLGraphicsSystem.hpp"
#include "Systems/SDL/SDLEventSystem.hpp"

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
 * @section Table Table of Contents
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
 * memory; most of this code is stolen from Haeleth in the namespace
 * libReallive. This page describes the reading of SEEN files from the
 * file to memory.
 *
 * @section Archive "The Archive and Scenario access"
 *
 * We start with the main class that represents the SEEN.TXT file,
 * libReallive::Archive. When we construct an Archive, we pass in the
 * path to a SEEN.TXT file to load. Currently, the only thing done on
 * startup is the parsing of the TOC, which defines which Scenarios
 * are in the SEEN.TXT archive.
 *
 * From the Archive, we can access libReallive::Scenarios using the
 * libReallive::Archive::scenario() member. This method will return
 * the Scenario relating to the passed in number. Archive has other
 * members for manipulating and rewriting the data, but these aren't
 * used in RLVM.
 *
 * @section Scenario "The Scenario"
 *
 * The libReallive::Scenario class represents a Scenario, a sequence
 * of commands with metadata.
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

    SDLSystem sdlSystem;
    libReallive::Archive arc(argv[1]);
    RLMachine rlmachine(sdlSystem, arc);

    // Attatch the modules for some commands
    rlmachine.attatchModule(new JmpModule);
    rlmachine.attatchModule(new SysModule(sdlSystem.graphics()));
    rlmachine.attatchModule(new StrModule);
    rlmachine.attatchModule(new MemModule);
    rlmachine.attatchModule(new MsgModule);
    rlmachine.attatchModule(new GrpModule);

    while(!rlmachine.halted()) {
      // Give SDL a chance to respond to events, redraw the screen,
      // etc.
      sdlSystem.run(rlmachine);

      // Run the rlmachine through another instruction
      rlmachine.executeNextInstruction();
    }
  }
  catch (libReallive::Error& b) {
    printf("Fatal error: %s\n", b.what());
    return 1;
  }
}
