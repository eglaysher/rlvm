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

#include "systems/base/system.h"

#include <boost/algorithm/string.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "libreallive/gameexe.h"
#include "long_operations/load_game_long_operation.h"
#include "machine/long_operation.h"
#include "machine/rlmachine.h"
#include "machine/serialization.h"
#include "modules/module_sys.h"
#include "systems/base/event_system.h"
#include "systems/base/graphics_system.h"
#include "systems/base/platform.h"
#include "systems/base/rlvm_info.h"
#include "systems/base/sound_system.h"
#include "systems/base/system_error.h"
#include "systems/base/text_system.h"
#include "utilities/exception.h"
#include "utilities/string_utilities.h"

using boost::replace_all;
using boost::to_lower;

namespace fs = boost::filesystem;

namespace {

const std::vector<std::string> ALL_FILETYPES = {"g00", "pdt", "anm", "gan",
                                                "hik", "wav", "ogg", "nwa",
                                                "mp3", "ovk", "koe", "nwk"};

struct LoadingGameFromStream : public LoadGameLongOperation {
  LoadingGameFromStream(RLMachine& machine,
                        const std::shared_ptr<std::stringstream>& selection)
      : LoadGameLongOperation(machine), selection_(selection) {}

  virtual void Load(RLMachine& machine) override {
    // We need to copy data here onto the stack because the action of loading
    // will deallocate this object.
    std::shared_ptr<std::stringstream> s = selection_;
    Serialization::loadGameFrom(*s, machine);
    // Warning: |this| is an invalid pointer now.
  }

  std::shared_ptr<std::stringstream> selection_;
};

}  // namespace

// I assume GAN files can't go through the OBJ_FILETYPES path.
const std::vector<std::string> OBJ_FILETYPES = {"anm", "g00", "pdt"};
const std::vector<std::string> IMAGE_FILETYPES = {"g00", "pdt"};
const std::vector<std::string> PDT_IMAGE_FILETYPES = {"pdt"};
const std::vector<std::string> GAN_FILETYPES = {"gan"};
const std::vector<std::string> ANM_FILETYPES = {"anm"};
const std::vector<std::string> HIK_FILETYPES = {"hik", "g00", "pdt"};
const std::vector<std::string> SOUND_FILETYPES = {"wav", "ogg", "nwa", "mp3"};
const std::vector<std::string> KOE_ARCHIVE_FILETYPES = {"ovk", "koe", "nwk"};
const std::vector<std::string> KOE_LOOSE_FILETYPES = {"ogg"};

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
// SystemGlobals
// -----------------------------------------------------------------------

SystemGlobals::SystemGlobals()
    : confirm_save_load_(true), low_priority_(false) {}

// -----------------------------------------------------------------------
// System
// -----------------------------------------------------------------------

System::System()
    : in_menu_(false),
      force_fast_forward_(false),
      force_wait_(false),
      use_western_font_(false) {
  std::fill(syscom_status_,
            syscom_status_ + NUM_SYSCOM_ENTRIES,
            SYSCOM_VISIBLE);
}

System::~System() {}

void System::SetPlatform(const std::shared_ptr<Platform>& platform) {
  platform_ = platform;
}

void System::TakeSelectionSnapshot(RLMachine& machine) {
  previous_selection_.reset(new std::stringstream);
  Serialization::saveGameTo(*previous_selection_, machine);
}

void System::RestoreSelectionSnapshot(RLMachine& machine) {
  // We need to reference this on the stack because it will call
  // System::reset() to get the black screen. (We'll reset again inside
  // LoadingGameFromStream.)
  std::shared_ptr<std::stringstream> s = previous_selection_;
  if (s) {
    // LoadingGameFromStream adds itself to the callstack of |machine| due to
    // subtle timing issues.
    new LoadingGameFromStream(machine, s);
  }
}

