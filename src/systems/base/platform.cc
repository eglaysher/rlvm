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

#include "systems/base/platform.h"

#include <map>
#include <string>

#include "libreallive/gameexe.h"
#include "utilities/string_utilities.h"

using std::string;

const char* ADDTIONAL_STRINGS_TO_LOAD[] = {
    "SAVE_NODATA",              "SYSTEM_SAVELOADMESSAGE_STR",
    "SAVEMESSAGE_TITLE_STR",    "SAVEMESSAGE_MESS_STR",
    "SAVEMESSAGE_MESS2_STR",    "LOADMESSAGE_TITLE_STR",
    "LOADMESSAGE_MESS_STR",     "LOADMESSAGE_MESS2_STR",
    "DLGSAVEMESSAGE_TITLE_STR", "DLGSAVEMESSAGE_OK_BUTTON_STR",
    "DLGLOADMESSAGE_TITLE_STR", "DLGLOADMESSAGE_OK_BUTTON_STR",
    "SYSTEM_ANIME_STR",         "SELPOINT_RETURN_MESS_STR",
    "VERSION_STR",              "MENU_RETURN_MESS_STR",
    "GAME_END_MESS_STR",        NULL};

// -----------------------------------------------------------------------
// Platform
// -----------------------------------------------------------------------
Platform::Platform(Gameexe& gameexe) {
  // Parse the Gameexe.ini syscom entries.
  GameexeFilteringIterator end = gameexe.filtering_end();
  for (GameexeFilteringIterator it = gameexe.filtering_begin("SYSCOM.");
       it != end;
       ++it) {
    string key = it->key().substr(7);
    if (key.size() == 3 && isdigit(key[0]) && isdigit(key[1]) &&
        isdigit(key[2])) {
      AddSyscomStringFor(key, it->ToString());
    } else if (key.size() == 7 && isdigit(key[0]) && isdigit(key[1]) &&
               isdigit(key[2]) && key[3] == '.' && isdigit(key[4]) &&
               isdigit(key[5]) && isdigit(key[6])) {
      AddSyscomStringFor(key, it->ToString());
    }
  }

  for (int i = 0; ADDTIONAL_STRINGS_TO_LOAD[i] != NULL; ++i) {
    GameexeInterpretObject toload = gameexe(ADDTIONAL_STRINGS_TO_LOAD[i]);
    if (toload.Exists())
      AddSyscomStringFor(toload.key(), toload.ToString());
  }
}

Platform::~Platform() {}

std::string Platform::GetSyscomString(const std::string& key) const {
  std::map<std::string, std::string>::const_iterator it =
      utf8_syscom_strings_.find(key);
  if (it != utf8_syscom_strings_.end())
    return it->second;
  else
    return "";
}

void Platform::AddSyscomStringFor(const std::string& key,
                                  const std::string& value) {
  std::string::size_type front = value.find('"');
  std::string::size_type back = value.rfind('"');
  std::string unquoted = string(value, front + 1, back);

  // TODO(erg): Use the correct encoding...
  std::string utf8 = cp932toUTF8(unquoted, 0);

  utf8_syscom_strings_[key] = utf8;
}
