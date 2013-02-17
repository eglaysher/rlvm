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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
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
#include "Systems/Base/TextPage.hpp"
#include "Systems/Base/TextSystem.hpp"

namespace {

void addKoeIcon(RLMachine& machine, int id) {
  machine.system().text().currentPage().koeMarker(id);
}

bool koeIsPlaying(RLMachine& machine) {
  return !machine.system().sound().koePlaying();
}

void addKoeWaitC(RLMachine& machine) {
  WaitLongOperation* wait_op = new WaitLongOperation(machine);
  wait_op->breakOnClicks();
  wait_op->breakOnEvent(boost::bind(koeIsPlaying, boost::ref(machine)));

  machine.pushLongOperation(wait_op);
}

void addKoeWait(RLMachine& machine) {
  WaitLongOperation* wait_op = new WaitLongOperation(machine);
  wait_op->breakOnEvent(boost::bind(koeIsPlaying, boost::ref(machine)));

  machine.pushLongOperation(wait_op);
}

struct koePlay_0 : public RLOp_Void_1<IntConstant_T> {
  void operator()(RLMachine& machine, int koe) {
    machine.system().sound().koePlay(koe);
    addKoeIcon(machine, koe);
  }
};

struct koePlay_1 : public RLOp_Void_2<IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, int koe, int character) {
    machine.system().sound().koePlay(koe, character);
    addKoeIcon(machine, koe);
  }
};

struct koePlayEx_0 : public RLOp_Void_1<IntConstant_T> {
  void operator()(RLMachine& machine, int koe) {
    machine.system().sound().koePlay(koe);
    addKoeIcon(machine, koe);
    addKoeWait(machine);
  }
};

struct koePlayEx_1 : public RLOp_Void_2<IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, int koe, int character) {
    machine.system().sound().koePlay(koe, character);
    addKoeIcon(machine, koe);
    addKoeWait(machine);
  }
};

struct koeDoPlayEx_1 : public RLOp_Void_2<IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, int koe, int character) {
    machine.system().sound().koePlay(koe);
    addKoeIcon(machine, koe);
    addKoeWait(machine);
  }
};

struct koePlayExC_0 : public RLOp_Void_1<IntConstant_T> {
  void operator()(RLMachine& machine, int koe) {
    machine.system().sound().koePlay(koe);
    addKoeIcon(machine, koe);
    addKoeWaitC(machine);
  }
};

struct koePlayExC_1 : public RLOp_Void_2<IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, int koe, int character) {
    machine.system().sound().koePlay(koe, character);
    addKoeIcon(machine, koe);
    addKoeWait(machine);
  }
};

struct koeDoPlayExC_1 : public RLOp_Void_2<IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, int koe, int character) {
    machine.system().sound().koePlay(koe);
    addKoeIcon(machine, koe);
    addKoeWaitC(machine);
  }
};

struct koeWait : public RLOp_Void_Void {
  void operator()(RLMachine& machine) {
    addKoeWait(machine);
  }
};

struct koeWaitC : public RLOp_Void_Void {
  void operator()(RLMachine& machine) {
    addKoeWaitC(machine);
  }
};

// Play the voice not taking |character| into account.
struct koeDoPlay_1 : public RLOp_Void_2<IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, int koe, int character) {
    machine.system().sound().koePlay(koe);
  }
};

// We ignore fadein because we'll never get that effect with the
// current mixing library.
struct koeSetVolume_0 : public RLOp_Void_1<IntConstant_T> {
  void operator()(RLMachine& machine, int level) {
    machine.system().sound().setKoeVolume(level, 0);
  }
};

struct koeUnMute_1 : public RLOp_Void_1<IntConstant_T> {
  void operator()(RLMachine& machine, int fadein) {
    machine.system().sound().setKoeVolume(255, fadein);
  }
};

struct koeMute_1 : public RLOp_Void_1<IntConstant_T> {
  void operator()(RLMachine& machine, int fadein) {
    machine.system().sound().setKoeVolume(0, fadein);
  }
};

}  // namespace

// -----------------------------------------------------------------------

KoeModule::KoeModule()
  : RLModule("Koe", 1, 23) {
  addOpcode(0, 0, "koePlay", new koePlay_0);
  addOpcode(0, 1, "koePlay", new koePlay_1);

  addOpcode(1, 0, "koePlayEx", new koePlayEx_0);
  addOpcode(1, 1, "koePlayEx", new koePlayEx_1);

  addOpcode(3, 0, "koeWait", new koeWait);
  addOpcode(4, 0, "koePlaying", returnIntValue(&SoundSystem::koePlaying));
  addOpcode(5, 0, "koeStop", callFunction(&SoundSystem::koeStop));
  addOpcode(6, 0, "koeWaitC", new koeWaitC);

  addOpcode(7, 0, "koePlayExC", new koePlayExC_0);
  addOpcode(7, 1, "koePlayExC", new koePlayExC_1);

  addOpcode(8, 0, "koeDoPlay", new koePlay_0);
  addOpcode(8, 1, "koeDoPlay", new koeDoPlay_1);

  addOpcode(9, 0, "koeDoPlayEx", new koePlayEx_0);
  addOpcode(9, 1, "koeDoPlayEx", new koeDoPlayEx_1);

  addOpcode(10, 0, "koeDoPlayExC", new koePlayExC_0);
  addOpcode(10, 1, "koeDoPlayExC", new koeDoPlayExC_1);

  addOpcode(11, 0, "koeVolume", returnIntValue(&SoundSystem::koeVolume));

  addOpcode(12, 0, "koeSetVolume", new koeSetVolume_0);
  addOpcode(12, 1, "koeSetVolume", callFunction(&SoundSystem::setKoeVolume));

  addOpcode(13, 0, "koeUnMute",
            callFunctionWith(&SoundSystem::setKoeVolume, 255, 0));
  addOpcode(13, 1, "koeUnMute", new koeUnMute_1);

  addOpcode(14, 0, "koeMute",
            callFunctionWith(&SoundSystem::setKoeVolume, 0, 0));
  addOpcode(14, 1, "koeMute", new koeMute_1);
}
