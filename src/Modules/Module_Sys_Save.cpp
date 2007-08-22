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
#include "MachineBase/RLModule.hpp"
#include "MachineBase/RLMachine.hpp"
#include "MachineBase/GeneralOperations.hpp"
#include "Systems/Base/System.hpp"

#include <iostream>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>

#include "json/value.h"
#include "json/reader.h"
#include "json/writer.h"

#include "utf8.h"
#include "Modules/cp932toUnicode.hpp"

using namespace std;
using boost::lexical_cast;
using boost::starts_with;
using boost::ends_with;
namespace fs = boost::filesystem;

// -----------------------------------------------------------------------

namespace {

void getSaveTime(Json::Value& value, int& y, int& m, int& d, int& wd, 
				 int& hh, int& mm, int& ss, int& ms)
{
  const Json::Value& intMem = value["saveTime"];
  y = intMem[0u].asInt();
  m = intMem[1u].asInt();
  d = intMem[2u].asInt();
  wd = intMem[3u].asInt();
  hh = intMem[4u].asInt();
  mm = intMem[5u].asInt();
  ss = intMem[6u].asInt();
  ms = intMem[7u].asInt();
}

// -----------------------------------------------------------------------

void loadJsonFile(RLMachine& machine, int slotNum, Json::Value& root)
{
  fs::path saveFile = machine.system().gameSaveDirectory() / 
	machine.makeSaveGameName(slotNum);
  fs::ifstream file(saveFile);
  if(!file)
  {
	ostringstream oss;
	oss << "Couldn't read save file " << saveFile.string();
	throw rlvm::Exception(oss.str());
  }

  string memoryContents;
  string line;
  while(getline(file, line))
  {
	memoryContents += line;
	memoryContents += "\n";
  }

  Json::Reader reader;
  if(!reader.parse(memoryContents, root))
  {
	throw rlvm::Exception("Json::Reader failed");
  }
}

}

// -----------------------------------------------------------------------

struct Sys_SaveExists : public RLOp_Store_1< IntConstant_T >
{
  int operator()(RLMachine& machine, int slot)
  {
	fs::path saveFile = machine.system().gameSaveDirectory() / 
	  machine.makeSaveGameName(slot);
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
	  Json::Value root;
	  loadJsonFile(machine, slot, root);

	  int y, m, d, wd, hh, mm, ss, ms;
	  getSaveTime(root, y, m, d, wd, hh, mm, ss, ms);
	  *yIt = y;
	  *mIt = m;
	  *dIt = d;
	  *wdIt = wd;
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
	  Json::Value root;
	  loadJsonFile(machine, slot, root);

	  int y, m, d, wd, hh, mm, ss, ms;
	  getSaveTime(root, y, m, d, wd, hh, mm, ss, ms);
	  *hhIt = hh;
	  *mmIt = mm;
	  *ssIt = ss;
	  *msIt = ms;
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
	  Json::Value root;
	  loadJsonFile(machine, slot, root);

	  int y, m, d, wd, hh, mm, ss, ms;
	  getSaveTime(root, y, m, d, wd, hh, mm, ss, ms);
	  *yIt = y;
	  *mIt = m;
	  *dIt = d;
	  *wdIt = wd;
	  *hhIt = hh;
	  *mmIt = mm;
	  *ssIt = ss;
	  *msIt = ms;
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
	  Json::Value root;
	  loadJsonFile(machine, slot, root);

	  int y, m, d, wd, hh, mm, ss, ms;
	  getSaveTime(root, y, m, d, wd, hh, mm, ss, ms);
	  *yIt = y;
	  *mIt = m;
	  *dIt = d;
	  *wdIt = wd;
	  *hhIt = hh;
	  *mmIt = mm;
	  *ssIt = ss;
	  *msIt = ms;

	  // Convert the UTF-8 string to the memory internal CP932
	  string utf8str = root["title"].asString();
	  wstring unicode;
	  utf8::utf8to32(utf8str.begin(), utf8str.end(), back_inserter(unicode));
	  *titleIt = unicodetocp932(unicode);
	}

	return fileExists;
  }
};

// -----------------------------------------------------------------------

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
	machine.saveGame(slot);
  }
};

// -----------------------------------------------------------------------

struct Sys_load : public RLOp_Void_1< IntConstant_T >
{
  bool advanceInstructionPointer() { return false; }

  void operator()(RLMachine& machine, int slot)
  {
	machine.loadGame(slot);
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

void addSysSaveOpcodes(RLModule& m)
{
  m.addOpcode(1409, 0, "SaveExists", new Sys_SaveExists);
  m.addOpcode(1410, 0, "SaveDate", new Sys_SaveDate);
  m.addOpcode(1411, 0, "SaveTime", new Sys_SaveTime);
  m.addOpcode(1412, 0, "SaveDateTime", new Sys_SaveDateTime);
  m.addOpcode(1413, 0, "SaveInfo", new Sys_SaveInfo);
  m.addUnsupportedOpcode(1414, 0, "GetSaveFlag");
  m.addOpcode(1421, 0, "LatestSave", new Sys_LatestSave);

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
