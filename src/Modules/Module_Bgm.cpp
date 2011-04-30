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

#include "Modules/Module_Bgm.hpp"

#include <string>

#include "MachineBase/RLMachine.hpp"
#include "MachineBase/RLOperation.hpp"
#include "MachineBase/LongOperation.hpp"
#include "MachineBase/GeneralOperations.hpp"
#include "MachineBase/RLOperation/DefaultValue.hpp"
#include "MachineBase/RLOperation/RLOp_Store.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/SoundSystem.hpp"

namespace {

struct LongOp_bgmWait : public LongOperation {
  bool operator()(RLMachine& machine) {
    return machine.system().sound().bgmStatus() == 0;
  }
};

struct bgmLoop_0 : public RLOp_Void_1<StrConstant_T> {
  void operator()(RLMachine& machine, string filename) {
    machine.system().sound().bgmPlay(filename, true);
  }
};

struct bgmLoop_1 : public RLOp_Void_2<StrConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, string filename, int fadein) {
    machine.system().sound().bgmPlay(filename, true, fadein);
  }
};

struct bgmLoop_2 : public RLOp_Void_3<StrConstant_T, IntConstant_T,
                                          IntConstant_T> {
  void operator()(RLMachine& machine, string filename, int fadein,
                  int fadeout) {
    machine.system().sound().bgmPlay(filename, true, fadein, fadeout);
  }
};

struct bgmPlay_0 : public RLOp_Void_1<StrConstant_T> {
  void operator()(RLMachine& machine, string filename) {
    machine.system().sound().bgmPlay(filename, false);
  }
};

struct bgmPlay_1 : public RLOp_Void_2<StrConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, string filename, int fadein) {
    machine.system().sound().bgmPlay(filename, false, fadein);
  }
};

struct bgmPlay_2 : public RLOp_Void_3<StrConstant_T, IntConstant_T,
                                          IntConstant_T> {
  void operator()(RLMachine& machine, string filename, int fadein,
                  int fadeout) {
    machine.system().sound().bgmPlay(filename, false, fadein, fadeout);
  }
};

struct bgmWait : public RLOp_Void_Void {
  void operator()(RLMachine& machine) {
    machine.pushLongOperation(new LongOp_bgmWait);
  }
};

struct bgmPlaying : public RLOp_Store_Void {
  int operator()(RLMachine& machine) {
    return machine.system().sound().bgmStatus() == 1;
  }
};

struct bgmSetVolume_0 : public RLOp_Void_1<IntConstant_T> {
  void operator()(RLMachine& machine, int vol) {
    machine.system().sound().setBgmVolumeScript(vol, 0);
  }
};

struct bgmFadeOutEx : public RLOp_Void_1<DefaultIntValue_T<1000> > {
  void operator()(RLMachine& machine, int fadeout) {
    machine.system().sound().bgmFadeOut(fadeout);
    machine.pushLongOperation(new LongOp_bgmWait);
  }
};

struct bgmUnMute_1 : public RLOp_Void_1<IntConstant_T> {
  void operator()(RLMachine& machine, int fadein) {
    machine.system().sound().setBgmVolumeScript(255, fadein);
  }
};

struct bgmMute_1 : public RLOp_Void_1<IntConstant_T> {
  void operator()(RLMachine& machine, int fadein) {
    machine.system().sound().setBgmVolumeScript(0, fadein);
  }
};

}  // namespace

BgmModule::BgmModule()
  : RLModule("Bgm", 1, 20) {
  // fun \\([A-Za-z0-9]+\\) +<1:Bgm:\\([0-9]+\\), \\([0-9]+\\).*$
  addOpcode(0, 0, "bgmLoop", new bgmLoop_0);
  addOpcode(0, 1, "bgmLoop", new bgmLoop_1);
  addOpcode(0, 2, "bgmLoop", new bgmLoop_2);

  addUnsupportedOpcode(1, 0, "bgmPlayEx");
  addUnsupportedOpcode(1, 1, "bgmPlayEx");
  addUnsupportedOpcode(1, 2, "bgmPlayEx");

  addOpcode(2, 0, "bgmPlay", new bgmPlay_0);
  addOpcode(2, 1, "bgmPlay", new bgmPlay_1);
  addOpcode(2, 2, "bgmPlay", new bgmPlay_2);

  addOpcode(3, 0, "bgmWait", new bgmWait);
  addOpcode(4, 0, "bgmPlaying", new bgmPlaying);
  addOpcode(5, 0, "bgmStop", callFunction(&SoundSystem::bgmStop));
  addOpcode(6, 0, "bgmStop2", callFunction(&SoundSystem::bgmStop));
  addUnsupportedOpcode(6, 0, "bgmStop2");
  addOpcode(7, 0, "bgmStatus", returnIntValue(&SoundSystem::bgmStatus));
  addUnsupportedOpcode(8, 0, "bgmRewind");
  addOpcode(9, 0, "bgmPause", callFunction(&SoundSystem::bgmPause));
  addOpcode(10, 0, "bgmUnPause", callFunction(&SoundSystem::bgmUnPause));
  addOpcode(11, 0, "bgmVolume", returnIntValue(&SoundSystem::bgmVolumeScript));

  addOpcode(12, 0, "bgmSetVolume", new bgmSetVolume_0);
  addOpcode(12, 1, "bgmSetVolume",
            callFunction(&SoundSystem::setBgmVolumeScript));
  addOpcode(13, 0, "bgmUnMute",
            callFunctionWith(&SoundSystem::setBgmVolumeScript, 255, 0));
  addOpcode(13, 1, "bgmUnMute", new bgmUnMute_1);
  addOpcode(14, 0, "bgmMute",
            callFunctionWith(&SoundSystem::setBgmVolumeScript, 0, 0));
  addOpcode(14, 1, "bgmMute", new bgmMute_1);

  addOpcode(105, 0, "bgmFadeOut", callFunction(&SoundSystem::bgmFadeOut));

  addOpcode(106, 0, "bgmFadeOutEx", new bgmFadeOutEx);
  addOpcode(106, 1, "bgmFadeOutEx", new bgmFadeOutEx);

  addUnsupportedOpcode(107, 0, "bgmStatus2");
  addUnsupportedOpcode(200, 0, "bgmTimer");
}
