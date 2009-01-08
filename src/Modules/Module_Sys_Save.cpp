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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// -----------------------------------------------------------------------

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

#include "Modules/Module_Sys_Save.hpp"
#include "MachineBase/RLOperation.hpp"
#include "MachineBase/RLOperation/RLOp_Store.hpp"
#include "MachineBase/RLOperation/References.hpp"
#include "MachineBase/RLOperation/Argc_T.hpp"
#include "MachineBase/RLOperation/Special_T.hpp"
#include "MachineBase/RLOperation/Complex_T.hpp"
#include "MachineBase/RLModule.hpp"
#include "MachineBase/RLMachine.hpp"
#include "MachineBase/LongOperation.hpp"
#include "MachineBase/Serialization.hpp"
#include "MachineBase/SaveGameHeader.hpp"
#include "MachineBase/Memory.hpp"
#include "MachineBase/GeneralOperations.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/TextSystem.hpp"
#include "Systems/Base/SoundSystem.hpp"
#include "Systems/Base/Surface.hpp"
#include "Systems/Base/Colour.hpp"
#include "libReallive/intmemref.h"

#include <boost/shared_ptr.hpp>

#include <algorithm>
#include <iostream>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>

#include "utf8.h"
#include "Effects/FadeEffect.hpp"

// For copy_n, which isn't part of the C++ standard and doesn't come on
// OSX.
#include <boost/multi_array/algorithm.hpp>

using namespace std;
using namespace libReallive;
using boost::lexical_cast;
using boost::starts_with;
using boost::ends_with;
using boost::bind;
using boost::shared_ptr;
namespace fs = boost::filesystem;

// -----------------------------------------------------------------------

struct Sys_SaveExists : public RLOp_Store_1< IntConstant_T >
{
  int operator()(RLMachine& machine, int slot)
  {
    fs::path saveFile = Serialization::buildSaveGameFilename(machine, slot);
    return fs::exists(saveFile) ? 1 : 0;
  }
};

// -----------------------------------------------------------------------

struct Sys_SaveDate : public RLOp_Store_5<
  IntConstant_T, IntReference_T, IntReference_T, IntReference_T, IntReference_T>
{
  int operator()(RLMachine& machine, int slot,
                 IntReferenceIterator yIt, IntReferenceIterator mIt,
                 IntReferenceIterator dIt, IntReferenceIterator wdIt)
  {
    int fileExists = Sys_SaveExists()(machine, slot);

    if(fileExists) {
      SaveGameHeader header = Serialization::loadHeaderForSlot(machine, slot);

      *yIt = header.save_time.date().year();
      *mIt = header.save_time.date().month();
      *dIt = header.save_time.date().day();
      *wdIt = header.save_time.date().day_of_week();
    }

    return fileExists;
  }
};

// -----------------------------------------------------------------------

struct Sys_SaveTime : public RLOp_Store_5<
  IntConstant_T, IntReference_T, IntReference_T, IntReference_T, IntReference_T>
{
  int operator()(RLMachine& machine, int slot,
                 IntReferenceIterator hhIt, IntReferenceIterator mmIt,
                 IntReferenceIterator ssIt, IntReferenceIterator msIt)
  {
    int fileExists = Sys_SaveExists()(machine, slot);

    if(fileExists)
    {
      SaveGameHeader header = Serialization::loadHeaderForSlot(machine, slot);

      *hhIt = header.save_time.time_of_day().hours();
      *mmIt = header.save_time.time_of_day().minutes();
      *ssIt = header.save_time.time_of_day().seconds();
      *msIt = header.save_time.time_of_day().fractional_seconds();
    }

    return fileExists;
  }
};

// -----------------------------------------------------------------------

