// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2007 Elliot Glaysher
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

#include "modules/module_sys_save.h"

#include <algorithm>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <limits>
#include <string>

#include "long_operations/load_game_long_operation.h"
#include "machine/general_operations.h"
#include "machine/long_operation.h"
#include "machine/memory.h"
#include "machine/rlmachine.h"
#include "machine/rlmodule.h"
#include "machine/rloperation.h"
#include "machine/rloperation/argc_t.h"
#include "machine/rloperation/complex_t.h"
#include "machine/rloperation/rlop_store.h"
#include "machine/rloperation/references.h"
#include "machine/rloperation/special_t.h"
#include "machine/save_game_header.h"
#include "machine/serialization.h"
#include "systems/base/colour.h"
#include "systems/base/surface.h"
#include "systems/base/system.h"
#include "libreallive/intmemref.h"
#include "utf8cpp/utf8.h"

// For copy_n, which isn't part of the C++ standard and doesn't come on
// OSX.
#include <boost/multi_array/algorithm.hpp>

using namespace libreallive;
using boost::lexical_cast;
using boost::starts_with;
using boost::ends_with;
namespace fs = boost::filesystem;
using std::get;

// -----------------------------------------------------------------------

namespace {

struct SaveExists : public RLOp_Store_1<IntConstant_T> {
  int operator()(RLMachine& machine, int slot) {
    fs::path saveFile = Serialization::buildSaveGameFilename(machine, slot);
    return fs::exists(saveFile) ? 1 : 0;
  }
};

struct SaveDate : public RLOp_Store_5<IntConstant_T,
                                      IntReference_T,
                                      IntReference_T,
                                      IntReference_T,
                                      IntReference_T> {
  int operator()(RLMachine& machine,
                 int slot,
                 IntReferenceIterator yIt,
                 IntReferenceIterator mIt,
                 IntReferenceIterator dIt,
                 IntReferenceIterator wdIt) {
    int fileExists = SaveExists()(machine, slot);

    if (fileExists) {
      SaveGameHeader header = Serialization::loadHeaderForSlot(machine, slot);

      *yIt = header.save_time.date().year();
      *mIt = header.save_time.date().month();
      *dIt = header.save_time.date().day();
      *wdIt = header.save_time.date().day_of_week();
    }

    return fileExists;
  }
};

struct SaveTime : public RLOp_Store_5<IntConstant_T,
                                      IntReference_T,
                                      IntReference_T,
                                      IntReference_T,
                                      IntReference_T> {
  int operator()(RLMachine& machine,
                 int slot,
                 IntReferenceIterator hhIt,
                 IntReferenceIterator mmIt,
                 IntReferenceIterator ssIt,
                 IntReferenceIterator msIt) {
    int fileExists = SaveExists()(machine, slot);

    if (fileExists) {
      SaveGameHeader header = Serialization::loadHeaderForSlot(machine, slot);

      *hhIt = header.save_time.time_of_day().hours();
      *mmIt = header.save_time.time_of_day().minutes();
      *ssIt = header.save_time.time_of_day().seconds();
      *msIt = header.save_time.time_of_day().fractional_seconds();
    }

    return fileExists;
  }
};

struct SaveDateTime : public RLOp_Store_9<IntConstant_T,
                                          IntReference_T,
                                          IntReference_T,
                                          IntReference_T,
                                          IntReference_T,
                                          IntReference_T,
                                          IntReference_T,
                                          IntReference_T,
                                          IntReference_T> {
  int operator()(RLMachine& machine,
                 int slot,
                 IntReferenceIterator yIt,
                 IntReferenceIterator mIt,
                 IntReferenceIterator dIt,
                 IntReferenceIterator wdIt,
                 IntReferenceIterator hhIt,
                 IntReferenceIterator mmIt,
                 IntReferenceIterator ssIt,
                 IntReferenceIterator msIt) {
    int fileExists = SaveExists()(machine, slot);

    if (fileExists) {
      SaveGameHeader header = Serialization::loadHeaderForSlot(machine, slot);

      *yIt = header.save_time.date().year();
      *mIt = header.save_time.date().month();
      *dIt = header.save_time.date().day();
      *wdIt = header.save_time.date().day_of_week();
      *hhIt = header.save_time.time_of_day().hours();
      *mmIt = header.save_time.time_of_day().minutes();
      *ssIt = header.save_time.time_of_day().seconds();
      *msIt = header.save_time.time_of_day().fractional_seconds();
    }

    return fileExists;
  }
};

struct SaveInfo : public RLOp_Store_10<IntConstant_T,
                                       IntReference_T,
                                       IntReference_T,
                                       IntReference_T,
                                       IntReference_T,
                                       IntReference_T,
                                       IntReference_T,
                                       IntReference_T,
                                       IntReference_T,
                                       StrReference_T> {
  int operator()(RLMachine& machine,
                 int slot,
                 IntReferenceIterator yIt,
                 IntReferenceIterator mIt,
                 IntReferenceIterator dIt,
                 IntReferenceIterator wdIt,
                 IntReferenceIterator hhIt,
                 IntReferenceIterator mmIt,
                 IntReferenceIterator ssIt,
                 IntReferenceIterator msIt,
                 StringReferenceIterator titleIt) {
    int fileExists = SaveExists()(machine, slot);

    if (fileExists) {
      SaveGameHeader header = Serialization::loadHeaderForSlot(machine, slot);

      *yIt = header.save_time.date().year();
      *mIt = header.save_time.date().month();
      *dIt = header.save_time.date().day();
      *wdIt = header.save_time.date().day_of_week();
      *hhIt = header.save_time.time_of_day().hours();
      *mmIt = header.save_time.time_of_day().minutes();
      *ssIt = header.save_time.time_of_day().seconds();
      *msIt = header.save_time.time_of_day().fractional_seconds();

      // Convert the UTF-8 string to the memory internal CP932
      *titleIt = header.title;
    }

    return fileExists;
  }
};

typedef Argc_T<Special_T<
    DefaultSpecialMapper,
    Complex3_T<IntReference_T, IntReference_T, IntConstant_T>,
    Complex3_T<StrReference_T, StrReference_T, IntConstant_T>>> GetSaveFlagList;

// Retrieves the values of variables from saved games. If slot is
// empty, returns 0 and does nothing further; if slot contains a saved
// game, returns 1 and processes the list of structures. For each
// entry in the list, count values are copied to a block of variables
// starting with dst, reading from src: the values copied are those
// that are stored in the saved game in slot.
//
// For example, an RPG that stored the player's level in F[100], the
// player's hit points in F[101], and the name of the player's class
// in S[10], could retrieve these values from saved games to display
// them in a custom load menu as follows:
//
//   str menu_line[10]
//   for (int i = 0) (i < length(menu_line)) (i += 1):
//     int (block) level, hp
//     str class
//     GetSaveFlag(i, {intF[100], level, 2}, {strS[10], class, 1})
//     menu_line[i] = 'Level \i{level} \s{class}, \i{hp} HP';
struct GetSaveFlag : public RLOp_Store_2<IntConstant_T, GetSaveFlagList> {
  int operator()(RLMachine& machine, int slot, GetSaveFlagList::type flagList) {
    int fileExists = SaveExists()(machine, slot);
    if (!fileExists)
      return 0;

    Memory overlayedMemory(machine, slot);
    Serialization::loadLocalMemoryForSlot(machine, slot, overlayedMemory);

    for (GetSaveFlagList::type::iterator it = flagList.begin();
         it != flagList.end();
         ++it) {
      switch (it->type) {
        case 0: {
          IntReferenceIterator jt =
              get<0>(it->first).changeMemoryTo(&overlayedMemory);
          boost::detail::multi_array::copy_n(
              jt, get<2>(it->first), get<1>(it->first));
          break;
        }
        case 1: {
          StringReferenceIterator jt =
              get<0>(it->second).changeMemoryTo(&overlayedMemory);
          boost::detail::multi_array::copy_n(
              jt, get<2>(it->second), get<1>(it->second));
          break;
        }
        default:
          throw rlvm::Exception("Illegal value in Special_T in GetSaveFlag");
          break;
      }
    }

    return 1;
  }
};

// Returns the slot most recently saved to, or −1 if no games have
// been saved.
struct LatestSave : public RLOp_Store_Void {
  int operator()(RLMachine& machine) {
    fs::path saveDir = machine.system().gameSaveDirectory();
    int latestSlot = -1;
    time_t latestTime = std::numeric_limits<time_t>::min();

    if (fs::exists(saveDir)) {
      fs::directory_iterator end;
      for (fs::directory_iterator it(saveDir); it != end; ++it) {
        string filename = it->path().filename().string();
        if (starts_with(filename, "save") && ends_with(filename, ".sav.gz")) {
          time_t mtime = fs::last_write_time(*it);

          if (mtime > latestTime) {
            latestTime = mtime;
            latestSlot = lexical_cast<int>(filename.substr(4, 3));
          }
        }
      }
    }

    return latestSlot;
  }
};

struct save : public RLOp_Void_1<IntConstant_T> {
  void operator()(RLMachine& machine, int slot) {
    Serialization::saveGlobalMemory(machine);
    Serialization::saveGameForSlot(machine, slot);
  }
};

struct LoadingGameFromSlot : public LoadGameLongOperation {
  int slot_;
  explicit LoadingGameFromSlot(RLMachine& machine, int slot)
      : LoadGameLongOperation(machine), slot_(slot) {}

