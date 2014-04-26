// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2008 Elliot Glaysher
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

#ifndef SRC_SYSTEMS_BASE_PLATFORM_H_
#define SRC_SYSTEMS_BASE_PLATFORM_H_

#include <map>
#include <string>

class Gameexe;
class RLMachine;
struct RlvmInfo;

// Encapsulates platform specific details; this is mainly dealing with the
// underlying widget set.
class Platform {
 public:
  // Parses out strings that are presented to the user as part of the platform.
  explicit Platform(Gameexe& gameexe);
  virtual ~Platform();

  // Returns a game specific string from the Gameexe.ini file.
  std::string GetSyscomString(const std::string& key) const;

  // Called every cycle.
  virtual void Run(RLMachine& machine) = 0;

  // Called on a right click where the game doesn't have its own syscom
  // handler.
  virtual void ShowNativeSyscomMenu(RLMachine& machine) = 0;

  // Invokes a standard dialog.
  virtual void InvokeSyscomStandardUI(RLMachine& machine, int syscom) = 0;

  // Displays the current interpreter info.
  virtual void ShowSystemInfo(RLMachine& machine, const RlvmInfo& info) = 0;

 private:
  // Strips quotes off of value and adds it to our internal strings database.
  void AddSyscomStringFor(const std::string& key, const std::string& value);

  // Parsed out syscom strings from the Gameexe.ini file. Gameexe.ini files
  // are in some input encoding (usually Shift_JIS), but the text system
  // expects UTF-8, so pre-parse them.
  std::map<std::string, std::string> utf8_syscom_strings_;
};

#endif  // SRC_SYSTEMS_BASE_PLATFORM_H_