struct Sys_SaveDateTime : public RLOp_Store_9<
  IntConstant_T, IntReference_T, IntReference_T, IntReference_T, IntReference_T,
  IntReference_T, IntReference_T, IntReference_T, IntReference_T >
{
  int operator()(RLMachine& machine, int slot,
                 IntReferenceIterator yIt, IntReferenceIterator mIt,
                 IntReferenceIterator dIt, IntReferenceIterator wdIt,
                 IntReferenceIterator hhIt, IntReferenceIterator mmIt,
                 IntReferenceIterator ssIt, IntReferenceIterator msIt)
  {
    int fileExists = Sys_SaveExists()(machine, slot);

    if(fileExists)
    {
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

// -----------------------------------------------------------------------


struct Sys_SaveInfo : public RLOp_Store_10<
  IntConstant_T, IntReference_T, IntReference_T, IntReference_T, IntReference_T,
  IntReference_T, IntReference_T, IntReference_T, IntReference_T, StrReference_T >
{
  int operator()(RLMachine& machine, int slot,
                 IntReferenceIterator yIt, IntReferenceIterator mIt,
                 IntReferenceIterator dIt, IntReferenceIterator wdIt,
                 IntReferenceIterator hhIt, IntReferenceIterator mmIt,
                 IntReferenceIterator ssIt, IntReferenceIterator msIt,
                 StringReferenceIterator titleIt)
  {
    int fileExists = Sys_SaveExists()(machine, slot);

    if(fileExists) {
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

// -----------------------------------------------------------------------

typedef Argc_T<
  Special_T<
    Complex3_T<IntReference_T, IntReference_T, IntConstant_T>,
    Complex3_T<StrReference_T, StrReference_T, IntConstant_T> > >
GetSaveFlagList;

/**
 * Retrieves the values of variables from saved games. If slot is
 * empty, returns 0 and does nothing further; if slot contains a saved
 * game, returns 1 and processes the list of structures. For each
 * entry in the list, count values are copied to a block of variables
 * starting with dst, reading from src: the values copied are those
 * that are stored in the saved game in slot.
 *
 * For example, an RPG that stored the player's level in F[100], the
 * player's hit points in F[101], and the name of the player's class
 * in S[10], could retrieve these values from saved games to display
 * them in a custom load menu as follows:
 *
 * @code
 * str menu_line[10]
 * for (int i = 0) (i < length(menu_line)) (i += 1):
 *   int (block) level, hp
 *   str class
 *   GetSaveFlag(i, {intF[100], level, 2}, {strS[10], class, 1})
 *   menu_line[i] = 'Level \i{level} \s{class}, \i{hp} HP';
 * @endcode
 */
struct Sys_GetSaveFlag : public RLOp_Store_2<
  IntConstant_T, GetSaveFlagList>
{
  /// Main operation
  int operator()(RLMachine& machine, int slot, GetSaveFlagList::type flagList)
  {
    int fileExists = Sys_SaveExists()(machine, slot);
    if(!fileExists)
      return 0;

    Memory overlayedMemory(machine, slot);
    Serialization::loadLocalMemoryForSlot(machine, slot, overlayedMemory);

    using boost::detail::multi_array::copy_n;
    for(GetSaveFlagList::type::iterator it = flagList.begin();
        it != flagList.end(); ++it)
    {
      switch(it->type)
      {
      case 0:
      {
        IntReferenceIterator jt = it->first.get<0>()
          .changeMemoryTo(&overlayedMemory);
        copy_n(jt, it->first.get<2>(), it->first.get<1>());
        break;
      }
      case 1:
      {
        StringReferenceIterator jt = it->second.get<0>()
          .changeMemoryTo(&overlayedMemory);
        copy_n(jt, it->second.get<2>(), it->second.get<1>());
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

// -----------------------------------------------------------------------

/**
 * Returns the slot most recently saved to, or −1 if no games have
 * been saved.
 */
struct Sys_LatestSave : public RLOp_Store_Void
{
  int operator()(RLMachine& machine)
  {
    fs::path saveDir = machine.system().gameSaveDirectory();
    int latestSlot = -1;
    time_t latestTime = numeric_limits<time_t>::min();

    if(fs::exists(saveDir))
    {
      fs::directory_iterator end;
      for(fs::directory_iterator it(saveDir); it != end; ++it)
      {
        string filename = it->leaf();
        if(starts_with(filename, "save") && ends_with(filename, ".sav.gz"))
        {
          time_t mtime = fs::last_write_time(*it);

          if(mtime > latestTime)
          {
            latestTime = mtime;
            latestSlot = lexical_cast<int>(filename.substr(4, 3));
          }
        }
      }
    }

    return latestSlot;
  }
};

// -----------------------------------------------------------------------

struct Sys_save : public RLOp_Void_1< IntConstant_T >
{
  void operator()(RLMachine& machine, int slot)
  {
    Serialization::saveGlobalMemory(machine);
    Serialization::saveGameForSlot(machine, slot);
  }
};

// -----------------------------------------------------------------------

/**
 * Implementation of fun load<1:Sys:03009, 0> ('slot'): Loads data
 * from a save game slot.
 *
 * Internally, load is fairly complex, consisting of several
 * LongOperations because we can't rely on normal flow control because
 * we're going to nuke the call stack and system memory in
 * LoadingGame.
 */
bool Sys_load::LoadingGame::operator()(RLMachine& machine) {
  Serialization::loadGameForSlot(machine, slot_);

  // Render the current state of the screen
  GraphicsSystem& graphics = machine.system().graphics();

  shared_ptr<Surface> dc0 = graphics.getDC(0);
  shared_ptr<Surface> currentWindow =
    graphics.renderToSurfaceWithBg(dc0);
  Size s = currentWindow->size();

  // Blank dc0 (because we won't be using it anyway) for the image
  // we're going to render to
  shared_ptr<Surface> blankScreen = graphics.buildSurface(s);
  blankScreen->fill(RGBAColour::Black());

  machine.pushLongOperation(
    new FadeEffect(machine, currentWindow, blankScreen, s, 250));

  // At this point, the stack has been nuked, and this current
  // object has already been deleted, leaving an invalid
  // *this. Returning false is the correct thing to do since
  // *returning true will pop an unrelated stack frame.
  return false;
}

void Sys_load::operator()(RLMachine& machine, int slot)
{
  // Render the current state of the screen
  GraphicsSystem& graphics = machine.system().graphics();

  shared_ptr<Surface> dc0 = graphics.getDC(0);
  shared_ptr<Surface> currentWindow =
    graphics.renderToSurfaceWithBg(dc0);
  Size s = currentWindow->size();

  // Blank dc0 (because we won't be using it anyway) for the image
  // we're going to render to
  dc0->fill(RGBAColour::Black());

  machine.pushLongOperation(new LoadingGame(slot));
  machine.pushLongOperation(
    new FadeEffect(machine, dc0, currentWindow, s, 250));

  // We have our before and after images to use as a transition now. Reset the
  // system to prevent a brief flash of the previous contents of the screen for
  // whatever number of user preceivable milliseconds.
  machine.system().reset();
}

// -----------------------------------------------------------------------

void addSysSaveOpcodes(RLModule& m)
{
  m.addOpcode(1409, 0, "SaveExists", new Sys_SaveExists);
  m.addOpcode(1410, 0, "SaveDate", new Sys_SaveDate);
  m.addOpcode(1411, 0, "SaveTime", new Sys_SaveTime);
  m.addOpcode(1412, 0, "SaveDateTime", new Sys_SaveDateTime);
  m.addOpcode(1413, 0, "SaveInfo", new Sys_SaveInfo);
  m.addOpcode(1414, 0, "GetSaveFlag", new Sys_GetSaveFlag);
  m.addOpcode(1421, 0, "LatestSave", new Sys_LatestSave);

  m.addOpcode(2053, 0, "SetConfirmSaveLoad",
              callFunction(&System::setConfirmSaveLoad));
  m.addOpcode(2003, 0, "ConfirmSaveLoad",
              returnIntValue(&System::confirmSaveLoad));

  m.addOpcode(3000, 0, "menu_save", new InvokeSyscomAsOp(0));
  m.addOpcode(3001, 0, "menu_load", new InvokeSyscomAsOp(1));

  m.addOpcode(3007, 0, "save", new Sys_save);
  m.addOpcode(3107, 0, "save_always", new Sys_save);

  m.addOpcode(3009, 0, "load", new Sys_load);
  m.addOpcode(3109, 0, "load_always", new Sys_load);

  m.addOpcode(3501, 0, "EnableAutoSavepoints",
              setToConstant(&RLMachine::setMarkSavepoints, 1));
  m.addOpcode(3502, 0, "DisableAutoSavepoints",
              setToConstant(&RLMachine::setMarkSavepoints, 0));
}
