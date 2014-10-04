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

#include "modules/module_koe.h"

#include <functional>

#include "long_operations/wait_long_operation.h"
#include "machine/general_operations.h"
#include "machine/long_operation.h"
#include "machine/rlmachine.h"
#include "machine/rloperation.h"
#include "machine/rloperation/default_value.h"
#include "systems/base/sound_system.h"
#include "systems/base/system.h"
#include "systems/base/text_page.h"
#include "systems/base/text_system.h"

namespace {

void addKoeIcon(RLMachine& machine, int id) {
  machine.system().text().GetCurrentPage().KoeMarker(id);
}

bool koeIsPlaying(RLMachine& machine) {
  return !machine.system().sound().KoePlaying();
}

void addKoeWaitC(RLMachine& machine) {
  WaitLongOperation* wait_op = new WaitLongOperation(machine);
  wait_op->BreakOnClicks();
  wait_op->BreakOnEvent(std::bind(koeIsPlaying, std::ref(machine)));

  machine.PushLongOperation(wait_op);
}

void addKoeWait(RLMachine& machine) {
  WaitLongOperation* wait_op = new WaitLongOperation(machine);
  wait_op->BreakOnEvent(std::bind(koeIsPlaying, std::ref(machine)));

  machine.PushLongOperation(wait_op);
}

struct koePlay_0 : public RLOpcode<IntConstant_T> {
  void operator()(RLMachine& machine, int koe) {
    machine.system().sound().KoePlay(koe);
    addKoeIcon(machine, koe);
  }
};

struct koePlay_1 : public RLOpcode<IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, int koe, int character) {
    machine.system().sound().KoePlay(koe, character);
    addKoeIcon(machine, koe);
  }
};

struct koePlayEx_0 : public RLOpcode<IntConstant_T> {
  void operator()(RLMachine& machine, int koe) {
    machine.system().sound().KoePlay(koe);
    addKoeIcon(machine, koe);
    addKoeWait(machine);
  }
};

struct koePlayEx_1 : public RLOpcode<IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, int koe, int character) {
    machine.system().sound().KoePlay(koe, character);
    addKoeIcon(machine, koe);
    addKoeWait(machine);
  }
};

struct koeDoPlayEx_1 : public RLOpcode<IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, int koe, int character) {
    machine.system().sound().KoePlay(koe);
    addKoeIcon(machine, koe);
    addKoeWait(machine);
  }
};

struct koePlayExC_0 : public RLOpcode<IntConstant_T> {
  void operator()(RLMachine& machine, int koe) {
    machine.system().sound().KoePlay(koe);
    addKoeIcon(machine, koe);
    addKoeWaitC(machine);
  }
};

struct koePlayExC_1 : public RLOpcode<IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, int koe, int character) {
    machine.system().sound().KoePlay(koe, character);
    addKoeIcon(machine, koe);
    addKoeWait(machine);
  }
};

struct koeDoPlayExC_1 : public RLOpcode<IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, int koe, int character) {
    machine.system().sound().KoePlay(koe);
    addKoeIcon(machine, koe);
    addKoeWaitC(machine);
  }
};

struct koeWait : public RLOpcode<> {
  void operator()(RLMachine& machine) { addKoeWait(machine); }
};

struct koeWaitC : public RLOpcode<> {
  void operator()(RLMachine& machine) { addKoeWaitC(machine); }
};

// Play the voice not taking |character| into account.
struct koeDoPlay_1 : public RLOpcode<IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, int koe, int character) {
    machine.system().sound().KoePlay(koe);
  }
};

// We ignore fadein because we'll never get that effect with the
// current mixing library.
struct koeSetVolume_0 : public RLOpcode<IntConstant_T> {
  void operator()(RLMachine& machine, int level) {
    machine.system().sound().SetKoeVolume(level, 0);
  }
};

struct koeUnMute_1 : public RLOpcode<IntConstant_T> {
  void operator()(RLMachine& machine, int fadein) {
    machine.system().sound().SetKoeVolume(255, fadein);
  }
};

struct koeMute_1 : public RLOpcode<IntConstant_T> {
  void operator()(RLMachine& machine, int fadein) {
    machine.system().sound().SetKoeVolume(0, fadein);
  }
};

}  // namespace

// -----------------------------------------------------------------------

KoeModule::KoeModule() : RLModule("Koe", 1, 23) {
  AddOpcode(0, 0, "koePlay", new koePlay_0);
  AddOpcode(0, 1, "koePlay", new koePlay_1);

  AddOpcode(1, 0, "koePlayEx", new koePlayEx_0);
  AddOpcode(1, 1, "koePlayEx", new koePlayEx_1);

  AddOpcode(3, 0, "koeWait", new koeWait);
  AddOpcode(4, 0, "koePlaying", ReturnIntValue(&SoundSystem::KoePlaying));
  AddOpcode(5, 0, "koeStop", CallFunction(&SoundSystem::KoeStop));
  AddOpcode(6, 0, "koeWaitC", new koeWaitC);

  AddOpcode(7, 0, "koePlayExC", new koePlayExC_0);
  AddOpcode(7, 1, "koePlayExC", new koePlayExC_1);

  AddOpcode(8, 0, "koeDoPlay", new koePlay_0);
  AddOpcode(8, 1, "koeDoPlay", new koeDoPlay_1);

  AddOpcode(9, 0, "koeDoPlayEx", new koePlayEx_0);
  AddOpcode(9, 1, "koeDoPlayEx", new koeDoPlayEx_1);

  AddOpcode(10, 0, "koeDoPlayExC", new koePlayExC_0);
  AddOpcode(10, 1, "koeDoPlayExC", new koeDoPlayExC_1);

  AddOpcode(11, 0, "GetKoeVolume", ReturnIntValue(&SoundSystem::GetKoeVolume));

  AddOpcode(12, 0, "koeSetVolume", new koeSetVolume_0);
  AddOpcode(12, 1, "koeSetVolume", CallFunction(&SoundSystem::SetKoeVolume));

  AddOpcode(
      13, 0, "koeUnMute", CallFunctionWith(&SoundSystem::SetKoeVolume, 255, 0));
  AddOpcode(13, 1, "koeUnMute", new koeUnMute_1);

  AddOpcode(
      14, 0, "koeMute", CallFunctionWith(&SoundSystem::SetKoeVolume, 0, 0));
  AddOpcode(14, 1, "koeMute", new koeMute_1);
}
