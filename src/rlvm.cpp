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
 * - @subpage theProblemDomainOfVisualNovels "The problem Domain of Visual Novels"
 * - @subpage dataIntoMachine "Reading SEEN files into the Machine"
 * - @subpage virtualMachine "The Virtual Machine and its support classes"
 */

// -----------------------------------------------------------------------

/**
 * @page theProblemDomainOfVisualNovels The problem Domain of Visual Novels
 * 
 * @section Overview
 *
 * Visual Novels (refered to as NVL, AVG or ADV games in Japanese) are
 * a form of interactive fiction in Japan which never really became
 * popular in most English speaking countries. They are simple, plot
 * and character oriented games which are very text-heavy. Gameplay
 * wise, they are comporable to a large slide show with text, images
 * and sound.
 * 
 * @section IsAndIsnt What RLVM is and isn't
 *
 * RLVM is a clone of a specific visual novel interpreter, the
 * RealLive system developed by VisualArts KK. It aims to (eventually)
 * become a compatible, portable interpreter for non-Windows users
 * that will play a large variety of commercial visual novels written
 * in Reallive.
 *
 * RLVM is not intended to to compete with VisualArts KK as a . While
 * someone could theoretically combine RLVM with <a
 * href="http://www.haeleth.net">Haeleth</a>'s <a
 * href="http://dev.haeleth.net/rldev.shtml">RLdev</a> compiler
 * toolkit to produce games (at least after RLVM supports a base set
 * of operations), it would be overly cumbersome and I would recommend
 * one of the many free visual novel development systems, which would
 * be both easier to use and more featurefull.
 *
 * RLVM is not a big truck.
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
 * libReallive::Archive. A SEEN.TXT file contains all of the executed
 * code in a Reallive game (barring DLL extensions to the Reallive
 * system). A SEEN.TXT file contains number identified Scenarios,
 * which represents small pieces of bytecode which are executed in our
 * virtual machine. When we construct an Archive, we pass in the
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
 * of commands and other metadata. It is divided into the
 * libReallive::Header and libReallive::Script. The header contains:
 *
 * - Debug information
 * - "Misc settings"
 * - A list of actors that appear in the scene (refered to as the {@em
 *   dramatic personae} table, which is used for debugging
 * - Metadata which can be added by Haeleth's <a
 *   href="http://dev.haeleth.net/rldev.shtml">RLdev</a> compiler.
 *
 * The Script contains:
 *
 * - A sequence of semi-parsed/tokenized bytecode elements, which are
 *   the elements that RLMachine executes.
 * - A list of entrypoints into the scenario
 * - A list of pointers (for goto, et cetera)
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

    Gameexe gameexe("Gameexe.ini");
    SDLSystem sdlSystem(gameexe);
    libReallive::Archive arc(argv[1]);
    RLMachine rlmachine(sdlSystem, arc);
    cerr << "Past initialization!" << endl;
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