int System::IsSyscomEnabled(int syscom) {
  CheckSyscomIndex(syscom, "System::is_syscom_enabled");

  // Special cases where state of the interpreter would override the
  // programmatically set (or user set) values.
  if (syscom == SYSCOM_SET_SKIP_MODE && !text().kidoku_read()) {
    // Skip mode should be grayed out when there's no text to read
    if (syscom_status_[syscom] == SYSCOM_VISIBLE)
      return SYSCOM_GREYED_OUT;
  } else if (syscom == SYSCOM_RETURN_TO_PREVIOUS_SELECTION) {
    if (syscom_status_[syscom] == SYSCOM_VISIBLE)
      return previous_selection_.get() ? SYSCOM_VISIBLE : SYSCOM_GREYED_OUT;
  }

  return syscom_status_[syscom];
}

void System::HideSyscom() {
  std::fill(syscom_status_,
            syscom_status_ + NUM_SYSCOM_ENTRIES,
            SYSCOM_INVISIBLE);
}

void System::HideSyscomEntry(int syscom) {
  CheckSyscomIndex(syscom, "System::hide_system");
  syscom_status_[syscom] = SYSCOM_INVISIBLE;
}

void System::EnableSyscom() {
  std::fill(syscom_status_,
            syscom_status_ + NUM_SYSCOM_ENTRIES,
            SYSCOM_VISIBLE);
}

void System::EnableSyscomEntry(int syscom) {
  CheckSyscomIndex(syscom, "System::enable_system");
  syscom_status_[syscom] = SYSCOM_VISIBLE;
}

void System::DisableSyscom() {
  std::fill(syscom_status_,
            syscom_status_ + NUM_SYSCOM_ENTRIES,
            SYSCOM_GREYED_OUT);
}

void System::DisableSyscomEntry(int syscom) {
  CheckSyscomIndex(syscom, "System::disable_system");
  syscom_status_[syscom] = SYSCOM_GREYED_OUT;
}

int System::ReadSyscom(int syscom) {
  throw rlvm::Exception("ReadSyscom unimplemented!");
}

void System::ShowSyscomMenu(RLMachine& machine) {
  Gameexe& gexe = machine.system().gameexe();

  if (gexe("CANCELCALL_MOD") == 1) {
    if (!in_menu_) {
      // Multiple right clicks shouldn't spawn multiple copies of the menu
      // system on top of each other.
      in_menu_ = true;
      machine.PushLongOperation(new MenuReseter(*this));

      std::vector<int> cancelcall = gexe("CANCELCALL");
      machine.Farcall(cancelcall.at(0), cancelcall.at(1));
    }
  } else if (platform_) {
    platform_->ShowNativeSyscomMenu(machine);
  } else {
    std::cerr << "(We don't deal with non-custom SYSCOM calls yet.)"
              << std::endl;
  }
}

