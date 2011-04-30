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
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <fstream>
#include <sstream>
#include <iostream>
#include <exception>
#include <stdexcept>
#include <string>
#include <boost/bind.hpp>

#include "MachineBase/Memory.hpp"
#include "MachineBase/RLMachine.hpp"
#include "MachineBase/SaveGameHeader.hpp"
#include "MachineBase/Serialization.hpp"
#include "MachineBase/StackFrame.hpp"
#include "Systems/Base/AnmGraphicsObjectData.hpp"
#include "Systems/Base/EventSystem.hpp"
#include "Systems/Base/GanGraphicsObjectData.hpp"
#include "Systems/Base/GraphicsObject.hpp"
#include "Systems/Base/GraphicsObjectOfFile.hpp"
#include "Systems/Base/GraphicsStackFrame.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/SoundSystem.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/TextSystem.hpp"
#include "Utilities/Exception.hpp"
#include "Utilities/algoplus.hpp"
#include "libReallive/archive.h"
#include "libReallive/intmemref.h"

using namespace std;
using namespace libReallive;
using namespace boost::archive;
namespace fs = boost::filesystem;

namespace Serialization {

RLMachine* g_current_machine = NULL;

const int CURRENT_LOCAL_VERSION = 2;

}  // namespace Serialization

namespace {

template<typename TYPE>
void checkInFileOpened(TYPE& file, const fs::path& home) {
  if (!file) {
    ostringstream oss;
    oss << "Could not open save game file " << home.string();
    throw rlvm::Exception(oss.str());
  }
}

}  // namespace

namespace Serialization {

void saveGameForSlot(RLMachine& machine, int slot) {
  fs::path path = buildSaveGameFilename(machine, slot);
  fs::ofstream file(path, ios::binary);
  checkInFileOpened(file, path);

  saveGameTo(file, machine);
}

void saveGameTo(std::ostream& oss, RLMachine& machine) {
  using namespace boost::iostreams;
  filtering_stream<output> filtered_output;
  filtered_output.push(zlib_compressor());
  filtered_output.push(oss);

  const SaveGameHeader header(machine.system().graphics().windowSubtitle());

  g_current_machine = &machine;

  try {
    text_oarchive oa(filtered_output);
    oa << CURRENT_LOCAL_VERSION
       << header
       << const_cast<const LocalMemory&>(machine.memory().local())
       << const_cast<const RLMachine&>(machine)
       << const_cast<const System&>(machine.system())
       << const_cast<const GraphicsSystem&>(machine.system().graphics())
       << const_cast<const TextSystem&>(machine.system().text())
       << const_cast<const SoundSystem&>(machine.system().sound());
  }
  catch(std::exception& e) {
    cerr << "--- WARNING: ERROR DURING SAVING FILE: " << e.what() << " ---"
         << endl;

    g_current_machine = NULL;
    throw e;
  }

  g_current_machine = NULL;
}

fs::path buildSaveGameFilename(RLMachine& machine, int slot) {
  ostringstream oss;
  oss << "save" << setw(3) << setfill('0') << slot << ".sav.gz";

  return machine.system().gameSaveDirectory() / oss.str();
}

SaveGameHeader loadHeaderForSlot(RLMachine& machine, int slot) {
  fs::path path = buildSaveGameFilename(machine, slot);
  fs::ifstream file(path, ios::binary);
  checkInFileOpened(file, path);

  return loadHeaderFrom(file);
}

SaveGameHeader loadHeaderFrom(std::istream& iss) {
  using namespace boost::iostreams;
  filtering_stream<input> filtered_input;
  filtered_input.push(zlib_decompressor());
  filtered_input.push(iss);

  int version;
  SaveGameHeader header;

  // Only load the header
  text_iarchive ia(filtered_input);
  ia >> version >> header;

  return header;
}

void loadLocalMemoryForSlot(RLMachine& machine, int slot, Memory& memory) {
  fs::path path = buildSaveGameFilename(machine, slot);
  fs::ifstream file(path, ios::binary);
  checkInFileOpened(file, path);

  loadLocalMemoryFrom(file, memory);
}

void loadLocalMemoryFrom(std::istream& iss, Memory& memory) {
  using namespace boost::iostreams;
  filtering_stream<input> filtered_input;
  filtered_input.push(zlib_decompressor());
  filtered_input.push(iss);

  int version;
  SaveGameHeader header;

  // Only load the header
  text_iarchive ia(filtered_input);
  ia >> version
     >> header
     >> memory.local();
}

void loadGameForSlot(RLMachine& machine, int slot) {
  fs::path path = buildSaveGameFilename(machine, slot);
  fs::ifstream file(path, ios::binary);
  checkInFileOpened(file, path);

  loadGameFrom(file, machine);
}

void loadGameFrom(std::istream& iss, RLMachine& machine) {
  using namespace boost::iostreams;
  filtering_stream<input> filtered_input;
  filtered_input.push(zlib_decompressor());
  filtered_input.push(iss);

  int version;
  SaveGameHeader header;

  g_current_machine = &machine;

  try {
    // Must clear the stack before reseting the System because LongOperations
    // often hold references to objects in the System heiarchy.
    machine.reset();

    text_iarchive ia(filtered_input);
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
  catch(std::exception& e) {
    cerr << "--- WARNING: ERROR DURING LOADING FILE: " << e.what() << " ---"
         << endl;

    g_current_machine = NULL;
    throw e;
  }

  g_current_machine = NULL;
}

}  // namespace Serialization
