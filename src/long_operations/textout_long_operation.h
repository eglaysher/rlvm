// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006 Elliot Glaysher
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

#ifndef SRC_LONG_OPERATIONS_TEXTOUT_LONG_OPERATION_H_
#define SRC_LONG_OPERATIONS_TEXTOUT_LONG_OPERATION_H_

#include <string>

#include "machine/long_operation.h"
#include "systems/base/event_listener.h"

class RLMachine;

class TextoutLongOperation : public LongOperation {
 public:
  TextoutLongOperation(RLMachine& machine, const std::string& utf8string);
  virtual ~TextoutLongOperation();

  void set_no_wait() { no_wait_ = true; }

  // Overriden from EventListener:
  virtual bool MouseButtonStateChanged(MouseButton mouseButton, bool pressed);
  virtual bool KeyStateChanged(KeyCode keyCode, bool pressed);

  // Overriden from LongOperation:
  virtual bool operator()(RLMachine& machine);

 private:
  bool DisplayAsMuchAsWeCanThenPause(RLMachine& machine);

  // Extract a name and send it to the text system as an automic
  // operation.
  bool DisplayName(RLMachine& machine);
  bool DisplayOneMoreCharacter(RLMachine& machine, bool& paused);

  std::string utf8_string_;

  int current_codepoint_;
  std::string current_char_;
  std::string::iterator current_position_;

  // Sets whether we should display as much text as we can immediately.
  bool no_wait_;

  // How long it's been since the last time we've added time to |total_time_|.
  static unsigned int time_at_last_pass_;

  // A countdown in milliseconds until we display the next character.
  static int next_character_countdown_;
};

#endif  // SRC_LONG_OPERATIONS_TEXTOUT_LONG_OPERATION_H_
