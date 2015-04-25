// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2013 Elliot Glaysher
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

#include "modules/module_g00.h"

#include <string>

#include "machine/general_operations.h"
#include "systems/base/graphics_system.h"
#include "systems/base/system.h"

struct g00Preload : public RLOpcode<IntConstant_T, StrConstant_T> {
  void operator()(RLMachine& machine, int slot, string name) {
    machine.system().graphics().PreloadG00(slot, name);
  }
};

G00Module::G00Module() : RLModule("G00", 1, 14) {
  AddOpcode(1000, 0, "g00Preload", new g00Preload);
  AddOpcode(
      1001, 0, "g00Clear", CallFunction(&GraphicsSystem::ClearPreloadedG00));
  AddOpcode(1002,
            0,
            "g00ClearAll",
            CallFunction(&GraphicsSystem::ClearAllPreloadedG00));
}
