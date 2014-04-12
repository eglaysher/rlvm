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

#include "libreallive/archive.h"
#include "libreallive/intmemref.h"
#include "machine/memory.h"
#include "machine/rlmachine.h"
#include "machine/save_game_header.h"
#include "machine/serialization.h"
#include "machine/stack_frame.h"
#include "systems/base/anm_graphics_object_data.h"
#include "systems/base/event_system.h"
#include "systems/base/gan_graphics_object_data.h"
#include "systems/base/graphics_object.h"
#include "systems/base/graphics_object_of_file.h"
#include "systems/base/graphics_stack_frame.h"
#include "systems/base/graphics_system.h"
#include "systems/base/sound_system.h"
#include "systems/base/system.h"
#include "systems/base/text_system.h"
#include "utilities/exception.h"
#include "utilities/gettext.h"

namespace fs = boost::filesystem;

namespace Serialization {

RLMachine* g_current_machine = NULL;

const int CURRENT_LOCAL_VERSION = 2;

}  // namespace Serialization

namespace {

template <typename TYPE>
void checkInFileOpened(TYPE& file, const fs::path& home) {
  if (!file) {
    throw rlvm::Exception(
        str(format(_("Could not open save game file %1%")) % home));
  }
}

}  // namespace

namespace Serialization {

void saveGameForSlot(RLMachine& machine, int slot) {
  fs::path path = buildSaveGameFilename(machine, slot);
  fs::ofstream file(path, std::ios::binary);
  checkInFileOpened(file, path);

  saveGameTo(file, machine);
}

void saveGameTo(std::ostream& oss, RLMachine& machine) {
  boost::iostreams::filtering_stream<boost::iostreams::output> filtered_output;
  filtered_output.push(boost::iostreams::zlib_compressor());
  filtered_output.push(oss);

  const SaveGameHeader header(machine.system().graphics().window_subtitle());

  g_current_machine = &machine;

  try {
    boost::archive::text_oarchive oa(filtered_output);
    oa << CURRENT_LOCAL_VERSION << header
       << const_cast<const LocalMemory&>(machine.memory().local())
       << const_cast<const RLMachine&>(machine)
       << const_cast<const System&>(machine.system())
       << const_cast<const GraphicsSystem&>(machine.system().graphics())
       << const_cast<const TextSystem&>(machine.system().text())
       << const_cast<const SoundSystem&>(machine.system().sound());
  }
  catch (std::exception& e) {
    std::cerr << "--- WARNING: ERROR DURING SAVING FILE: " << e.what() << " ---"
              << std::endl;

    g_current_machine = NULL;
    throw e;
  }

  g_current_machine = NULL;
}

fs::path buildSaveGameFilename(RLMachine& machine, int slot) {
  std::ostringstream oss;
  oss << "save" << std::setw(3) << std::setfill('0') << slot << ".sav.gz";

  return machine.system().GameSaveDirectory() / oss.str();
}

SaveGameHeader loadHeaderForSlot(RLMachine& machine, int slot) {
  fs::path path = buildSaveGameFilename(machine, slot);
  fs::ifstream file(path, std::ios::binary);
  checkInFileOpened(file, path);

  return loadHeaderFrom(file);
}

SaveGameHeader loadHeaderFrom(std::istream& iss) {
  boost::iostreams::filtering_stream<boost::iostreams::input> filtered_input;
  filtered_input.push(boost::iostreams::zlib_decompressor());
  filtered_input.push(iss);

  int version;
  SaveGameHeader header;

  // Only load the header
  boost::archive::text_iarchive ia(filtered_input);
  ia >> version >> header;

  return header;
}

void loadLocalMemoryForSlot(RLMachine& machine, int slot, Memory& memory) {
  fs::path path = buildSaveGameFilename(machine, slot);
  fs::ifstream file(path, std::ios::binary);
  checkInFileOpened(file, path);

  loadLocalMemoryFrom(file, memory);
}

void loadLocalMemoryFrom(std::istream& iss, Memory& memory) {
  boost::iostreams::filtering_stream<boost::iostreams::input> filtered_input;
  filtered_input.push(boost::iostreams::zlib_decompressor());
  filtered_input.push(iss);

  int version;
  SaveGameHeader header;

  // Only load the header
  boost::archive::text_iarchive ia(filtered_input);
  ia >> version >> header >> memory.local();
}

void loadGameForSlot(RLMachine& machine, int slot) {
  fs::path path = buildSaveGameFilename(machine, slot);
  fs::ifstream file(path, std::ios::binary);
  checkInFileOpened(file, path);

  loadGameFrom(file, machine);
}

void loadGameFrom(std::istream& iss, RLMachine& machine) {
  boost::iostreams::filtering_stream<boost::iostreams::input> filtered_input;
  filtered_input.push(boost::iostreams::zlib_decompressor());
  filtered_input.push(iss);

  int version;
  SaveGameHeader header;

  g_current_machine = &machine;

  try {
    // Must clear the stack before reseting the System because LongOperations
    // often hold references to objects in the System heiarchy.
    machine.Reset();

    boost::archive::text_iarchive ia(filtered_input);
    ia >> version >> header >> machine.memory().local() >> machine >>
        machine.system() >> machine.system().graphics() >>
        machine.system().text() >> machine.system().sound();

    machine.system().graphics().ReplayGraphicsStack(machine);

    machine.system().graphics().ForceRefresh();
  }
  catch (std::exception& e) {
    std::cerr << "--- WARNING: ERROR DURING LOADING FILE: " << e.what()
              << " ---" << std::endl;

    g_current_machine = NULL;
    throw e;
  }

  g_current_machine = NULL;
}

}  // namespace Serialization