void System::InvokeSyscom(RLMachine& machine, int syscom) {
  switch (syscom) {
    case SYSCOM_SAVE:
      InvokeSaveOrLoad(
          machine, syscom, "SYSTEMCALL_SAVE_MOD", "SYSTEMCALL_SAVE");
      break;
    case SYSCOM_LOAD:
      InvokeSaveOrLoad(
          machine, syscom, "SYSTEMCALL_LOAD_MOD", "SYSTEMCALL_LOAD");
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
        platform_->InvokeSyscomStandardUI(machine, syscom);
      break;
    }
    case SYSCOM_RETURN_TO_PREVIOUS_SELECTION:
      RestoreSelectionSnapshot(machine);
      break;
    case SYSCOM_SHOW_WEATHER:
      graphics().set_should_show_weather(!graphics().should_show_weather());
      break;
    case SYSCOM_SHOW_OBJECT_1:
      graphics().set_should_show_object1(!graphics().should_show_object1());
      break;
    case SYSCOM_SHOW_OBJECT_2:
      graphics().set_should_show_object2(!graphics().should_show_object2());
      break;
    case SYSCOM_CLASSIFY_TEXT:
      std::cerr << "We have no idea what classifying text even means!"
                << std::endl;
      break;
    case SYSCOM_OPEN_MANUAL_PATH:
      std::cerr << "Opening manual path..." << std::endl;
      break;
    case SYSCOM_SET_SKIP_MODE:
      text().SetSkipMode(!text().skip_mode());
      break;
    case SYSCOM_AUTO_MODE:
      text().SetAutoMode(!text().auto_mode());
      break;
    case SYSCOM_MENU_RETURN:
      // This is a hack since we probably have a bunch of crap on the stack.
      machine.ClearLongOperationsOffBackOfStack();

      // Simulate a MenuReturn.
      Sys_MenuReturn()(machine);
      break;
    case SYSCOM_EXIT_GAME:
      machine.Halt();
      break;
    case SYSCOM_SHOW_BACKGROUND:
      graphics().ToggleInterfaceHidden();
      break;
    case SYSCOM_HIDE_MENU:
      // Do nothing. The menu will be hidden on its own.
      break;
    case SYSCOM_GENERIC_1:
    case SYSCOM_GENERIC_2:
    case SYSCOM_SCREEN_MODE:
    case SYSCOM_WINDOW_DECORATION_STYLE:
      std::cerr << "No idea what to do!" << std::endl;
      break;
  }
}

void System::ShowSystemInfo(RLMachine& machine) {
  if (platform_) {
    RlvmInfo info;

    std::string regname = gameexe()("REGNAME").ToString("");
    size_t pos = regname.find('\\');
    if (pos != string::npos) {
      info.game_brand = regname.substr(0, pos);
      info.game_name = regname.substr(pos + 1);
    } else {
      info.game_brand = "";
      info.game_name = regname;
    }

    info.game_version = gameexe()("VERSION_STR").ToString("");
    info.game_path = gameexe()("__GAMEPATH").ToString("");
    info.rlvm_version = GetRlvmVersionString();
    info.rlbabel_loaded = machine.DllLoaded("rlBabel");
    info.text_transformation = machine.GetTextEncoding();

    platform_->ShowSystemInfo(machine, info);
  }
}

boost::filesystem::path System::FindFile(
    const std::string& file_name,
    const std::vector<std::string>& extensions) {
  if (filesystem_cache_.empty())
    BuildFileSystemCache();

  // Hack to get around fileNames like "REALNAME?010", where we only
  // want REALNAME.
  std::string lower_name =
      string(file_name.begin(), find(file_name.begin(), file_name.end(), '?'));
  to_lower(lower_name);

  std::pair<FileSystemCache::const_iterator, FileSystemCache::const_iterator>
      ret = filesystem_cache_.equal_range(lower_name);
  for (const std::string& extension : extensions) {
    for (FileSystemCache::const_iterator it = ret.first; it != ret.second;
         ++it) {
      if (extension == it->second.first) {
        return it->second.second;
      }
    }
  }

  // Error.
  return fs::path();
}

void System::Reset() {
  in_menu_ = false;
  previous_selection_.reset();

  EnableSyscom();

  sound().Reset();
  graphics().Reset();
  text().Reset();
}

std::string System::Regname() {
  Gameexe& gexe = gameexe();
  std::string regname = gexe("REGNAME");
  replace_all(regname, "\\", "_");

  // Note that we assume the Gameexe file is written in Shift-JIS. I don't
  // think you can write it in anything else.
  return cp932toUTF8(regname, 0);
}

boost::filesystem::path System::GameSaveDirectory() {
  fs::path base_dir = GetHomeDirectory() / ".rlvm" / Regname();
  fs::create_directories(base_dir);

  return base_dir;
}

bool System::ShouldFastForward() {
  return (event().CtrlPressed() && text().ctrl_key_skip()) ||
         text().CurrentlySkipping() || force_fast_forward_;
}

