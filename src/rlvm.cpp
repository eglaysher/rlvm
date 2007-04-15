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

#include "libReallive/reallive.h"
#include "libReallive/gameexe.h"
#include "MachineBase/RLMachine.hpp"
#include "Systems/SDL/SDLSystem.hpp"
#include "Modules/Modules.hpp"
#include "Utilities.h"

#include <boost/program_options.hpp>
#include <boost/filesystem/operations.hpp>

// We include this here because SDL is retarded and works by #define
// main(inat argc, char* agrv[]). Loosers.
#include <SDL/SDL.h>

#include <iostream>
#include <sstream>

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
 * - @subpage dataIntoMachine "Reading SEEN files into the Machine"
 * - @subpage virtualMachine "The Virtual Machine and its support classes"
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
 * - A list of actors that appear in the scene (referred to as the {@em
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

// -----------------------------------------------------------------------

void printVersionInformation()
{
  const string svnRevision = "$Rev$";
  string number = svnRevision.substr(6);
  stringstream ss;
  ss << number;
  int ver;
  ss >> ver;

  cout
    << "rlvm (svn revision #" << ver << ")" << endl
    << "Copyright (C) 2006,2007 Elliot Glaysher, Haeleth, et all." << endl
    << "This is free software.  You may redistribute copies of it under the terms of"
    << endl
    << "the GNU General Public License <http://www.gnu.org/licenses/gpl.html>."
    << endl
    << "There is NO WARRANTY, to the extent permitted by law."
    << endl;
}

// -----------------------------------------------------------------------

void printUsage(const string& name, po::options_description& opts)
{
  cout << "Usage: " << name << " [options] <game root>" << endl;
  cout << opts << endl;
}

// -----------------------------------------------------------------------


int main(int argc, char* argv[])
{
  srand(time(NULL));

  // Set global state: allow spaces in game paths
  fs::path::default_name_check(fs::native);

  // -----------------------------------------------------------------------
  // Parse command line options

  // Declare the supported options.
  po::options_description opts("Options");
  opts.add_options()
    ("help", "produce help message")
    ("version", "display version and license information")
    ("gameexe", po::value<string>(), "Override location of Gameexe.ini")
    ("seen", po::value<string>(), "Override location of SEEN.TXT")
    ("start-seen", po::value<int>(), "Force start at SEEN#")
    ("memory", "Forces debug mode (Sets #MEMORY=1 in the Gameexe.ini file)")
    ;

  // Declare the final option to be game-root
  po::options_description hidden("Hidden");
  hidden.add_options()
    ("game-root", po::value<string>(), "Location of game root");
  po::positional_options_description p;
  p.add("game-root", -1);

  // Use these on the command line
  po::options_description commandLineOpts;
  commandLineOpts.add(opts).add(hidden);

  po::variables_map vm;
  po::store(po::basic_command_line_parser<char>(argc, argv).
            options(commandLineOpts).positional(p).run(),
            vm);
  po::notify(vm);

  // -----------------------------------------------------------------------
  // Process command line options
  string gamerootPath, gameexePath, seenPath;

  if(vm.count("help"))
  {
    printUsage(argv[0], opts);
    return 0;
  }

  if(vm.count("version"))
  {
    printVersionInformation();
    return 0;
  }

  if(vm.count("game-root"))
  {
    gamerootPath = vm["game-root"].as<string>();

    if(!fs::exists(gamerootPath))
    {
      cerr << "ERROR: Path '" << gamerootPath << "' does not exist." << endl;
      return -1;
    }
    if(!fs::is_directory(gamerootPath))
    {
      cerr << "ERROR: Path '" << gamerootPath << "' is not a directory." << endl;
      return -1;
    }
    if (gamerootPath[gamerootPath.size() - 1] != '/') gamerootPath += "/";

    // Some games hide data in a lower subdirectory.  A little hack to
    // make these behave as expected...
    if (correctPathCase(gamerootPath + "Gameexe.ini") == "") {
      if (correctPathCase(gamerootPath + "KINETICDATA/Gameexe.ini") != "")
        gamerootPath += "KINETICDATA/";
      else if (correctPathCase(gamerootPath + "REALLIVEDATA/Gameexe.ini") != "")
        gamerootPath += "REALLIVEDATA/";
      else
        cerr << "WARNING: Path '" << gamerootPath << "' may not contain a RealLive game." << endl;
    }
  }
  else
  {
    printUsage(argv[0], opts);
    return -1;
  }

  // --gameexe
  if(vm.count("gameexe"))
  {
    gameexePath = vm["gameexe"].as<string>();
  }
  else
  {
    gameexePath = correctPathCase(gamerootPath + "Gameexe.ini");
    cerr << "gameexePath: " << gameexePath << endl;
  }

  // --seen
  if(vm.count("seen"))
  {
    seenPath = vm["seen"].as<string>();
  }
  else
  {
    seenPath = correctPathCase(gamerootPath + "Seen.txt");
  }

  try {
    Gameexe gameexe(gameexePath);
    gameexe("__GAMEPATH") = gamerootPath;

    // Possibly force starting at a different seen
    if(vm.count("start-seen"))
    {
      gameexe("SEEN_START") = vm["start-seen"].as<int>();
    }

    if(vm.count("memory"))
    {
      gameexe("MEMORY") = 1;
    }

    SDLSystem sdlSystem(gameexe);
    libReallive::Archive arc(seenPath);
    RLMachine rlmachine(sdlSystem, arc);
    addAllModules(rlmachine);

    rlmachine.setHaltOnException(false);

    while(!rlmachine.halted()) {
      // Give SDL a chance to respond to events, redraw the screen,
      // etc.
      sdlSystem.run(rlmachine);


      // Run the rlmachine through another instruction
      rlmachine.executeNextInstruction();
    }
  }
  catch (rlvm::Exception& e) {
    cerr << "Fatal RLVM error: " << e.what() << endl;
    return 1;
  }
  catch (libReallive::Error& b) {
    cerr << "Fatal libReallive error: " << b.what() << endl;
    return 1;
  }
  catch(std::exception& e) {
    cout << "Uncaught exception: " << e.what() << endl;
    return 1;
  }
  catch(const char* e) {
    cout << "Uncaught exception: " << e << endl;
    return 1;    
  }

  return 0;
}
