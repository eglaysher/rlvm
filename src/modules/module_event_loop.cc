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

#include "modules/module_event_loop.h"

#include "machine/general_operations.h"
#include "machine/rlmachine.h"
#include "systems/base/graphics_system.h"
#include "systems/base/system.h"
#include "systems/base/text_system.h"

namespace {

struct setOverride : public RLOp_Void_1<IntConstant_T> {
  bool value_;
  explicit setOverride(int value) : value_(value) {}

  void operator()(RLMachine& machine, int window) {
    machine.system().text().setVisualOverride(window, value_);
  }
};

}  // namespace

EventLoopModule::EventLoopModule() : RLModule("EventLoop", 0, 4) {
  AddUnsupportedOpcode(120, 0, "SetInterrupt");
  AddUnsupportedOpcode(121, 0, "ClearInterrupt");
  AddUnsupportedOpcode(303, 0, "yield");

  // Theoretically the same as rtl, but we don't really know.
  AddOpcode(300, 0, "rtlButton", CallFunction(&RLMachine::ReturnFromFarcall));
  AddOpcode(301, 0, "rtlCancel", CallFunction(&RLMachine::ReturnFromFarcall));
  AddOpcode(302, 0, "rtlSystem", CallFunction(&RLMachine::ReturnFromFarcall));

  AddOpcode(1000,
            0,
            "ShowBackground",
            CallFunction(&GraphicsSystem::toggleInterfaceHidden));
  AddOpcode(
      1100, 0, "SetSkipMode", CallFunctionWith(&TextSystem::setSkipMode, 1));
  AddOpcode(
      1101, 0, "ClearSkipMode", CallFunctionWith(&TextSystem::setSkipMode, 0));
  AddOpcode(1102, 0, "SkipMode", ReturnIntValue(&TextSystem::skipMode));

  // opcode<0:4:1202, 0> and opcode<0:4:1200, 0> are used in the CLANNAD menu
  // system; no idea what they do.
  AddOpcode(1200, 0, "TextwindowOverrideShow", new setOverride(true));
  AddOpcode(1200,
            2,
            "TextwindowOverrideShow",
            CallFunctionWith(&TextSystem::setVisualOverrideAll, true));
  AddOpcode(1201, 0, "TextwindowOverrideHide", new setOverride(false));
  AddOpcode(1201,
            2,
            "TextwindowOverrideHide",
            CallFunctionWith(&TextSystem::setVisualOverrideAll, false));
  AddOpcode(1202,
            0,
            "ClearTextwindowOverrides",
            CallFunction(&TextSystem::clearVisualOverrides));
}
