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

#include "MachineBase/Serialization.hpp"

// include headers that implement a archive in simple text format
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/map.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <fstream>
#include <sstream>
#include <iostream>

#include "Utilities/Exception.hpp"
#include "Utilities/dynamic_bitset_serialize.hpp"
#include "MachineBase/RLMachine.hpp"
#include "MachineBase/Memory.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/EventSystem.hpp"
#include "Systems/Base/TextSystem.hpp"
#include "Systems/Base/SoundSystem.hpp"
#include "Utilities/gettext.h"
#include "libReallive/intmemref.h"

using namespace std;
using namespace libReallive;
using namespace boost::archive;
namespace fs = boost::filesystem;

namespace Serialization {

// - Was at 2 was most of rlvm's lifetime.
// - Was changed to 3 during the 0.7 release because boost 1.35 had a serious
//   bug in its implementation of vectors of primitive types which made
//   archives not-backwards (or forwards) compatible. Thankfully, the save
//   games themselves don't use that feature.
const int CURRENT_GLOBAL_VERSION = 3;

fs::path buildGlobalMemoryFilename(RLMachine& machine) {
  return machine.system().gameSaveDirectory() / "global.sav.gz";
}

void saveGlobalMemory(RLMachine& machine) {
  fs::path home = buildGlobalMemoryFilename(machine);
  fs::ofstream file(home, ios::binary);
  if (!file) {
    throw rlvm::Exception(_("Could not open global memory file."));
  }

  saveGlobalMemoryTo(file, machine);
}

void saveGlobalMemoryTo(std::ostream& oss, RLMachine& machine) {
  using namespace boost::iostreams;
  filtering_stream<output> filtered_output;
  filtered_output.push(zlib_compressor());
  filtered_output.push(oss);

  text_oarchive oa(filtered_output);
  System& sys = machine.system();

  oa << CURRENT_GLOBAL_VERSION
     << const_cast<const GlobalMemory&>(machine.memory().global())
     << const_cast<const SystemGlobals&>(sys.globals())
     << const_cast<const GraphicsSystemGlobals&>(sys.graphics().globals())
     << const_cast<const EventSystemGlobals&>(sys.event().globals())
     << const_cast<const TextSystemGlobals&>(sys.text().globals())
     << const_cast<const SoundSystemGlobals&>(sys.sound().globals());
}

void loadGlobalMemory(RLMachine& machine) {
  fs::path home = buildGlobalMemoryFilename(machine);
  fs::ifstream file(home, ios::binary);

  // If we were able to open the file for reading, load it. Don't
  // complain if we're unable to, since this may be the first run on
  // this certain game and it may not exist yet.
  if (file) {
    try {
      loadGlobalMemoryFrom(file, machine);
    } catch(...) {
      // Swallow ALL exceptions during file reading. If loading the global
      // memory file fails in any way, something is EXTREMELY wrong. Either
      // we're trying to read an incompatible old version's files or the global
      // data is corrupted. Either way, we can't safely do ANYTHING with this
      // game's entire save data so move it out of the way.
      fs::path save_dir = machine.system().gameSaveDirectory();
      fs::path dest_save_dir = save_dir.parent_path() /
                               (save_dir.filename() / ".old_corrupted_data");

      if (fs::exists(dest_save_dir))
        fs::remove_all(dest_save_dir);
      fs::rename(save_dir, dest_save_dir);

      cerr << "WARNING: Unable to read saved global memory file. Moving "
           << save_dir << " to " << dest_save_dir << endl;
    }
  }
}

void loadGlobalMemoryFrom(std::istream& iss, RLMachine& machine) {
  using namespace boost::iostreams;
  filtering_stream<input> filtered_input;
  filtered_input.push(zlib_decompressor());
  filtered_input.push(iss);

  text_iarchive ia(filtered_input);
  System& sys = machine.system();
  int version;
  ia >> version;

  // Load global memory.
  ia >> machine.memory().global();

  // When Karmic Koala came out, support for all boost earlier than 1.36 was
  // dropped. For years, I had used boost 1.35 on Ubuntu. It turns out that
  // boost 1.35 had a serious bug in it, where it wouldn't save vectors of
  // primitive data types correctly. These global data files no longer load
  // correctly.
  //
  // After flirting with moving to Google protobuf (can't; doesn't handle
  // complex object graphs like GraphicsObject and its copy-on-write stuff),
  // and then trying to fix the problem in a forked copy of the serialization
  // headers which was unsuccessful, I'm just saying to hell with the user's
  // settings. Most people don't change these values and save games and global
  // memory still work (per above.)
  if (version == CURRENT_GLOBAL_VERSION) {
    ia >> sys.globals()
       >> sys.graphics().globals()
       >> sys.event().globals()
       >> sys.text().globals()
       >> sys.sound().globals();

    // Restore options which may have System specific implementations. (This
    // will probably expand as more of RealLive is implemented).
    sys.sound().restoreFromGlobals();
  }
}

}  // namespace Serialization
