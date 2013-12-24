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

#include "MachineBase/GameHacks.hpp"

#include <functional>

#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/System.hpp"
#include "libReallive/gameexe.h"

using namespace std;

namespace {

void PBRIDE_ResetAutoMode(RLMachine& machine) {
  // During the first ending credits, if you click to skip them, the draw mode
  // doesn't automatically get reset to DrawAuto. RealLive.exe takes care of
  // this (draw mode on the stack, perhaps?), but until we know what causes
  // this, hack.
  machine.system().graphics().setScreenUpdateMode(
      GraphicsSystem::SCREENUPDATEMODE_AUTOMATIC);
}

void LB_SkipBaseball(RLMachine& machine) {
  // Baseball is a weird minigame that requires talking to a DLL. :( We will
  // *never* emulate it properly without reverse engineering what the DLL does.
  machine.returnFromFarcall();
}

}  // namespace

void addGameHacks(RLMachine& machine) {
  std::string diskmark = machine.system().gameexe()("DISKMARK");

  if (diskmark == "P_BRIDE_SE.ENV") {
    machine.addLineAction(310, 446, bind(PBRIDE_ResetAutoMode, ref(machine)));
  } else if (diskmark == "LB.ENV" || diskmark == "LB_EX.ENV") {
    machine.addLineAction(7030, 15, bind(LB_SkipBaseball, ref(machine)));
  }
}
