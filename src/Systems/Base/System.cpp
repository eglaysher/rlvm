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

#include "Systems/Base/System.hpp"

#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "MachineBase/LongOperation.hpp"
#include "MachineBase/RLMachine.hpp"
#include "Modules/Module_Sys.hpp"
#include "Systems/Base/EventSystem.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/Platform.hpp"
#include "Systems/Base/RlvmInfo.hpp"
#include "Systems/Base/SoundSystem.hpp"
#include "Systems/Base/SystemError.hpp"
#include "Systems/Base/TextSystem.hpp"
#include "Utilities/Exception.hpp"
#include "libReallive/gameexe.h"

using namespace std;
using boost::bind;
using boost::replace_all;

namespace fs = boost::filesystem;

// -----------------------------------------------------------------------
// SystemGlobals
// -----------------------------------------------------------------------

SystemGlobals::SystemGlobals()
  : confirm_save_load_(true), low_priority_(false) {}

// -----------------------------------------------------------------------
// System
// -----------------------------------------------------------------------

System::System()
    : in_menu_(false), force_fast_forward_(false), force_wait_(false) {
  fill(syscom_status_, syscom_status_ + NUM_SYSCOM_ENTRIES, SYSCOM_VISIBLE);
}

// -----------------------------------------------------------------------

void System::checkSyscomIndex(int index, const char* function) {
  if (index < 0 || index >= NUM_SYSCOM_ENTRIES) {
    ostringstream oss;
    oss << "Illegal syscom index #" << index << " in " << function;
    throw std::runtime_error(oss.str());
  }
}

// -----------------------------------------------------------------------

int System::isSyscomEnabled(int syscom) {
  checkSyscomIndex(syscom, "System::is_syscom_enabled");

  // Special cases where state of the interpreter would override the
  // programmatically set (or user set) values.
  if (syscom == SYSCOM_SET_SKIP_MODE && !text().kidokuRead()) {
    // Skip mode should be grayed out when there's no text to read
    if (syscom_status_[syscom] == SYSCOM_VISIBLE)
      return SYSCOM_GREYED_OUT;
  } else if (syscom == SYSCOM_RETURN_TO_PREVIOUS_SELECTION) {
    // So far, we don't have an implementation for this, so grey it out all the
    // time.
    if (syscom_status_[syscom] == SYSCOM_VISIBLE)
      return SYSCOM_GREYED_OUT;
  }

  return syscom_status_[syscom];
}

// -----------------------------------------------------------------------

void System::hideSyscom() {
  fill(syscom_status_, syscom_status_ + NUM_SYSCOM_ENTRIES, SYSCOM_INVISIBLE);
}

// -----------------------------------------------------------------------

void System::hideSyscomEntry(int syscom) {
  checkSyscomIndex(syscom, "System::hide_system");
  syscom_status_[syscom] = SYSCOM_INVISIBLE;
}

// -----------------------------------------------------------------------

void System::enableSyscom() {
  fill(syscom_status_, syscom_status_ + NUM_SYSCOM_ENTRIES, SYSCOM_VISIBLE);
}

// -----------------------------------------------------------------------

void System::enableSyscomEntry(int syscom) {
  checkSyscomIndex(syscom, "System::enable_system");
  syscom_status_[syscom] = SYSCOM_VISIBLE;
}

// -----------------------------------------------------------------------

void System::disableSyscom() {
  fill(syscom_status_, syscom_status_ + NUM_SYSCOM_ENTRIES, SYSCOM_GREYED_OUT);
}

// -----------------------------------------------------------------------

void System::disableSyscomEntry(int syscom) {
  checkSyscomIndex(syscom, "System::disable_system");
  syscom_status_[syscom] = SYSCOM_GREYED_OUT;
}

// -----------------------------------------------------------------------

int System::readSyscom(int syscom) {
  throw rlvm::Exception("ReadSyscom unimplemented!");
}

// -----------------------------------------------------------------------

class MenuReseter : public LongOperation {
 public:
  explicit MenuReseter(System& sys) : sys_(sys) {}

  bool operator()(RLMachine& machine) {
    sys_.in_menu_ = false;
    return true;
  }

 private:
  System& sys_;
};

