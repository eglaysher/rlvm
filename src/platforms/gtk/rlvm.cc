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

// We include this here because SDL is retarded and works by #define
// main(inat argc, char* agrv[]). Loosers.
#include <SDL2/SDL.h>

#include <boost/program_options.hpp>
#include <iostream>
#include <string>

#include "platforms/gtk/gtk_rlvm_instance.h"
#include "systems/base/system.h"
#include "utilities/file.h"

namespace po = boost::program_options;
namespace fs = boost::filesystem;

using std::cerr;
using std::cout;
using std::endl;
using std::string;

// -----------------------------------------------------------------------

void printVersionInformation() {
  cout
      << "rlvm (" << GetRlvmVersionString() << ")" << endl
      << "Copyright (C) 2006-2014 Elliot Glaysher, et all." << endl << endl
      << "Contains code that is: " << endl
      << "  Copyright (C) 2006-2007 Peter \"Haeleth\" Jolly" << endl
      << "  Copyright (C) 2004-2006 Kazunori \"jagarl\" Ueno" << endl << endl
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

void printUsage(const string& name, po::options_description& opts) {
  cout << "Usage: " << name << " [options] <game root>" << endl;
  cout << opts << endl;
}

int main(int argc, char* argv[]) {
  GtkRLVMInstance instance(&argc, &argv);

  // -----------------------------------------------------------------------
  // Parse command line options

  // Declare the supported options.
  po::options_description opts("Options");
  opts.add_options()("help", "Produce help message")(
      "help-debug", "Print help message for people working on rlvm")(
      "version", "Display version and license information")(
      "font", po::value<string>(), "Specifies TrueType font to use.");

  po::options_description debugOpts("Debugging Options");
  debugOpts.add_options()(
      "start-seen", po::value<int>(), "Force start at SEEN#")(
      "dump-seen", po::value<int>(), "Dumps rlvm's internal parsing of SEEN#")(
      "load-save", po::value<int>(), "Load a saved game on start")(
      "memory", "Forces debug mode (Sets #MEMORY=1 in the Gameexe.ini file)")(
      "undefined-opcodes", "Display a message on undefined opcodes")(
      "count-undefined",
      "On exit, present a summary table about how many times each undefined "
      "opcode was called")("trace", "Prints opcodes as they are run)");

  // Declare the final option to be game-root
  po::options_description hidden("Hidden");
  hidden.add_options()(
      "game-root", po::value<string>(), "Location of game root");

  po::positional_options_description p;
  p.add("game-root", -1);

  // Use these on the command line
  po::options_description commandLineOpts;
  commandLineOpts.add(opts).add(hidden).add(debugOpts);

  po::variables_map vm;
  try {
    po::store(po::basic_command_line_parser<char>(argc, argv)
                  .options(commandLineOpts)
                  .positional(p)
                  .run(),
              vm);
    po::notify(vm);
  }
  catch (boost::program_options::multiple_occurrences& e) {
    cerr << "Couldn't parse command line: multiple_occurances." << endl
         << " (Hint: this can happen when your shell doesn't escape properly,"
         << endl
         << "  e.g. \"/path/to/Clannad Full Voice/\" without the quotes.)"
         << endl;
    return -1;
  }
  catch (boost::program_options::error& e) {
    cerr << "Couldn't parse command line: " << e.what() << endl;
    return -1;
  }

  // -----------------------------------------------------------------------

  po::options_description allOpts("Allowed options");
  allOpts.add(opts).add(debugOpts);

  // -----------------------------------------------------------------------
  // Process command line options
  fs::path gamerootPath;

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
    gamerootPath = instance.SelectGameDirectory();
    if (gamerootPath.empty())
      return -1;
  }

  if (vm.count("start-seen"))
    instance.set_seen_start(vm["start-seen"].as<int>());

  if (vm.count("dump-seen"))
    instance.set_dump_seen(vm["dump-seen"].as<int>());

  if (vm.count("memory"))
    instance.set_memory();

  if (vm.count("undefined-opcodes"))
    instance.set_undefined_opcodes();

  if (vm.count("count-undefined"))
    instance.set_count_undefined();

  if (vm.count("trace"))
    instance.set_tracing();

  if (vm.count("load-save"))
    instance.set_load_save(vm["load-save"].as<int>());

  if (vm.count("font"))
    instance.set_custom_font(vm["font"].as<string>());

  instance.Run(gamerootPath);

  return 0;
}
