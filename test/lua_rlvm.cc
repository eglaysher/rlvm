// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006, 2007 Elliot Glaysher
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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
//
// -----------------------------------------------------------------------

#include <boost/filesystem/operations.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <sstream>
#include <string>

// We include this here because SDL is retarded and works by #define
// main(inat argc, char* agrv[]). Loosers.
#include <SDL2/SDL.h>

#include "libreallive/gameexe.h"
#include "libreallive/reallive.h"
#include "machine/game_hacks.h"
#include "machine/rlmachine.h"
#include "machine/serialization.h"
#include "modules/module_sys_save.h"
#include "modules/modules.h"
#include "script_machine/script_machine.h"
#include "script_machine/script_world.h"
#include "systems/base/event_system.h"
#include "systems/base/graphics_system.h"
#include "systems/base/sound_system.h"
#include "systems/base/system_error.h"
#include "systems/sdl/sdl_system.h"
#include "utilities/exception.h"
#include "utilities/file.h"
#include "utilities/find_font_file.h"

using namespace std;

namespace po = boost::program_options;
namespace fs = boost::filesystem;

// -----------------------------------------------------------------------

void printVersionInformation() {
  cout
      << "lua_rlvm (" << GetRlvmVersionString() << ")" << endl
      << "Copyright (C) 2006-2014 Elliot Glaysher, Haeleth, Jagarl, et all."
      << endl << endl
      << "This program is free software: you can redistribute it and/or modify"
      << endl
      << "it under the terms of the GNU General Public License as published by"
      << endl
      << "the Free Software Foundation, either version 3 of the License, or"
      << endl << "(at your option) any later version." << endl << endl
      << "This program is distributed in the hope that it will be useful,"
      << endl
      << "but WITHOUT ANY WARRANTY; without even the implied warranty of"
      << endl << "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the"
      << endl << "GNU General Public License for more details." << endl << endl
      << "You should have received a copy of the GNU General Public License"
      << endl
      << "along with this program.  If not, see <http://www.gnu.org/licenses/>."
      << endl << endl;
}

// -----------------------------------------------------------------------

void printUsage(const string& name, po::options_description& opts) {
  cout << "Usage: " << name << " [options] <lua script to run> <game root>"
       << endl << opts << endl;
}

// -----------------------------------------------------------------------