void System::DumpRenderTree(RLMachine& machine) {
  std::ostringstream oss;
  oss << "Dump_SEEN" << std::setw(4) << std::setfill('0')
      << machine.SceneNumber() << "_Line" << machine.line_number() << ".txt";

  std::ofstream tree(oss.str().c_str());
  graphics().Refresh(&tree);
}

boost::filesystem::path System::GetHomeDirectory() {
  std::string drive, home;
  char* homeptr = getenv("HOME");
  char* driveptr = getenv("HOMEDRIVE");
  char* homepathptr = getenv("HOMEPATH");
  char* profileptr = getenv("USERPROFILE");
  if (homeptr != 0 && (home = homeptr) != "") {
    // UN*X like home directory
    return fs::path(home);
  } else if (driveptr != 0 && homepathptr != 0 && (drive = driveptr) != "" &&
             (home = homepathptr) != "") {
    // Windows.
    return fs::path(drive) / fs::path(home);
  } else if (profileptr != 0 && (home = profileptr) != "") {
    // Windows?
    return fs::path(home);
  } else {
    throw SystemError("Could not find location of home directory.");
  }
}

void System::InvokeSaveOrLoad(RLMachine& machine,
                              int syscom,
                              const std::string& mod_key,
                              const std::string& location) {
  GameexeInterpretObject save_mod = gameexe()(mod_key);
  GameexeInterpretObject save_loc = gameexe()(location);

  if (save_mod.Exists() && save_loc.Exists() && save_mod == 1) {
    std::vector<int> raw_ints = save_loc;
    int scenario = raw_ints.at(0);
    int entrypoint = raw_ints.at(1);

    text().set_system_visible(false);
    machine.PushLongOperation(new RestoreTextSystemVisibility);
    machine.Farcall(scenario, entrypoint);
  } else if (platform_) {
    platform_->InvokeSyscomStandardUI(machine, syscom);
  }
}

void System::CheckSyscomIndex(int index, const char* function) {
  if (index < 0 || index >= NUM_SYSCOM_ENTRIES) {
    std::ostringstream oss;
    oss << "Illegal syscom index #" << index << " in " << function;
    throw std::runtime_error(oss.str());
  }
}

void System::BuildFileSystemCache() {
  // First retrieve all the directories defined in the #FOLDNAME section.
  std::vector<std::string> valid_directories;
  Gameexe& gexe = gameexe();
  GameexeFilteringIterator it = gexe.filtering_begin("FOLDNAME");
  GameexeFilteringIterator end = gexe.filtering_end();
  for (; it != end; ++it) {
    std::string dir = it->ToString();
    if (!dir.empty()) {
      to_lower(dir);
      valid_directories.push_back(dir);
    }
  }

  fs::path gamepath(gexe("__GAMEPATH").ToString());
  fs::directory_iterator dir_end;
  for (fs::directory_iterator dir(gamepath); dir != dir_end; ++dir) {
    if (fs::is_directory(dir->status())) {
      std::string lowername = dir->path().filename().string();
      to_lower(lowername);
      if (find(valid_directories.begin(), valid_directories.end(), lowername) !=
          valid_directories.end()) {
        AddDirectoryToCache(dir->path());
      }
    }
  }
}

void System::AddDirectoryToCache(const fs::path& directory) {
  fs::directory_iterator dir_end;
  for (fs::directory_iterator dir(directory); dir != dir_end; ++dir) {
    if (fs::is_directory(dir->status())) {
      AddDirectoryToCache(dir->path());
    } else {
      std::string extension = dir->path().extension().string();
      if (extension.size() > 1 && extension[0] == '.')
        extension = extension.substr(1);
      to_lower(extension);

      if (find(ALL_FILETYPES.begin(), ALL_FILETYPES.end(), extension) !=
          ALL_FILETYPES.end()) {
        std::string stem = dir->path().stem().string();
        to_lower(stem);

        filesystem_cache_.emplace(stem, make_pair(extension, dir->path()));
      }
    }
  }
}

std::string GetRlvmVersionString() { return "Version 0.14"; }
