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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// -----------------------------------------------------------------------

#include <boost/program_options.hpp>
#include <boost/filesystem/operations.hpp>

// We include this here because SDL is retarded and works by #define
// main(inat argc, char* agrv[]). Loosers.
#include <SDL/SDL.h>

#include <iostream>
#include <sstream>
#include <string>

#include "MachineBase/RLMachine.hpp"
#include "MachineBase/Serialization.hpp"
#include "Modules/Modules.hpp"
#include "Platforms/gcn/GCNPlatform.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/SoundSystem.hpp"
#include "Systems/Base/SystemError.hpp"
#include "Systems/SDL/SDLSystem.hpp"
#include "Utilities/Exception.hpp"
#include "Utilities/File.hpp"
#include "Utilities/findFontFile.h"
#include "libReallive/gameexe.h"
#include "libReallive/reallive.h"

using namespace std;

namespace po = boost::program_options;
namespace fs = boost::filesystem;

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
 * the fine details of the RealLive system, along with doing most of
 * the really hard reverse engineering work that I'd rather not do.
 *
 * @section Table Table of Contents
 *
 * The documentation is divided into the following sections.
 *
 * - @subpage theProblemDomainOfVisualNovels "The problem Domain of Visual Novels"
 * - @subpage architectureReview "RLVM Architecture"
 */

// -----------------------------------------------------------------------

/**
 * @page theProblemDomainOfVisualNovels The problem Domain of Visual Novels
 *
 * @section Overview
 *
 * Visual Novels (referred to as NVL, AVG or ADV games in Japanese) are
 * a form of interactive fiction in Japan which never really became
 * popular in most English speaking countries. They are simple, plot
 * and character oriented games which are very text-heavy. Gameplay
 * wise, they are comparable to a large slide show with text, images
 * and sound, and can be thought of as massive, more serious, mature
 * versions of the Choose-Your-Own-Adventure series of children's books.
 *
 * @section IsAndIsnt What RLVM is and isn't
 *
 * RLVM is a clone of a specific visual novel interpreter, the
 * RealLive system developed by VisualArts KK. It aims to (eventually)
 * become a compatible, portable interpreter for non-Windows users
 * that will play a large variety of commercial visual novels written
 * in Reallive.
 *
 * RLVM is not intended to compete with VisualArts KK as a development
 * toolkit. While someone could theoretically combine RLVM with <a
 * href="http://www.haeleth.net">Haeleth</a>'s <a
 * href="http://dev.haeleth.net/rldev.shtml">RLdev</a> compiler
 * toolkit to produce games (at least after RLVM supports a base set
 * of operations), it would be overly cumbersome and I would recommend
 * one of the many free visual novel development systems, which would
 * be both easier to use and more featurefull.
 *
 * RLVM is not meant to facilitate piracy. Please buy these games;
 * many people put their hearts into writing these stories and they
 * deserve to be rewarded financially.
 *
 * Finally, RLVM is not a big truck...It's a series of tubes.
 */

// -----------------------------------------------------------------------

/**
 * @page architectureReview RLVM Architecture
 *
 * RLVM is divided into several high level directories:
 *
 * - A modified version of Haeleth's @c libReallive, which is
 *   responsible for reading and parsing the SEEN.TXT file and
 *   creating the corresponding object representation. There is also a
 *   class Gameexe which parses the Gameexe.ini file in every RealLive
 *   game.
 * - The Opcode Definitions / Modules, which can be found in the
 *   subidrectory @c src/Modules/ . These files contain the
 *   definitions for the individual Opcodes.
 * - The core of the virtual machine found in @c src/MachineBase/ :
 *   - RLMachine: the main class which contains all execution state
 *   - RLOperation: the base class of every opcode definition.
 *   - LongOperation: the base class for all operations that persist
 *     for multiple cycles through the game loop.
 * - The Base System classes found in @c src/Systems/Base , which define the
 *   generalized interface for system dependent operations like sound and
 *   graphics. It was originally meant to only define abstract interface which
 *   would be implemented in System/ specific directories. Said base classes
 *   have grown to hold significant cross-platform logic, implemented per
 *   system by subclassing. It has also grown to contain a whole lot of
 *   non-subclassed, cross platform code.
 * - The System subclasses, such as @c src/Systems/SDL , which
 *   implement the Base System interface for SDL. Additional
 *   subclasses could be written for DirectX, or some other game
 *   interface.
 *
 * The above was part of the original version of this document. In the
 * intervening time, I've also added:
 *
 * - LongOperations and Effects: LongOperations are functors that get run
 *   during the mainloop instead of a RealLive instruction. Sometimes execution
 *   of RealLive code needs to stop so a command that would run longer than a
 *   single run through the gameloop can take all the time it needs while still
 *   updating the screen. Effects are graphical LongOperations that transition
 *   between two images.
 * - Encodings: A combination of Haeleth's encoding functions from rlBabel and
 *   various encoding utility functions I've written.
 */