// -----------------------------------------------------------------------

void System::showSyscomMenu(RLMachine& machine) {
  Gameexe& gexe = machine.system().gameexe();

  if (gexe("CANCELCALL_MOD") == 1) {
    if (!in_menu_) {
      // Multiple right clicks shouldn't spawn multiple copies of the menu
      // system on top of each other.
      in_menu_ = true;
      machine.pushLongOperation(new MenuReseter(*this));

      vector<int> cancelcall = gexe("CANCELCALL");
      machine.farcall(cancelcall.at(0), cancelcall.at(1));
    }
  } else if (platform_) {
    platform_->showNativeSyscomMenu(machine);
  } else {
    cerr << "(We don't deal with non-custom SYSCOM calls yet.)" << endl;
  }
}

// -----------------------------------------------------------------------

void System::invokeSyscom(RLMachine& machine, int syscom) {
  switch (syscom) {
  case SYSCOM_SAVE:
    invokeSaveOrLoad(machine, syscom, "SYSTEMCALL_SAVE_MOD", "SYSTEMCALL_SAVE");
    break;
  case SYSCOM_LOAD:
    invokeSaveOrLoad(machine, syscom, "SYSTEMCALL_LOAD_MOD", "SYSTEMCALL_LOAD");
    break;
  case SYSCOM_MESSAGE_SPEED:
  case SYSCOM_WINDOW_ATTRIBUTES:
  case SYSCOM_VOLUME_SETTINGS:
  case SYSCOM_MISCELLANEOUS_SETTINGS:
  case SYSCOM_VOICE_SETTINGS:
  case SYSCOM_FONT_SELECTION:
  case SYSCOM_BGM_FADE:
  case SYSCOM_BGM_SETTINGS:
  case SYSCOM_AUTO_MODE_SETTINGS:
  case SYSCOM_USE_KOE:
  case SYSCOM_DISPLAY_VERSION: {
    if (platform_)
      platform_->invokeSyscomStandardUI(machine, syscom);
    break;
  }
  case SYSCOM_RETURN_TO_PREVIOUS_SELECTION:
    cerr << "Implement return to previous selection later!" << endl;
    break;
  case SYSCOM_SHOW_WEATHER:
    graphics().setShowWeather(!graphics().showWeather());
    break;
  case SYSCOM_SHOW_OBJECT_1:
    graphics().setShowObject1(!graphics().showObject1());
    break;
  case SYSCOM_SHOW_OBJECT_2:
    graphics().setShowObject2(!graphics().showObject2());
    break;
  case SYSCOM_CLASSIFY_TEXT:
    cerr << "We have no idea what classifying text even means!" << endl;
    break;
  case SYSCOM_OPEN_MANUAL_PATH:
    cerr << "Opening manual path..." << endl;
    break;
  case SYSCOM_SET_SKIP_MODE:
    text().setSkipMode(!text().skipMode());
    break;
  case SYSCOM_AUTO_MODE:
    text().setAutoMode(!text().autoMode());
    break;
  case SYSCOM_MENU_RETURN:
    // This is a hack since we probably have a bunch of crap on the stack.
    machine.clearLongOperationsOffBackOfStack();

    // Simulate a MenuReturn.
    Sys_MenuReturn()(machine);
    break;
  case SYSCOM_EXIT_GAME:
    machine.halt();
    break;
  case SYSCOM_SHOW_BACKGROUND:
    graphics().toggleInterfaceHidden();
    break;
  case SYSCOM_GENERIC_1:
  case SYSCOM_GENERIC_2:
  case SYSCOM_SCREEN_MODE:
  case SYSCOM_WINDOW_DECORATION_STYLE:
  case SYSCOM_HIDE_MENU:
    cerr << "No idea what to do!" << endl;
    break;
  };
}

// -----------------------------------------------------------------------

void System::showSystemInfo(RLMachine& machine) {
  if (platform_) {
    RlvmInfo info;

    string regname = gameexe()("REGNAME").to_string("");
    size_t pos = regname.find('\\');
    if (pos != string::npos) {
      info.game_brand = regname.substr(0, pos);
      info.game_name = regname.substr(pos + 1);
    } else {
      info.game_brand = "";
      info.game_name = regname;
    }

    info.game_version = gameexe()("VERSION_STR").to_string("");
    info.game_path = gameexe()("__GAMEPATH").to_string("");
    info.rlvm_version = rlvm_version();
    info.rlbabel_loaded = machine.dllLoaded("rlBabel");
    info.text_transformation = machine.getTextEncoding();

    platform_->showSystemInfo(machine, info);
  }
}

