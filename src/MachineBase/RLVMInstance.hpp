// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2011 Elliot Glaysher
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
// -----------------------------------------------------------------------

#ifndef SRC_MACHINEBASE_RLVMINSTANCE_hpp_
#define SRC_MACHINEBASE_RLVMINSTANCE_hpp_

#include <boost/filesystem/operations.hpp>

class Platform;
class System;

// The main, cross platform emulator class. Has template methods for
// implementing platform specific GUI.
class RLVMInstance {
 public:
  RLVMInstance();
  virtual ~RLVMInstance();

  // Runs the main emulation loop.
  void Run(const boost::filesystem::path& gamepath);

  void set_seen_start(int in) { seen_start_ = in; }
  void set_memory() { memory_ = true; }
  void set_undefined_opcodes() { undefined_opcodes_ = true; }
  void set_count_undefined() { count_undefined_copcodes_ = true; }
  void set_load_save(int in) { load_save_ = in; }

  // Optionally brings up a file selection dialog to get the game directory. In
  // case this isn't implemented or the user clicks cancel, returns an empty
  // path.
  virtual boost::filesystem::path SelectGameDirectory();

 protected:
  // Should bring up a platform native dialog box to report the message.
  virtual void ReportFatalError(const std::string& message_text,
                                const std::string& informative_text);

  // Gives the native subclass a chance to do work during the main loop.
  virtual void DoNativeWork() {}

  // Returns a Platform object that displays native UI.
  virtual Platform* BuildNativePlatform(System& system);

 private:
  // Finds a game file, causing an error if not found.
  boost::filesystem::path FindGameFile(
      const boost::filesystem::path& gamerootPath,
      const std::string& filename);

  // Checks for AVG32/Siglus engine games, which people may be confused about.
  void CheckBadEngine(const boost::filesystem::path& gamerootPath,
                      const char** filenames,
                      const std::string& message_text);

  // Which SEEN# we should start execution from (-1 if we shouldn't set this).
  int seen_start_;

  // Whether we should force '#MEMORY=1' to enter debug mode.
  bool memory_;

  // Whether we should print undefined opcode messages.
  bool undefined_opcodes_;

  // Whether we should print out a table of undefined opcodes that the game
  // used on exit.
  bool count_undefined_copcodes_;

  // Loads the specified save file as soon as emulation starts if not -1.
  int load_save_;
};

#endif  // SRC_MACHINEBASE_RLVMINSTANCE_hpp_
