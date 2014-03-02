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

#include "MachineBase/DumpScenario.hpp"
#include "MachineBase/GameHacks.hpp"
#include "MachineBase/Memory.hpp"
#include "MachineBase/RLMachine.hpp"
#include "MachineBase/Serialization.hpp"
#include "Modules/Modules.hpp"
#include "Modules/Module_Sys_Save.hpp"
#include "Platforms/gcn/GCNPlatform.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/SystemError.hpp"
#include "Systems/SDL/SDLSystem.hpp"
#include "Utilities/Exception.hpp"
#include "Utilities/File.hpp"
#include "Utilities/findFontFile.h"
#include "Utilities/gettext.h"
#include "Utilities/StringUtilities.hpp"
#include "libReallive/gameexe.h"
#include "libReallive/reallive.h"
#include "utf8cpp/utf8.h"

using namespace std;

namespace fs = boost::filesystem;

// AVG32 file checks. We can't run AVG32 games.
const char* avg32_exes[] = {"avg3216m.exe", "avg3217m.exe", NULL};

// Siglus engine filenames. We can't run VisualArts' newer engine.
const char* siglus_exes[] = {"siglus.exe",       "siglusengine-ch.exe",
                             "siglusengine.exe", "siglusenginechs.exe",
                             NULL};

RLVMInstance::RLVMInstance()
    : seen_start_(-1),
      memory_(false),
      undefined_opcodes_(false),
      count_undefined_copcodes_(false),
      load_save_(-1),
      dump_seen_(-1) {
  srand(time(NULL));
}

RLVMInstance::~RLVMInstance() {}

void RLVMInstance::Run(const boost::filesystem::path& gamerootPath) {
  try {
    fs::path gameexePath = FindGameFile(gamerootPath, "Gameexe.ini");
    fs::path seenPath = FindGameFile(gamerootPath, "Seen.txt");

    // Check for VisualArt's older and newer engines, which we can't emulate:
    CheckBadEngine(gamerootPath, avg32_exes, _("Can't run AVG32 games"));
    CheckBadEngine(gamerootPath, siglus_exes, _("Can't run Siglus games"));

    Gameexe gameexe(gameexePath);
    gameexe("__GAMEPATH") = gamerootPath.string();

    // Possibly force starting at a different seen
    if (seen_start_ != -1)
      gameexe("SEEN_START") = seen_start_;

    if (memory_)
      gameexe("MEMORY") = 1;

    if (!custom_font_.empty()) {
      if (!fs::exists(custom_font_)) {
        throw rlvm::UserPresentableError(
            _("Could not open font file."),
            _("Please make sure the font file specified with --font exists and "
              "is a TrueType font."));
      }

      gameexe("__GAMEFONT") = custom_font_;
    }

    libReallive::Archive arc(seenPath.string(), gameexe("REGNAME"));
    if (dump_seen_ != -1) {
      libReallive::Scenario* scenario = arc.scenario(dump_seen_);
      DumpScenario(scenario);
      return;
    }

    SDLSystem sdlSystem(gameexe);
    RLMachine rlmachine(sdlSystem, arc);
    addAllModules(rlmachine);
    addGameHacks(rlmachine);

    // Validate our font file
    // TODO(erg): Remove this when we switch to native font selection dialogs.
    fs::path fontFile = findFontFile(sdlSystem);
    if (fontFile.empty() || !fs::exists(fontFile)) {
      throw rlvm::UserPresentableError(
          _("Could not find msgothic.ttc or a suitable fallback font."),
          _("Please place a copy of msgothic.ttc in either your home directory "
            "or in the game path."));
    }

    // Initialize our platform dialogs (we have to do this after
    // looking for a font because we use that font internally).
    boost::shared_ptr<Platform> platform(
        new GCNPlatform(sdlSystem, sdlSystem.graphics().screenRect()));
    sdlSystem.setPlatform(platform);

    if (undefined_opcodes_)
      rlmachine.setPrintUndefinedOpcodes(true);

    if (count_undefined_copcodes_)
      rlmachine.recordUndefinedOpcodeCounts();

    Serialization::loadGlobalMemory(rlmachine);

    // Now to preform a quick integrity check. If the user opened the Japanese
    // version of CLANNAD (or any other game), and then installed a patch, our
    // user data is going to be screwed!
    DoUserNameCheck(rlmachine);

    rlmachine.setHaltOnException(false);

    if (load_save_ != -1)
      Sys_load()(rlmachine, load_save_);

    while (!rlmachine.halted()) {
      // Give SDL a chance to respond to events, redraw the screen,
      // etc.
      sdlSystem.run(rlmachine);

      // Run the rlmachine through another instruction
      rlmachine.executeNextInstruction();
    }

    Serialization::saveGlobalMemory(rlmachine);
  }
  catch (rlvm::UserPresentableError& e) {
    ReportFatalError(e.message_text(), e.informative_text());
  }
  catch (rlvm::Exception& e) {
    ReportFatalError(_("Fatal RLVM error"), e.what());
  }
  catch (libReallive::Error& e) {
    ReportFatalError(_("Fatal libReallive error"), e.what());
  }
  catch (SystemError& e) {
    ReportFatalError(_("Fatal local system error"), e.what());
  }
  catch (std::exception& e) {
    ReportFatalError(_("Uncaught exception"), e.what());
  }
  catch (const char* e) {
    ReportFatalError(_("Uncaught exception"), e);
  }
}

