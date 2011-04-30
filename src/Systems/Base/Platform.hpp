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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
// -----------------------------------------------------------------------


#ifndef SRC_SYSTEMS_BASE_PLATFORM_HPP_
#define SRC_SYSTEMS_BASE_PLATFORM_HPP_

#include <map>
#include <string>
#include <vector>

class Gameexe;
class RLMachine;
class RlvmInfo;

// Encapsulates platform specific details; this is mainly dealing with the
// underlying widget set.
class Platform {
 public:
  // Parses out strings that are presented to the user as part of the platform.
  explicit Platform(Gameexe& gameexe);
  virtual ~Platform();

  // Called on a right click where the game doesn't have its own syscom
  // handler.
  virtual void showNativeSyscomMenu(RLMachine& machine) = 0;

  // Invokes a standard dialog.
  virtual void invokeSyscomStandardUI(RLMachine& machine, int syscom) = 0;

  // Displays the current interpreter info.
  virtual void showSystemInfo(RLMachine& machine, const RlvmInfo& info) = 0;

 protected:
  // Specifies a single entry in the menu.
  struct MenuSpec {
    MenuSpec(int id);
    MenuSpec(int id, const char* label);
    MenuSpec(int id, const char* label, const std::vector<MenuSpec>& submenu);

    // Syscom id >= 0, or a MENU* thing.
    int syscom_id;

    // User interface string key. If NULL, converts |syscom_id| to a string.
    const char* label;

    // Specification of the child menu.
    std::vector<MenuSpec> submenu;
  };

  static const int MENU_SEPARATOR = -1;
  static const int MENU = -2;

  // Returns a game specific string from the Gameexe.ini file.
  std::string syscomString(const std::string& key) const;

  // Returns the display label for |item|.
  std::string GetMenuLabel(const MenuSpec& item) const;

  // Returns a cross platform description of the syscom menu.
  void GetMenuSpecification(RLMachine& machine,
                            std::vector<MenuSpec>& out_menu);

 private:
  // Strips quotes off of value and adds it to our internal strings database.
  void addSyscomStringFor(const std::string& key, const std::string& value);

  // Parsed out syscom strings from the Gameexe.ini file. Gameexe.ini files
  // are in some input encoding (usually Shift_JIS), but the text system
  // expects UTF-8, so pre-parse them.
  std::map<std::string, std::string> utf8_syscom_strings_;
};

#endif  // SRC_SYSTEMS_BASE_PLATFORM_HPP_
