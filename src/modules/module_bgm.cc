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

#include "modules/module_bgm.h"

#include <functional>
#include <string>

#include "long_operations/wait_long_operation.h"
#include "machine/rlmachine.h"
#include "machine/rloperation.h"
#include "machine/long_operation.h"
#include "machine/general_operations.h"
#include "machine/rloperation/default_value.h"
#include "machine/rloperation/rlop_store.h"
#include "systems/base/system.h"
#include "systems/base/sound_system.h"

namespace {

bool BgmWait(RLMachine& machine) {
  return machine.system().sound().BgmStatus() == 0;
}

LongOperation* MakeBgmWait(RLMachine& machine) {
  WaitLongOperation* wait_op = new WaitLongOperation(machine);
  wait_op->BreakOnEvent(std::bind(BgmWait, std::ref(machine)));
  return wait_op;
}

struct LongOp_bgmWait : public LongOperation {
  bool operator()(RLMachine& machine) {
    return machine.system().sound().BgmStatus() == 0;
  }
};

struct bgmLoop_0 : public RLOpcode<StrConstant_T> {
  void operator()(RLMachine& machine, string filename) {
    machine.system().sound().BgmPlay(filename, true);
  }
};

struct bgmLoop_1 : public RLOpcode<StrConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, string filename, int fadein) {
    machine.system().sound().BgmPlay(filename, true, fadein);
  }
};

struct bgmLoop_2
    : public RLOpcode<StrConstant_T, IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine,
                  string filename,
                  int fadein,
                  int fadeout) {
    machine.system().sound().BgmPlay(filename, true, fadein, fadeout);
  }
};

struct bgmPlay_0 : public RLOpcode<StrConstant_T> {
  void operator()(RLMachine& machine, string filename) {
    machine.system().sound().BgmPlay(filename, false);
  }
};

struct bgmPlay_1 : public RLOpcode<StrConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, string filename, int fadein) {
    machine.system().sound().BgmPlay(filename, false, fadein);
  }
};

struct bgmPlay_2
    : public RLOpcode<StrConstant_T, IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine,
                  string filename,
                  int fadein,
                  int fadeout) {
    machine.system().sound().BgmPlay(filename, false, fadein, fadeout);
  }
};

struct bgmWait : public RLOpcode<> {
  void operator()(RLMachine& machine) {
    machine.PushLongOperation(MakeBgmWait(machine));
  }
};

struct bgmPlaying : public RLStoreOpcode<> {
  int operator()(RLMachine& machine) {
    return machine.system().sound().BgmStatus() == 1;
  }
};

struct bgmSetVolume_0 : public RLOpcode<IntConstant_T> {
  void operator()(RLMachine& machine, int vol) {
    machine.system().sound().SetBgmVolumeScript(vol, 0);
  }
};

struct bgmFadeOutEx : public RLOpcode<DefaultIntValue_T<1000>> {
  void operator()(RLMachine& machine, int fadeout) {
    machine.system().sound().BgmFadeOut(fadeout);
    machine.PushLongOperation(MakeBgmWait(machine));
  }
};

struct bgmUnMute_1 : public RLOpcode<IntConstant_T> {
  void operator()(RLMachine& machine, int fadein) {
    machine.system().sound().SetBgmVolumeScript(255, fadein);
  }
};

struct bgmMute_1 : public RLOpcode<IntConstant_T> {
  void operator()(RLMachine& machine, int fadein) {
    machine.system().sound().SetBgmVolumeScript(0, fadein);
  }
};

}  // namespace

BgmModule::BgmModule() : RLModule("Bgm", 1, 20) {
  // fun \\([A-Za-z0-9]+\\) +<1:Bgm:\\([0-9]+\\), \\([0-9]+\\).*$
  AddOpcode(0, 0, "bgmLoop", new bgmLoop_0);
  AddOpcode(0, 1, "bgmLoop", new bgmLoop_1);
  AddOpcode(0, 2, "bgmLoop", new bgmLoop_2);

  AddUnsupportedOpcode(1, 0, "bgmPlayEx");
  AddUnsupportedOpcode(1, 1, "bgmPlayEx");
  AddUnsupportedOpcode(1, 2, "bgmPlayEx");

  AddOpcode(2, 0, "bgmPlay", new bgmPlay_0);
  AddOpcode(2, 1, "bgmPlay", new bgmPlay_1);
  AddOpcode(2, 2, "bgmPlay", new bgmPlay_2);

  AddOpcode(3, 0, "bgmWait", new bgmWait);
  AddOpcode(4, 0, "bgmPlaying", new bgmPlaying);
  AddOpcode(5, 0, "bgmStop", CallFunction(&SoundSystem::BgmStop));
  AddOpcode(6, 0, "bgmStop2", CallFunction(&SoundSystem::BgmStop));
  AddOpcode(7, 0, "bgmStatus", ReturnIntValue(&SoundSystem::BgmStatus));
  AddUnsupportedOpcode(8, 0, "bgmRewind");
  AddOpcode(9, 0, "bgmPause", CallFunction(&SoundSystem::BgmPause));
  AddOpcode(10, 0, "bgmUnPause", CallFunction(&SoundSystem::BgmUnPause));
  AddOpcode(11, 0, "bgmVolume", ReturnIntValue(&SoundSystem::bgm_volume_script));

  AddOpcode(12, 0, "bgmSetVolume", new bgmSetVolume_0);
  AddOpcode(
      12, 1, "bgmSetVolume", CallFunction(&SoundSystem::SetBgmVolumeScript));
  AddOpcode(13,
            0,
            "bgmUnMute",
            CallFunctionWith(&SoundSystem::SetBgmVolumeScript, 255, 0));
  AddOpcode(13, 1, "bgmUnMute", new bgmUnMute_1);
  AddOpcode(14,
            0,
            "bgmMute",
            CallFunctionWith(&SoundSystem::SetBgmVolumeScript, 0, 0));
  AddOpcode(14, 1, "bgmMute", new bgmMute_1);

  AddOpcode(105, 0, "bgmFadeOut", CallFunction(&SoundSystem::BgmFadeOut));

  AddOpcode(106, 0, "bgmFadeOutEx", new bgmFadeOutEx);
  AddOpcode(106, 1, "bgmFadeOutEx", new bgmFadeOutEx);

  AddUnsupportedOpcode(107, 0, "bgmStatus2");
  AddUnsupportedOpcode(200, 0, "bgmTimer");
}
