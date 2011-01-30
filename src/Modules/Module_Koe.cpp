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

namespace {

struct LongOp_koeWait : public LongOperation {
  bool operator()(RLMachine& machine) {
    return !machine.system().sound().koePlaying();
  }
};

struct koePlayEx_0 : public RLOp_Void_1<IntConstant_T> {
  void operator()(RLMachine& machine, int koe) {
    machine.system().sound().koePlay(koe);
    machine.pushLongOperation(new LongOp_koeWait);
  }
};

struct koePlayEx_1 : public RLOp_Void_2<IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, int koe, int character) {
    machine.system().sound().koePlay(koe, character);
    machine.pushLongOperation(new LongOp_koeWait);
  }
};

struct koeDoPlayEx_1 : public RLOp_Void_2<IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, int koe, int character) {
    machine.system().sound().koePlay(koe);
    machine.pushLongOperation(new LongOp_koeWait);
  }
};

struct koeWait : public RLOp_Void_Void {
  void operator()(RLMachine& machine) {
    machine.pushLongOperation(new LongOp_koeWait);
  }
};

struct koeWaitC : public RLOp_Void_Void {
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

// Play the voice not taking |character| into account.
struct koeDoPlay2 : public RLOp_Void_2<IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, int koe, int character) {
    machine.system().sound().koePlay(koe);
  }
};

}  // namespace

// -----------------------------------------------------------------------

KoeModule::KoeModule()
  : RLModule("Koe", 1, 23) {
  addOpcode(0, 0, "koePlay", callFunction(
      static_cast<void(SoundSystem::*)(int)>(&SoundSystem::koePlay)));
  addOpcode(0, 1, "koePlay", callFunction(
      static_cast<void(SoundSystem::*)(int, int)>(&SoundSystem::koePlay)));

  addOpcode(1, 0, "koePlayEx", new koePlayEx_0);
  addOpcode(1, 1, "koePlayEx", new koePlayEx_1);

  addOpcode(3, 0, "koeWait", new koeWait);
  addOpcode(4, 0, "koePlaying", returnIntValue(&SoundSystem::koePlaying));
  addOpcode(5, 0, "koeStop", callFunction(&SoundSystem::koeStop));
  addOpcode(6, 0, "koeWaitC", new koeWaitC);

  addUnsupportedOpcode(7, 0, "koePlayExC");
  addUnsupportedOpcode(7, 1, "koePlayExC");

  addOpcode(8, 0, "koeDoPlay", callFunction(
      static_cast<void(SoundSystem::*)(int)>(&SoundSystem::koePlay)));
  addOpcode(8, 1, "koeDoPlay", new koeDoPlay2);

  addOpcode(9, 0, "koeDoPlayEx", new koePlayEx_0);
  addOpcode(9, 1, "koeDoPlayEx", new koeDoPlayEx_1);

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
