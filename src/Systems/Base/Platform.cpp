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

#include "Systems/Base/Platform.hpp"

#include <iomanip>
#include <map>
#include <sstream>
#include <string>

#include "libReallive/gameexe.h"
#include "Systems/Base/System.hpp"
#include "Utilities/StringUtilities.hpp"

using std::string;

const char* ADDTIONAL_STRINGS_TO_LOAD[] = {
  "SAVE_NODATA",
  "SYSTEM_SAVELOADMESSAGE_STR",
  "SAVEMESSAGE_TITLE_STR",
  "SAVEMESSAGE_MESS_STR",
  "SAVEMESSAGE_MESS2_STR",
  "LOADMESSAGE_TITLE_STR",
  "LOADMESSAGE_MESS_STR",
  "LOADMESSAGE_MESS2_STR",
  "DLGSAVEMESSAGE_TITLE_STR",
  "DLGSAVEMESSAGE_OK_BUTTON_STR",
  "DLGLOADMESSAGE_TITLE_STR",
  "DLGLOADMESSAGE_OK_BUTTON_STR",
  "SYSTEM_ANIME_STR",
  "SELPOINT_RETURN_MESS_STR",
  "VERSION_STR",
  "MENU_RETURN_MESS_STR",
  "GAME_END_MESS_STR",
  NULL
};

Platform::MenuSpec::MenuSpec(int id) : syscom_id(id), label(NULL) {}

Platform::MenuSpec::MenuSpec(int id, const char* l)
    : syscom_id(id),
      label(l) {}

Platform::MenuSpec::MenuSpec(int id,
                             const char* l,
                             const std::vector<MenuSpec>& s)
    : syscom_id(id),
      label(l),
      submenu(s) {}

// -----------------------------------------------------------------------
// Platform
// -----------------------------------------------------------------------
Platform::Platform(Gameexe& gameexe) {
  // Parse the Gameexe.ini syscom entries.
  GameexeFilteringIterator end = gameexe.filtering_end();
  for (GameexeFilteringIterator it = gameexe.filtering_begin("SYSCOM.");
       it != end; ++it) {
    string key = it->key().substr(7);
    if (key.size() == 3 && isdigit(key[0]) && isdigit(key[1]) &&
        isdigit(key[2])) {
      addSyscomStringFor(key, it->to_string());
    } else if (key.size() == 7 && isdigit(key[0]) && isdigit(key[1]) &&
               isdigit(key[2]) && key[3] == '.' && isdigit(key[4]) &&
               isdigit(key[5]) && isdigit(key[6])) {
      addSyscomStringFor(key, it->to_string());
    }
  }

  for (int i = 0; ADDTIONAL_STRINGS_TO_LOAD[i] != NULL; ++i) {
    GameexeInterpretObject toload = gameexe(ADDTIONAL_STRINGS_TO_LOAD[i]);
    if (toload.exists())
      addSyscomStringFor(toload.key(), toload.to_string());
  }
}

Platform::~Platform() {}

std::string Platform::syscomString(const std::string& key) const {
  std::map<std::string, std::string>::const_iterator it =
    utf8_syscom_strings_.find(key);
  if (it != utf8_syscom_strings_.end())
    return it->second;
  else
    return "";
}

std::string Platform::GetMenuLabel(const MenuSpec& item) const {
  std::string label;
  if (item.label == NULL) {
    std::ostringstream labelss;
    labelss << std::setw(3) << std::setfill('0') << item.syscom_id;
    label = syscomString(labelss.str());
  } else {
    label = syscomString(item.label);
  }

  return label;
}

void Platform::GetMenuSpecification(RLMachine& machine,
                                    std::vector<MenuSpec>& out_menu) {
  out_menu.push_back(MenuSpec(SYSCOM_SET_SKIP_MODE));
  out_menu.push_back(MenuSpec(SYSCOM_AUTO_MODE));
  out_menu.push_back(MenuSpec(SYSCOM_SHOW_BACKGROUND));
  out_menu.push_back(MenuSpec(MENU_SEPARATOR));
  out_menu.push_back(MenuSpec(SYSCOM_SAVE));
  out_menu.push_back(MenuSpec(SYSCOM_LOAD));
  out_menu.push_back(MenuSpec(SYSCOM_RETURN_TO_PREVIOUS_SELECTION));
  out_menu.push_back(MenuSpec(MENU_SEPARATOR));

  std::vector<MenuSpec> return_to_menu;
  return_to_menu.push_back(MenuSpec(SYSCOM_HIDE_MENU, "028.000"));
  return_to_menu.push_back(MenuSpec(SYSCOM_MENU_RETURN, "028.001"));
  out_menu.push_back(MenuSpec(MENU, "028", return_to_menu));

  std::vector<MenuSpec> exit_game_menu;
  exit_game_menu.push_back(MenuSpec(SYSCOM_HIDE_MENU, "029.000"));
  exit_game_menu.push_back(MenuSpec(SYSCOM_EXIT_GAME, "029.001"));
  out_menu.push_back(MenuSpec(MENU, "029", exit_game_menu));
}

void Platform::addSyscomStringFor(const std::string& key,
                                  const std::string& value) {
  std::string::size_type front = value.find('"');
  std::string::size_type back = value.rfind('"');
  std::string unquoted = string(value, front + 1, back);

  // TODO: Use the correct encoding...
  std::string utf8 = cp932toUTF8(unquoted, 0);

  utf8_syscom_strings_[key] = utf8;
}