// -----------------------------------------------------------------------

void printVersionInformation() {
  cout
    << "rlvm (" << rlvm_version() <<  ")" << endl
    << "Copyright (C) 2006-2009 Elliot Glaysher, et all." << endl
    << endl
    << "Contains code that is: " << endl
    << "  Copyright (C) 2006-2007 Peter \"Haeleth\" Jolly" << endl
    << "  Copyright (C) 2004-2006 Kazunori \"jagarl\" Ueno" << endl
    << endl
    << "This program is free software: you can redistribute it and/or modify"
    << endl
    << "it under the terms of the GNU General Public License as published by"
    << endl
    << "the Free Software Foundation, either version 3 of the License, or"
    << endl
    << "(at your option) any later version."
    << endl << endl
    << "This program is distributed in the hope that it will be useful,"
    << endl
    << "but WITHOUT ANY WARRANTY; without even the implied warranty of"
    << endl
    << "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the"
    << endl
    << "GNU General Public License for more details."
    << endl << endl
    << "You should have received a copy of the GNU General Public License"
    << endl
    << "along with this program.  If not, see <http://www.gnu.org/licenses/>."
    << endl << endl;
}

// -----------------------------------------------------------------------

void printUsage(const string& name, po::options_description& opts) {
  cout << "Usage: " << name << " [options] <game root>" << endl;
  cout << opts << endl;
}

// -----------------------------------------------------------------------