  virtual void load(RLMachine& machine) {
    Serialization::loadGameForSlot(machine, slot_);
  }
};

}  // namespace

// -----------------------------------------------------------------------

void Sys_load::operator()(RLMachine& machine, int slot) {
  // LoadGameLongOperation will add self to |machine|'s stack.
  new LoadingGameFromSlot(machine, slot);
}

// -----------------------------------------------------------------------

void addSysSaveOpcodes(RLModule& m) {
  m.addOpcode(1409, 0, "SaveExists", new SaveExists);
  m.addOpcode(1410, 0, "SaveDate", new SaveDate);
  m.addOpcode(1411, 0, "SaveTime", new SaveTime);
  m.addOpcode(1412, 0, "SaveDateTime", new SaveDateTime);
  m.addOpcode(1413, 0, "SaveInfo", new SaveInfo);
  m.addOpcode(1414, 0, "GetSaveFlag", new GetSaveFlag);
  m.addOpcode(1421, 0, "LatestSave", new LatestSave);

  m.addOpcode(
      2053, 0, "SetConfirmSaveLoad", callFunction(&System::setConfirmSaveLoad));
  m.addOpcode(
      2003, 0, "ConfirmSaveLoad", returnIntValue(&System::confirmSaveLoad));

  m.addOpcode(3000, 0, "menu_save", new InvokeSyscomAsOp(0));
  m.addOpcode(3001, 0, "menu_load", new InvokeSyscomAsOp(1));

  m.addOpcode(3007, 0, "save", new save);
  m.addOpcode(3107, 0, "save_always", new save);

  m.addOpcode(3009, 0, "load", new Sys_load);
  m.addOpcode(3109, 0, "load_always", new Sys_load);

  m.addOpcode(3100, 0, "menu_save_always", new InvokeSyscomAsOp(0));
  m.addOpcode(3101, 0, "menu_load_always", new InvokeSyscomAsOp(1));

  m.addOpcode(3500, 0, "Savepoint", callFunction(&RLMachine::markSavepoint));
  m.addOpcode(3501,
              0,
              "EnableAutoSavepoints",
              callFunctionWith(&RLMachine::setMarkSavepoints, 1));
  m.addOpcode(3502,
              0,
              "DisableAutoSavepoints",
              callFunctionWith(&RLMachine::setMarkSavepoints, 0));
}