int main(int argc, char* argv[]) {
  srand(time(NULL));

  // -----------------------------------------------------------------------
  // Parse command line options

  // Declare the supported options.
  po::options_description opts("Options");
  opts.add_options()("help", "Produce help message")(
      "version", "Display version and license information")(
      "font", po::value<string>(), "Specifies TrueType font to use.")(
      "undefined-opcodes", "Display a message on undefined opcodes")(
      "load-save", po::value<int>(), "Load a saved game on start")(
      "memory", "Forces debug mode (Sets #MEMORY=1 in the Gameexe.ini file)")(
      "count-undefined",
      "On exit, present a summary table about how many times each undefined "
      "opcode was called")(
      "save-on-decision",
      po::value<int>(),
      "Automatically save the game on decision points to the specified save "
      "game slot. Useful while debugging crashes far into a game.")(
      "save-on-decision-counting-from",
      po::value<int>(),
      "Like --save-on-decision, but will increment the save number every "
      "time.");

  // Declare the final option to be game-root
  po::options_description hidden("Hidden");
  hidden.add_options()(
      "script-location", po::value<string>(), "Location of the lua script")(
      "game-root", po::value<string>(), "Location of game root");

  po::positional_options_description p;
  p.add("script-location", 1);
  p.add("game-root", 1);

  // Use these on the command line
  po::options_description commandLineOpts;
  commandLineOpts.add(opts).add(hidden);

  po::variables_map vm;
  po::store(po::basic_command_line_parser<char>(argc, argv)
                .options(commandLineOpts)
                .positional(p)
                .run(),
            vm);
  po::notify(vm);

  // -----------------------------------------------------------------------
  // Process command line options
  fs::path scriptLocation, gamerootPath, gameexePath, seenPath;

  if (vm.count("help")) {
    printUsage(argv[0], opts);
    return 0;
  }

  if (vm.count("version")) {
    printVersionInformation();
    return 0;
  }

  if (vm.count("script-location")) {
    scriptLocation = vm["script-location"].as<string>();

    if (!fs::exists(scriptLocation)) {
      cerr << "ERROR: File '" << gamerootPath << "' does not exist." << endl;
      return -1;
    }
  } else {
    printUsage(argv[0], opts);
    return -1;
  }

  if (vm.count("game-root")) {
    gamerootPath = vm["game-root"].as<string>();

    if (!fs::exists(gamerootPath)) {
      cerr << "ERROR: Path '" << gamerootPath << "' does not exist." << endl;
      return -1;
    }

    if (!fs::is_directory(gamerootPath)) {
      cerr << "ERROR: Path '" << gamerootPath << "' is not a directory."
           << endl;
      return -1;
    }

    // Some games hide data in a lower subdirectory.  A little hack to
    // make these behave as expected...
    if (CorrectPathCase(gamerootPath / "Gameexe.ini").empty()) {
      if (!CorrectPathCase(gamerootPath / "KINETICDATA" / "Gameexe.ini")
               .empty()) {
        gamerootPath /= "KINETICDATA/";
      } else if (!CorrectPathCase(gamerootPath / "REALLIVEDATA" / "Gameexe.ini")
                      .empty()) {
        gamerootPath /= "REALLIVEDATA/";
      } else {
        cerr << "WARNING: Path '" << gamerootPath << "' may not contain a "
             << "RealLive game." << endl;
      }
    }
  } else {
    printUsage(argv[0], opts);
    return -1;
  }

  try {
    gameexePath = CorrectPathCase(gamerootPath / "Gameexe.ini");
    seenPath = CorrectPathCase(gamerootPath / "Seen.txt");

    Gameexe gameexe(gameexePath);
    gameexe("__GAMEPATH") = gamerootPath.string();

    if (vm.count("memory"))
      gameexe("MEMORY") = 1;

    // Run the incoming lua file and do some basic error checking on what it
    // wants us to do.
    ScriptWorld world;

    SDLSystem sdlSystem(gameexe);
    libreallive::Archive arc(seenPath.string(), gameexe("REGNAME"));

    ScriptMachine rlmachine(world, sdlSystem, arc);
    AddAllModules(rlmachine);
    AddGameHacks(rlmachine);
    world.InitializeMachine(rlmachine);
    world.LoadToplevelFile(scriptLocation.string());

    // Make sure we go as fast as possible:
    sdlSystem.set_force_fast_forward();

    if (vm.count("undefined-opcodes"))
      rlmachine.SetPrintUndefinedOpcodes(true);

    if (vm.count("count-undefined"))
      rlmachine.RecordUndefinedOpcodeCounts();

    if (vm.count("save-on-decision")) {
      int decision_num = vm["save-on-decision"].as<int>();
      rlmachine.set_save_on_decision_slot(decision_num);
    }

    if (vm.count("save-on-decision-counting-from")) {
      int start_from = vm["save-on-decision-counting-from"].as<int>();
      rlmachine.set_save_on_decision_slot(start_from);
      rlmachine.set_increment_on_save();
    }

    Serialization::loadGlobalMemory(rlmachine);
    rlmachine.SetHaltOnException(false);

    if (vm.count("load-save")) {
      Sys_load()(rlmachine, vm["load-save"].as<int>());
    }

    while (!rlmachine.halted()) {
      // Give SDL a chance to respond to events, redraw the screen,
      // etc.
      sdlSystem.Run(rlmachine);

      // Run the rlmachine through as many instructions as we can in a 10ms time
      // slice. Bail out if we switch to long operation mode, or if the screen
      // is marked as dirty.
      unsigned int start_ticks = sdlSystem.event().GetTicks();
      unsigned int end_ticks = start_ticks;
      do {
        rlmachine.ExecuteNextInstruction();
        end_ticks = sdlSystem.event().GetTicks();
      } while (!rlmachine.CurrentLongOperation() &&
               !sdlSystem.force_wait() &&
               (end_ticks - start_ticks < 10));

      sdlSystem.set_force_wait(false);
    }

    Serialization::saveGlobalMemory(rlmachine);
  }
  catch (rlvm::Exception& e) {
    cerr << "Fatal RLVM error: " << e.what() << endl;
    return 1;
  }
  catch (libreallive::Error& e) {
    cerr << "Fatal libreallive error: " << e.what() << endl;
    return 1;
  }
  catch (SystemError& e) {
    cerr << "Fatal local system error: " << e.what() << endl;
    return 1;
  }
  catch (std::exception& e) {
    cout << "Uncaught exception: " << e.what() << endl;
    return 1;
  }
  catch (const char* e) {
    cout << "Uncaught exception: " << e << endl;
    return 1;
  }

  return 0;
}
