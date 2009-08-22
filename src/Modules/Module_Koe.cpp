// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2007 Elliot Glaysher
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

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

#include "Modules/Module_Koe.hpp"

#include <boost/bind.hpp>

#include "LongOperations/WaitLongOperation.hpp"
#include "MachineBase/GeneralOperations.hpp"
#include "MachineBase/LongOperation.hpp"
#include "MachineBase/RLMachine.hpp"
#include "MachineBase/RLOperation.hpp"
#include "MachineBase/RLOperation/DefaultValue.hpp"
#include "Systems/Base/SoundSystem.hpp"
#include "Systems/Base/System.hpp"

// -----------------------------------------------------------------------

struct LongOp_koeWait : public LongOperation {
  bool operator()(RLMachine& machine) {
    return !machine.system().sound().koePlaying();
  }
};

// -----------------------------------------------------------------------

struct Koe_koeWait : public RLOp_Void_Void {
  void operator()(RLMachine& machine) {
    machine.pushLongOperation(new LongOp_koeWait);
  }
};

// -----------------------------------------------------------------------

struct Koe_koeWaitC : public RLOp_Void_Void {
  static bool isPlaying(RLMachine& machine) {
    return !machine.system().sound().koePlaying();
  }

  void operator()(RLMachine& machine) {
    WaitLongOperation* wait_op = new WaitLongOperation(machine);
    wait_op->breakOnClicks();
    wait_op->breakOnEvent(boost::bind(isPlaying, boost::ref(machine)));

    machine.pushLongOperation(wait_op);
  }
};

// -----------------------------------------------------------------------

KoeModule::KoeModule()
  : RLModule("Koe", 1, 23) {
  addOpcode(0, 0, "koePlay", callFunction(
                (void (SoundSystem::*)(int))&SoundSystem::koePlay));
  addOpcode(0, 1, "koePlay", callFunction(
                (void (SoundSystem::*)(int, int))&SoundSystem::koePlay));

  addUnsupportedOpcode(1, 0, "koePlayEx");
  addUnsupportedOpcode(1, 1, "koePlayEx");

  addOpcode(3, 0, "koeWait", new Koe_koeWait);
  addOpcode(4, 0, "koePlaying", returnIntValue(&SoundSystem::koePlaying));
  addOpcode(5, 0, "koeStop", callFunction(&SoundSystem::koeStop));
  addOpcode(6, 0, "koeWaitC", new Koe_koeWaitC);

  addUnsupportedOpcode(7, 0, "koePlayExC");
  addUnsupportedOpcode(7, 1, "koePlayExC");

  addUnsupportedOpcode(8, 0, "koeDoPlay");
  addUnsupportedOpcode(8, 1, "koeDoPlay");

  addUnsupportedOpcode(9, 0, "koeDoPlayEx");
  addUnsupportedOpcode(9, 1, "koeDoPlayEx");

  addUnsupportedOpcode(10, 0, "koeDoPlayExC");
  addUnsupportedOpcode(10, 1, "koeDoPlayExC");

  addUnsupportedOpcode(11, 0, "koeVolume");

  addUnsupportedOpcode(12, 0, "koeSetVolume");
  addUnsupportedOpcode(12, 1, "koeSetVolume");

  addUnsupportedOpcode(13, 0, "koeUnMute");
  addUnsupportedOpcode(13, 1, "koeUnMute");

  addUnsupportedOpcode(14, 0, "koeMute");
  addUnsupportedOpcode(14, 1, "koeMute");
}
