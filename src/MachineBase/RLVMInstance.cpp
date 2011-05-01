// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2011 Elliot Glaysher
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
// -----------------------------------------------------------------------

#include "MachineBase/RLVMInstance.hpp"

#include <iostream>

#include "MachineBase/GameHacks.hpp"
#include "MachineBase/RLMachine.hpp"
#include "MachineBase/Serialization.hpp"
#include "Modules/Modules.hpp"
#include "Modules/Module_Sys_Save.hpp"
#include "Systems/Base/SystemError.hpp"
#include "Systems/SDL/SDLSystem.hpp"
#include "Utilities/Exception.hpp"
#include "Utilities/File.hpp"
#include "Utilities/findFontFile.h"
#include "libReallive/gameexe.h"
#include "libReallive/reallive.h"

using namespace std;

namespace fs = boost::filesystem;

// AVG32 file checks. We can't run AVG32 games.
const char* avg32_exes[] = {
  "avg3216m.exe",
  "avg3217m.exe",
  NULL
};

// Siglus engine filenames. We can't run VisualArts' newer engine.
const char* siglus_exes[] = {
  "siglus.exe",
  "siglusengine-ch.exe",
  "siglusengine.exe",
  "siglusenginechs.exe",
  NULL
};

RLVMInstance::RLVMInstance()
    : seen_start_(-1),
      memory_(false),
      undefined_opcodes_(false),
      count_undefined_copcodes_(false),
      load_save_(-1) {
  srand(time(NULL));

  // Set global state: allow spaces in game paths
  fs::path::default_name_check(fs::native);
}

RLVMInstance::~RLVMInstance() {}

void RLVMInstance::Run(const boost::filesystem::path& gamerootPath) {
  try {
    fs::path gameexePath = FindGameFile(gamerootPath, "Gameexe.ini");
    fs::path seenPath = FindGameFile(gamerootPath, "Seen.txt");

    // Check for VisualArt's older and newer engines, which we can't emulate:
    CheckBadEngine(gamerootPath, avg32_exes, "Can't run AVG32 games");
    CheckBadEngine(gamerootPath, siglus_exes, "Can't run Siglus games");

    Gameexe gameexe(gameexePath);
    gameexe("__GAMEPATH") = gamerootPath.file_string();

    // Possibly force starting at a different seen
    if (seen_start_ != -1)
      gameexe("SEEN_START") = seen_start_;

    if (memory_)
      gameexe("MEMORY") = 1;

    SDLSystem sdlSystem(gameexe);
    sdlSystem.setPlatform(BuildNativePlatform(sdlSystem));

    libReallive::Archive arc(seenPath.file_string(), gameexe("REGNAME"));
    RLMachine rlmachine(sdlSystem, arc);
    addAllModules(rlmachine);
    addGameHacks(rlmachine);

    // Validate our font file
    // TODO(erg): Remove this when we switch to native font selection dialogs.
    fs::path fontFile = findFontFile(sdlSystem);
    if (fontFile.empty() || !fs::exists(fontFile)) {
      throw rlvm::UserPresentableError(
          "Could not find msgothic.ttc or a suitable fallback font.",
          "Please place a copy of msgothic.ttc in either your home directory "
          "or in the game path.");
    }

    if (undefined_opcodes_)
      rlmachine.setPrintUndefinedOpcodes(true);

    if (count_undefined_copcodes_)
      rlmachine.recordUndefinedOpcodeCounts();

    Serialization::loadGlobalMemory(rlmachine);
    rlmachine.setHaltOnException(false);

    if (load_save_ != -1)
      Sys_load()(rlmachine, load_save_);

    while (!rlmachine.halted()) {
      // Give SDL a chance to respond to events, redraw the screen,
      // etc.
      sdlSystem.run(rlmachine);

      // Run the rlmachine through another instruction
      rlmachine.executeNextInstruction();

      // Maybe process native events that we don't otherwise care about.
      DoNativeWork();
    }

    Serialization::saveGlobalMemory(rlmachine);
  } catch (rlvm::UserPresentableError& e) {
    ReportFatalError(e.message_text(), e.informative_text());
  } catch (rlvm::Exception& e) {
    ReportFatalError("Fatal RLVM error", e.what());
  } catch (libReallive::Error& e) {
    ReportFatalError("Fatal libReallive error", e.what());
  } catch (SystemError& e) {
    ReportFatalError("Fatal local system error", e.what());
  } catch (std::exception& e) {
    ReportFatalError("Uncaught exception", e.what());
  } catch (const char* e) {
    ReportFatalError("Uncaught exception", e);
  }
}

boost::filesystem::path RLVMInstance::SelectGameDirectory() {
  return boost::filesystem::path();
}

void RLVMInstance::ReportFatalError(const std::string& message_text,
                                    const std::string& informative_text) {
  cerr << message_text << ": " << informative_text << endl;
}

Platform* RLVMInstance::BuildNativePlatform(System& system) {
  return NULL;
}

boost::filesystem::path RLVMInstance::FindGameFile(
      const boost::filesystem::path& gamerootPath,
      const std::string& filename) {
  fs::path search_for = gamerootPath / filename;
  fs::path corrected_path = correctPathCase(search_for);
  if (corrected_path.empty()) {
    ostringstream oss;
    oss << "Could not open " << search_for
        << ". Please make sure it exists.";
    throw rlvm::UserPresentableError("Could not load game", oss.str());
  }

  return corrected_path;
}

void RLVMInstance::CheckBadEngine(
    const boost::filesystem::path& gamerootPath,
    const char** filenames,
    const std::string& message_text) {
  for (const char** cur_file = filenames; *cur_file; cur_file++) {
    if (fs::exists(correctPathCase(gamerootPath / *cur_file))) {
      throw rlvm::UserPresentableError(message_text,
                                       "rlvm can only play RealLive games.");
    }
  }
}