int main(int argc, char* argv[]) {
  srand(time(NULL));

  // Set global state: allow spaces in game paths
  fs::path::default_name_check(fs::native);

  // -----------------------------------------------------------------------
  // Parse command line options

  // Declare the supported options.
  po::options_description opts("Options");
  opts.add_options()
      ("help", "Produce help message")
      ("help-debug", "Print help message for people working on rlvm")
      ("version", "Display version and license information")
      ("font", po::value<string>(), "Specifies TrueType font to use.");

  po::options_description debugOpts("Debugging Options");
  debugOpts.add_options()
      ("gameexe", po::value<string>(), "Override location of Gameexe.ini")
      ("seen", po::value<string>(), "Override location of SEEN.TXT")
      ("start-seen", po::value<int>(), "Force start at SEEN#")
      ("memory", "Forces debug mode (Sets #MEMORY=1 in the Gameexe.ini file)")
      ("undefined-opcodes", "Display a message on undefined opcodes")
      ("count-undefined",
       "On exit, present a summary table about how many times each undefined "
       "opcode was called");

  // Declare the final option to be game-root
  po::options_description hidden("Hidden");
  hidden.add_options()
      ("game-root", po::value<string>(), "Location of game root");

  po::positional_options_description p;
  p.add("game-root", -1);

  // Use these on the command line
  po::options_description commandLineOpts;
  commandLineOpts.add(opts).add(hidden).add(debugOpts);

  po::variables_map vm;
  po::store(po::basic_command_line_parser<char>(argc, argv).
            options(commandLineOpts).positional(p).run(),
            vm);
  po::notify(vm);

  // -----------------------------------------------------------------------

  po::options_description allOpts("Allowed options");
  allOpts.add(opts).add(debugOpts);

  // -----------------------------------------------------------------------
  // Process command line options
  fs::path gamerootPath, gameexePath, seenPath;

  if (vm.count("help")) {
    printUsage(argv[0], opts);
    return 0;
  }

  if (vm.count("help-debug")) {
    printUsage(argv[0], allOpts);
    return 0;
  }

  if (vm.count("version")) {
    printVersionInformation();
    return 0;
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
    if (correctPathCase(gamerootPath / "Gameexe.ini").empty()) {
      if (!correctPathCase(gamerootPath / "KINETICDATA" /
                           "Gameexe.ini").empty()) {
        gamerootPath /= "KINETICDATA/";
      } else if (!correctPathCase(gamerootPath / "REALLIVEDATA" /
                                  "Gameexe.ini").empty()) {
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

  // --gameexe
  if (vm.count("gameexe"))
    gameexePath = correctPathCase(vm["gameexe"].as<string>());
  else
    gameexePath = correctPathCase(gamerootPath / "Gameexe.ini");

  // --seen
  if (vm.count("seen"))
    seenPath = correctPathCase(vm["seen"].as<string>());
  else
    seenPath = correctPathCase(gamerootPath / "Seen.txt");

  try {
    cerr << "gameexePath: " << gameexePath << endl;
    Gameexe gameexe(gameexePath);
    gameexe("__GAMEPATH") = gamerootPath.file_string();

    if (vm.count("font")) {
      string font = vm["font"].as<string>();
      if (fs::exists(font)) {
        gameexe("__GAMEFONT") = font;
        cerr << "Using custom font " << vm["font"].as<string>() << endl;
      } else {
        cerr << "Couldn't open font file \"" << font << "\"" << endl;
        return -1;
      }
    }

    // Possibly force starting at a different seen
    if (vm.count("start-seen"))
      gameexe("SEEN_START") = vm["start-seen"].as<int>();

    if (vm.count("memory"))
      gameexe("MEMORY") = 1;

    SDLSystem sdlSystem(gameexe);
    libReallive::Archive arc(seenPath.file_string(), gameexe("REGNAME"));
    RLMachine rlmachine(sdlSystem, arc);
    addAllModules(rlmachine);

    // Validate our font file
    fs::path fontFile = findFontFile(sdlSystem);
    if (fontFile.empty() || !fs::exists(fontFile)) {
      cerr << "Could not open font file. Please either: " << endl
           << endl
           << "1) Place a copy of msgothic.ttc in your home directory." << endl
           << "2) Place a copy of msgothic.ttc in \""
           << gamerootPath.file_string() << "\"" << endl
           << "3) Specify an alternate font with the --font option." << endl;
      return -2;
    }

    // Initialize our platform dialogs (we have to do this after
    // looking for a font because we use that font internally).
    boost::shared_ptr<Platform> platform(
        new GCNPlatform(sdlSystem, sdlSystem.graphics().screenRect()));
    sdlSystem.setPlatform(platform);

    if (vm.count("undefined-opcodes"))
      rlmachine.setPrintUndefinedOpcodes(true);

    if (vm.count("count-undefined"))
      rlmachine.recordUndefinedOpcodeCounts();

    Serialization::loadGlobalMemory(rlmachine);
    rlmachine.setHaltOnException(false);

    while (!rlmachine.halted()) {
      // Give SDL a chance to respond to events, redraw the screen,
      // etc.
      sdlSystem.run(rlmachine);

      // Run the rlmachine through another instruction
      rlmachine.executeNextInstruction();
    }

    Serialization::saveGlobalMemory(rlmachine);
  } catch (rlvm::Exception& e) {
    cerr << "Fatal RLVM error: " << e.what() << endl;
    return 1;
  } catch (libReallive::Error& e) {
    cerr << "Fatal libReallive error: " << e.what() << endl;
    return 1;
  } catch (SystemError& e) {
    cerr << "Fatal local system error: " << e.what() << endl;
    return 1;
  } catch (std::exception& e) {
    cout << "Uncaught exception: " << e.what() << endl;
    return 1;
  } catch (const char* e) {
    cout << "Uncaught exception: " << e << endl;
    return 1;
  }

  return 0;
}
