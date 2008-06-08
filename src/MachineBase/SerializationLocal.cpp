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

// include headers that implement a archive in simple text format
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/scoped_ptr.hpp>
#include <boost/date_time/posix_time/time_serialize.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>


#include "Utilities.h"

#include "MachineBase/RLMachine.hpp"
#include "MachineBase/StackFrame.hpp"
#include "MachineBase/Memory.hpp"
#include "MachineBase/SaveGameHeader.hpp"
#include "MachineBase/Serialization.hpp"
#include "algoplus.hpp"

#include "Systems/Base/System.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/GraphicsStackFrame.hpp"
#include "Systems/Base/GraphicsObject.hpp"
#include "Systems/Base/GraphicsObjectOfFile.hpp"
#include "Systems/Base/AnmGraphicsObjectData.hpp"
#include "Systems/Base/GanGraphicsObjectData.hpp"
#include "Systems/Base/EventSystem.hpp"
#include "Systems/Base/TextSystem.hpp"
#include "Systems/Base/SoundSystem.hpp"
#include "libReallive/intmemref.h"
#include "libReallive/archive.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <exception>
#include <stdexcept>
#include <boost/bind.hpp>

using namespace std;
using namespace libReallive;
using namespace boost::archive;
namespace fs = boost::filesystem;

// -----------------------------------------------------------------------

namespace Serialization {

RLMachine* g_currentMachine = NULL;

// -----------------------------------------------------------------------

const int CURRENT_LOCAL_VERSION = 2;

}

// -----------------------------------------------------------------------

namespace {

template<typename TYPE>
void checkInFileOpened(TYPE& file, const fs::path& home)
{
  if(!file)
  {
    ostringstream oss;
    oss << "Could not open save game file " << home.string();
    throw rlvm::Exception(oss.str());
  }
}

}

// -----------------------------------------------------------------------

namespace Serialization {

// -----------------------------------------------------------------------

void saveGameForSlot(RLMachine& machine, int slot)
{
  fs::path path = buildSaveGameFilename(machine, slot);
  fs::ofstream file(path);
  checkInFileOpened(file, path);
  return saveGameTo(file, machine);
}

// -----------------------------------------------------------------------

void saveGameTo(std::ostream& oss, RLMachine& machine)
{
  const SaveGameHeader header(machine.system().graphics().windowSubtitle());

  g_currentMachine = &machine;

  try
  {

    text_oarchive oa(oss);
    oa << CURRENT_LOCAL_VERSION
       << header
       << const_cast<const LocalMemory&>(machine.memory().local())
       << const_cast<const RLMachine&>(machine)
       << const_cast<const System&>(machine.system())
       << const_cast<const GraphicsSystem&>(machine.system().graphics())
       << const_cast<const TextSystem&>(machine.system().text())
       << const_cast<const SoundSystem&>(machine.system().sound());
  }
  catch(std::exception& e)
  {
    cerr << "--- WARNING: ERROR DURING SAVING FILE: " << e.what() << " ---"
         << endl;

    g_currentMachine = NULL;
    throw e;
  }

  g_currentMachine = NULL;
}

// -----------------------------------------------------------------------

fs::path buildSaveGameFilename(RLMachine& machine, int slot)
{
  ostringstream oss;
  oss << "save" << setw(3) << setfill('0') << slot << ".sav";

  return machine.system().gameSaveDirectory() / oss.str();
}

// -----------------------------------------------------------------------

SaveGameHeader loadHeaderForSlot(RLMachine& machine, int slot)
{
  fs::path path = buildSaveGameFilename(machine, slot);
  fs::ifstream file(path);
  checkInFileOpened(file, path);
  return loadHeaderFrom(file);
}

// -----------------------------------------------------------------------

SaveGameHeader loadHeaderFrom(std::istream& iss)
{
  int version;
  SaveGameHeader header;

  // Only load the header
  text_iarchive ia(iss);
  ia >> version >> header;

  return header;
}

// -----------------------------------------------------------------------

void loadLocalMemoryForSlot(RLMachine& machine, int slot, Memory& memory)
{
  fs::path path = buildSaveGameFilename(machine, slot);
  fs::ifstream file(path);
  checkInFileOpened(file, path);
  loadLocalMemoryFrom(file, memory);
} 

// -----------------------------------------------------------------------

void loadLocalMemoryFrom(std::istream& iss, Memory& memory)
{
  int version;
  SaveGameHeader header;

  // Only load the header
  text_iarchive ia(iss);
  ia >> version
     >> header
     >> memory.local();
}

// -----------------------------------------------------------------------

void loadGameForSlot(RLMachine& machine, int slot)
{
  fs::path path = buildSaveGameFilename(machine, slot);
  fs::ifstream file(path);
  checkInFileOpened(file, path);
  loadGameFrom(file, machine);
} 

// -----------------------------------------------------------------------

void loadGameFrom(std::istream& iss, RLMachine& machine)
{
  int version;
  SaveGameHeader header;

  g_currentMachine = &machine;

  try
  {
    machine.system().reset();

    text_iarchive ia(iss);
    ia >> version
       >> header
       >> machine.memory().local()
       >> machine
       >> machine.system()
       >> machine.system().graphics()
       >> machine.system().text()
       >> machine.system().sound();

    machine.system().graphics().replayGraphicsStack(machine);

    machine.system().graphics().forceRefresh();
  }
  catch(std::exception& e)
  {
    cerr << "--- WARNING: ERROR DURING LOADING FILE: " << e.what() << " ---"
         << endl;

    g_currentMachine = NULL;
    throw e;
  }

  g_currentMachine = NULL;
}

}
