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

#include "machine/rlvm_instance.h"

#include <iostream>
#include <string>

#include "libreallive/gameexe.h"
#include "libreallive/reallive.h"
#include "machine/dump_scenario.h"
#include "machine/game_hacks.h"
#include "machine/memory.h"
#include "machine/rlmachine.h"
#include "machine/serialization.h"
#include "modules/module_sys_save.h"
#include "modules/modules.h"
#include "platforms/gcn/gcn_platform.h"
#include "systems/base/event_system.h"
#include "systems/base/graphics_system.h"
#include "systems/base/system_error.h"
#include "systems/sdl/sdl_system.h"
#include "utf8cpp/utf8.h"
#include "utilities/exception.h"
#include "utilities/file.h"
#include "utilities/find_font_file.h"
#include "utilities/gettext.h"
#include "utilities/string_utilities.h"

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
      tracing_(false),
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

    libreallive::Archive arc(seenPath.string(), gameexe("REGNAME"));
    if (dump_seen_ != -1) {
      libreallive::Scenario* scenario = arc.GetScenario(dump_seen_);
      DumpScenario(scenario);
      return;
    }

    SDLSystem sdlSystem(gameexe);
    RLMachine rlmachine(sdlSystem, arc);
    AddAllModules(rlmachine);
    AddGameHacks(rlmachine);

    // Validate our font file
    // TODO(erg): Remove this when we switch to native font selection dialogs.
    fs::path fontFile = FindFontFile(sdlSystem);
    if (fontFile.empty() || !fs::exists(fontFile)) {
      throw rlvm::UserPresentableError(
          _("Could not find msgothic.ttc or a suitable fallback font."),
          _("Please place a copy of msgothic.ttc in either your home directory "
            "or in the game path."));
    }

    // Initialize our platform dialogs (we have to do this after
    // looking for a font because we use that font internally).
    std::shared_ptr<GCNPlatform> platform(
        new GCNPlatform(sdlSystem, sdlSystem.graphics().screen_rect()));
    sdlSystem.SetPlatform(platform);

    if (undefined_opcodes_)
      rlmachine.SetPrintUndefinedOpcodes(true);

    if (count_undefined_copcodes_)
      rlmachine.RecordUndefinedOpcodeCounts();

    if (tracing_)
      rlmachine.set_tracing_on();

    Serialization::loadGlobalMemory(rlmachine);

    // Now to preform a quick integrity check. If the user opened the Japanese
    // version of CLANNAD (or any other game), and then installed a patch, our
    // user data is going to be screwed!
    DoUserNameCheck(rlmachine);

    rlmachine.SetHaltOnException(false);

    if (load_save_ != -1)
      Sys_load()(rlmachine, load_save_);

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

      // Sleep to be nice to the processor and to give the GPU a chance to
      // catch up.
      if (!sdlSystem.ShouldFastForward()) {
        int real_sleep_time = 10 - (end_ticks - start_ticks);
        if (real_sleep_time < 1)
          real_sleep_time = 1;
        sdlSystem.event().Wait(real_sleep_time);
      }

      sdlSystem.set_force_wait(false);
    }

    Serialization::saveGlobalMemory(rlmachine);
  }
  catch (rlvm::UserPresentableError& e) {
    ReportFatalError(e.message_text(), e.informative_text());
  }
  catch (rlvm::Exception& e) {
    ReportFatalError(_("Fatal RLVM error"), e.what());
  }
  catch (libreallive::Error& e) {
    ReportFatalError(_("Fatal libreallive error"), e.what());
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
  std::cerr << message_text << ": " << informative_text << std::endl;
}

void RLVMInstance::DoUserNameCheck(RLMachine& machine) {
  try {
    int encoding = machine.GetProbableEncodingType();

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
  fs::path corrected_path = CorrectPathCase(search_for);
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
    if (fs::exists(CorrectPathCase(gamerootPath / *cur_file))) {
      throw rlvm::UserPresentableError(message_text,
                                       _("rlvm can only play RealLive games."));
    }
  }
}
