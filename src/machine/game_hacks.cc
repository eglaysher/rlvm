// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2009 Elliot Glaysher
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

#include "machine/game_hacks.h"

#include <functional>
#include <string>

#include "libreallive/gameexe.h"
#include "machine/rlmachine.h"
#include "systems/base/graphics_system.h"
#include "systems/base/system.h"

using std::bind;
using std::ref;

namespace {

void PBRIDE_ResetAutoMode(RLMachine& machine) {
  // During the first ending credits, if you click to skip them, the draw mode
  // doesn't automatically get reset to DrawAuto. RealLive.exe takes care of
  // this (draw mode on the stack, perhaps?), but until we know what causes
  // this, hack.
  machine.system().graphics().SetScreenUpdateMode(
      GraphicsSystem::SCREENUPDATEMODE_AUTOMATIC);
}

void LB_SkipBaseball(RLMachine& machine) {
  // Baseball is a weird minigame that requires talking to a DLL. :( We will
  // *never* emulate it properly without reverse engineering what the DLL does.
  machine.ReturnFromFarcall();
}

}  // namespace

void AddGameHacks(RLMachine& machine) {
  std::string diskmark = machine.system().gameexe()("DISKMARK");

  if (diskmark == "P_BRIDE_SE.ENV") {
    machine.AddLineAction(310, 446, bind(PBRIDE_ResetAutoMode, ref(machine)));
  } else if (diskmark == "LB.ENV" || diskmark == "LB_EX.ENV") {
    machine.AddLineAction(7030, 15, bind(LB_SkipBaseball, ref(machine)));
  }
}
