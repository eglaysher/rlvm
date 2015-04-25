// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006, 2007 Elliot Glaysher
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

#include "modules/module_debug.h"

#include <string>
#include <iostream>

#include "libreallive/gameexe.h"
#include "machine/rlmachine.h"
#include "machine/rloperation.h"
#include "systems/base/system.h"
#include "utilities/string_utilities.h"

// The Debug Module (mod<1:255)
//
// Module that defines runtime debugging operations. The following
// operations are only executed when \#MEMORY is defined in the
// incoming Gameexe file.

namespace {

struct DebugMessageInt : public RLOpcode<IntConstant_T> {
  void operator()(RLMachine& machine, int value) {
    if (machine.system().gameexe()("MEMORY").Exists())
      std::cerr << "DebugMessage: " << value << std::endl;
  }
};

struct DebugMessageStr : public RLOpcode<StrConstant_T> {
  void operator()(RLMachine& machine, std::string value) {
    if (machine.system().gameexe()("MEMORY").Exists()) {
      std::string utfvalue = cp932toUTF8(value, machine.GetTextEncoding());
      std::cerr << "DebugMessage: " << utfvalue << std::endl;
    }
  }
};

}  // namespace

DebugModule::DebugModule() : RLModule("Debug", 1, 255) {
  AddOpcode(10, 0, "__DebugMessage", new DebugMessageInt);
  AddOpcode(10, 1, "__DebugMessage", new DebugMessageStr);
}