// -----------------------------------------------------------------------

void System::invokeSaveOrLoad(RLMachine& machine,
                              int syscom,
                              const std::string& mod_key,
                              const std::string& location) {
  GameexeInterpretObject save_mod = gameexe()(mod_key);
  GameexeInterpretObject save_loc = gameexe()(location);

  if (save_mod.exists() && save_loc.exists() && save_mod == 1) {
    vector<int> raw_ints = save_loc;
    int scenario = raw_ints.at(0);
    int entrypoint = raw_ints.at(1);

    text().setSystemVisible(false);
    machine.pushLongOperation(new RestoreTextSystemVisibility);
    machine.farcall(scenario, entrypoint);
  } else if (platform_) {
    platform_->invokeSyscomStandardUI(machine, syscom);
  }
}

// -----------------------------------------------------------------------

void System::addPath(GameexeInterpretObject gio) {
  boost::filesystem::path gamepath(gameexe()("__GAMEPATH").to_string());
  gamepath /= gio.to_string();
  cached_search_paths.push_back(gamepath);
}

// -----------------------------------------------------------------------

const std::vector<boost::filesystem::path>& System::getSearchPaths() {
  if (cached_search_paths.size() == 0) {
    Gameexe& gexe = gameexe();

    // This *can't* be rewritten as a for_each + bind because of the
    // forwarding problem. See
    // http://www.boost.org/libs/bind/bind.html#Limitations.
    GameexeFilteringIterator it = gexe.filtering_begin("FOLDNAME");
    GameexeFilteringIterator end = gexe.filtering_end();
    for (; it != end; ++it)
      addPath(*it);
  }

  return cached_search_paths;
}

// -----------------------------------------------------------------------

void System::reset() {
  in_menu_ = false;

  enableSyscom();

  sound().reset();
  graphics().reset();
  text().reset();
}

// -----------------------------------------------------------------------

boost::filesystem::path System::getHomeDirectory() {
  string drive, home;
  char *homeptr     = getenv("HOME");
  char *driveptr    = getenv("HOMEDRIVE");
  char *homepathptr = getenv("HOMEPATH");
  char *profileptr  = getenv("USERPROFILE");
  if (homeptr != 0 && (home = homeptr) != "") {
    // UN*X like home directory
    return fs::path(home);
  } else if (driveptr != 0 &&
             homepathptr !=0 &&
             (drive = driveptr) != "" &&
             (home  = homepathptr) != "") {
    // Windows.
    return fs::path(drive) / fs::path(home);
  } else if (profileptr != 0 && (home = profileptr) != "") {
    // Windows?
    return fs::path(home);
  } else {
    throw SystemError("Could not find location of home directory.");
  }
}

// -----------------------------------------------------------------------

std::string System::regname() {
  Gameexe& gexe = gameexe();
  string regname = gexe("REGNAME");
  replace_all(regname, "\\", "_");
  return regname;
}

// -----------------------------------------------------------------------

boost::filesystem::path System::gameSaveDirectory() {
  fs::path base_dir = getHomeDirectory() / ".rlvm" / regname();
  fs::create_directories(base_dir);

  return base_dir;
}

// -----------------------------------------------------------------------

bool System::fastForward() {
  return (event().ctrlPressed() && text().ctrlKeySkip()) ||
    text().currentlySkipping() ||
    force_fast_forward_;
}

// -----------------------------------------------------------------------

void System::dumpRenderTree(RLMachine& machine) {
  ostringstream oss;
  oss << "Dump_SEEN" << setw(4) << setfill('0') << machine.sceneNumber()
      << "_Line" << machine.lineNumber() << ".txt";

  ofstream tree(oss.str().c_str());
  graphics().refresh(&tree);
}

// -----------------------------------------------------------------------

std::string rlvm_version() {
  return "Version 0.7";
}
