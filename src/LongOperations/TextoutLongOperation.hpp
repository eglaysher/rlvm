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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// -----------------------------------------------------------------------

#ifndef SRC_LONGOPERATIONS_TEXTOUTLONGOPERATION_HPP_
#define SRC_LONGOPERATIONS_TEXTOUTLONGOPERATION_HPP_

#include "MachineBase/LongOperation.hpp"
#include "Systems/Base/EventListener.hpp"

#include <string>

class RLMachine;

class TextoutLongOperation : public LongOperation {
 public:
  TextoutLongOperation(RLMachine& machine, const std::string& utf8string);
  ~TextoutLongOperation();

  void setNoWait(bool in = true) { no_wait_ = in; }

  // Overriden from EventListener:
  virtual bool mouseButtonStateChanged(MouseButton mouseButton, bool pressed);
  virtual bool keyStateChanged(KeyCode keyCode, bool pressed);

  // Overriden from LongOperation:
  virtual bool operator()(RLMachine& machine);
  virtual bool sleepEveryTick();

 private:
  bool displayAsMuchAsWeCanThenPause(RLMachine& machine);

  // Extract a name and send it to the text system as an automic
  // operation.
  bool displayName(RLMachine& machine);
  bool displayOneMoreCharacter(RLMachine& machine, bool& paused);

  std::string m_utf8string;

  int current_codepoint_;
  std::string current_char_;
  std::string::iterator current_position_;

  // Sets whether we should display as much text as we can immediately.
  bool no_wait_;
};

#endif  // SRC_LONGOPERATIONS_TEXTOUTLONGOPERATION_HPP_
