// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2007 Elliot Glaysher
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

#include "Modules/Module_Sys_Save.hpp"
#include "MachineBase/RLOperation.hpp"
#include "MachineBase/RLOperation/RLOp_Store.hpp"
#include "MachineBase/RLOperation/References.hpp"
#include "MachineBase/RLOperation/Argc_T.hpp"
#include "MachineBase/RLOperation/Special_T.hpp"
#include "MachineBase/RLOperation/Complex_T.hpp"
#include "MachineBase/RLModule.hpp"
#include "MachineBase/RLMachine.hpp"
#include "MachineBase/Serialization.hpp"
#include "MachineBase/SaveGameHeader.hpp"
#include "MachineBase/Memory.hpp"
#include "MachineBase/GeneralOperations.hpp"
#include "Systems/Base/System.hpp"
#include "libReallive/intmemref.h"

#include <algorithm>
#include <iostream>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>

#include "json/value.h"
#include "json/reader.h"
#include "json/writer.h"

#include "utf8.h"
#include "Modules/cp932toUnicode.hpp"

// For copy_n, which isn't part of the C++ standard and doesn't come on
// OSX.
#include <boost/multi_array/algorithm.hpp>

using namespace std;
using namespace libReallive;
using boost::lexical_cast;
using boost::starts_with;
using boost::ends_with;
using boost::bind;
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

	if(fileExists)
	{
      SaveGameHeader header = Serialization::loadHeaderForSlot(machine, slot);

	  *yIt = header.saveTime.date().year();
	  *mIt = header.saveTime.date().month();
	  *dIt = header.saveTime.date().day();
	  *wdIt = header.saveTime.date().day_of_week();
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

	  *hhIt = header.saveTime.time_of_day().hours();
	  *mmIt = header.saveTime.time_of_day().minutes();
	  *ssIt = header.saveTime.time_of_day().seconds();
	  *msIt = header.saveTime.time_of_day().fractional_seconds();
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

	  *yIt = header.saveTime.date().year();
	  *mIt = header.saveTime.date().month();
	  *dIt = header.saveTime.date().day();
	  *wdIt = header.saveTime.date().day_of_week();
	  *hhIt = header.saveTime.time_of_day().hours();
	  *mmIt = header.saveTime.time_of_day().minutes();
	  *ssIt = header.saveTime.time_of_day().seconds();
	  *msIt = header.saveTime.time_of_day().fractional_seconds();
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

	if(fileExists)
	{
      SaveGameHeader header = Serialization::loadHeaderForSlot(machine, slot);

	  *yIt = header.saveTime.date().year();
	  *mIt = header.saveTime.date().month();
	  *dIt = header.saveTime.date().day();
	  *wdIt = header.saveTime.date().day_of_week();
	  *hhIt = header.saveTime.time_of_day().hours();
	  *mmIt = header.saveTime.time_of_day().minutes();
	  *ssIt = header.saveTime.time_of_day().seconds();
	  *msIt = header.saveTime.time_of_day().fractional_seconds();

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
		if(starts_with(filename, "save") && ends_with(filename, ".jsn"))
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
	cerr << "SAVING TO SLOT " << slot << endl;
	Serialization::saveGlobalMemory(machine);
    Serialization::saveGameForSlot(machine, slot);
  }
};
 
// -----------------------------------------------------------------------

struct Sys_load : public RLOp_Void_1< IntConstant_T >
{
  bool advanceInstructionPointer() { return false; }

  void operator()(RLMachine& machine, int slot)
  {
    Serialization::loadGameForSlot(machine, slot);
  }
};

// -----------------------------------------------------------------------

struct Sys_EnableAutoSavepoints : public RLOp_Void_Void
{
  void operator()(RLMachine& machine) { machine.setMarkSavepoints(1); }
};

// -----------------------------------------------------------------------

struct Sys_DisableAutoSavepoints : public RLOp_Void_Void
{
  void operator()(RLMachine& machine) { machine.setMarkSavepoints(0); }
};

// -----------------------------------------------------------------------

void addSysSaveOpcodes(RLModule& m, System& system)
{
  m.addOpcode(1409, 0, "SaveExists", new Sys_SaveExists);
  m.addOpcode(1410, 0, "SaveDate", new Sys_SaveDate);
  m.addOpcode(1411, 0, "SaveTime", new Sys_SaveTime);
  m.addOpcode(1412, 0, "SaveDateTime", new Sys_SaveDateTime);
  m.addOpcode(1413, 0, "SaveInfo", new Sys_SaveInfo);
//  m.addUnsupportedOpcode(1414, 0, "GetSaveFlag");
  m.addOpcode(1414, 0, "GetSaveFlag", new Sys_GetSaveFlag);
  m.addOpcode(1421, 0, "LatestSave", new Sys_LatestSave);

  m.addOpcode(2053, 0, "SetConfirmSaveLoad",
              setToIncomingInt(system, &System::setConfirmSaveLoad));
  m.addOpcode(2003, 0, "ConfirmSaveLoad",
              returnIntValue(system, &System::confirmSaveLoad));

  m.addUnsupportedOpcode(3000, 0, "menu_save");
  m.addUnsupportedOpcode(3001, 0, "menu_load");

  m.addOpcode(3007, 0, "save", new Sys_save);
  m.addOpcode(3107, 0, "save_always", new Sys_save);

  m.addOpcode(3009, 0, "load", new Sys_load);
  m.addOpcode(3109, 0, "load_always", new Sys_load);

  m.addOpcode(3501, 0, "EnableAutoSavepoints", 
              new Sys_EnableAutoSavepoints);
  m.addOpcode(3502, 0, "DisableAutoSavepoints", 
              new Sys_DisableAutoSavepoints);
}