boost::filesystem::path RLVMInstance::SelectGameDirectory() {
  return boost::filesystem::path();
}

void RLVMInstance::ReportFatalError(const std::string& message_text,
                                    const std::string& informative_text) {
  cerr << message_text << ": " << informative_text << endl;
}

void RLVMInstance::DoUserNameCheck(RLMachine& machine) {
  try {
    int encoding = machine.getProbableEncodingType();

    // Iterate over all the names in both global and local memory banks.
    GlobalMemory& g = machine.memory().global();
    for (int i = 0; i < SIZE_OF_NAME_BANK; ++i)
      cp932toUTF8(g.global_names[i], encoding);

    LocalMemory& l = machine.memory().local();
    for (int i = 0; i < SIZE_OF_NAME_BANK; ++i)
      cp932toUTF8(l.local_names[i], encoding);
  }
  catch (...) {
    // We've failed to interpret one of the name strings as a string in the
    // text encoding of the current native encoding. We're going to fail to
    // display any line that refers to the player's name.
    //
    // That's obviously bad and there's no real way to recover from this so
    // just reset all of global memory.
    if (AskUserPrompt(
            _("Corrupted global memory"),
            _("You appear to have run this game without a translation patch "
              "previously. This can cause lines of text to not print."),
            _("Reset"),
            _("Continue with broken names"))) {
      machine.HardResetMemory();
    }
  }
}

boost::filesystem::path RLVMInstance::FindGameFile(
    const boost::filesystem::path& gamerootPath,
    const std::string& filename) {
  fs::path search_for = gamerootPath / filename;
  fs::path corrected_path = correctPathCase(search_for);
  if (corrected_path.empty()) {
    throw rlvm::UserPresentableError(
        _("Could not load game"),
        str(format(_("Could not open %1%. Please make sure it exists.")) %
            search_for));
  }

  return corrected_path;
}

void RLVMInstance::CheckBadEngine(const boost::filesystem::path& gamerootPath,
                                  const char** filenames,
                                  const std::string& message_text) {
  for (const char** cur_file = filenames; *cur_file; cur_file++) {
    if (fs::exists(correctPathCase(gamerootPath / *cur_file))) {
      throw rlvm::UserPresentableError(message_text,
                                       _("rlvm can only play RealLive games."));
    }
  }
}
