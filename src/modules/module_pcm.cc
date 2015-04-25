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

#include "modules/module_pcm.h"

#include <functional>
#include <string>

#include "long_operations/wait_long_operation.h"
#include "machine/general_operations.h"
#include "machine/long_operation.h"
#include "systems/base/sound_system.h"
#include "systems/base/system.h"

namespace {

bool NoLongerPlaying(RLMachine& machine, int channel) {
  return !machine.system().sound().WavPlaying(channel);
}

void addPcmWait(RLMachine& machine, int channel) {
  WaitLongOperation* wait_op = new WaitLongOperation(machine);
  wait_op->BreakOnEvent(std::bind(NoLongerPlaying, std::ref(machine), channel));
  machine.PushLongOperation(wait_op);
}

struct wavPlay_0 : public RLOpcode<StrConstant_T> {
  void operator()(RLMachine& machine, std::string fileName) {
    machine.system().sound().WavPlay(fileName, false);
  }
};

struct wavPlay_1 : public RLOpcode<StrConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, std::string fileName, int channel) {
    machine.system().sound().WavPlay(fileName, false, channel);
  }
};

struct wavPlay_2
    : public RLOpcode<StrConstant_T, IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine,
                  std::string fileName,
                  int channel,
                  int fadein) {
    machine.system().sound().WavPlay(fileName, false, channel, fadein);
  }
};

struct wavPlayEx_0 : public RLOpcode<StrConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, std::string fileName, int channel) {
    machine.system().sound().WavPlay(fileName, false, channel);
    addPcmWait(machine, channel);
  }
};

struct wavPlayEx_1
    : public RLOpcode<StrConstant_T, IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine,
                  std::string fileName,
                  int channel,
                  int fadein) {
    machine.system().sound().WavPlay(fileName, false, channel, fadein);
    addPcmWait(machine, channel);
  }
};

struct wavLoop_0 : public RLOpcode<StrConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, std::string fileName, int channel) {
    machine.system().sound().WavPlay(fileName, true, channel);
  }
};

struct wavLoop_1
    : public RLOpcode<StrConstant_T, IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine,
                  std::string fileName,
                  int channel,
                  int fadein) {
    machine.system().sound().WavPlay(fileName, true, channel, fadein);
  }
};

struct wavWait : public RLOpcode<IntConstant_T> {
  void operator()(RLMachine& machine, int channel) {
    // Start waiting on the channel
    addPcmWait(machine, channel);
  }
};

struct wavPlaying : public RLStoreOpcode<IntConstant_T> {
  int operator()(RLMachine& machine, int channel) {
    return machine.system().sound().WavPlaying(channel);
  }
};

struct wavVolume : public RLStoreOpcode<IntConstant_T> {
  int operator()(RLMachine& machine, int channel) {
    return machine.system().sound().GetChannelVolume(channel);
  }
};

struct wavSetVolume_0 : public RLOpcode<IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, int channel, int level) {
    machine.system().sound().SetChannelVolume(channel, level);
  }
};

// We ignore fadein because we'll never get that effect with the
// current mixing library.
struct wavSetVolume_1
    : public RLOpcode<IntConstant_T, IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, int channel, int level, int fadeInMs) {
    machine.system().sound().SetChannelVolume(channel, level, fadeInMs);
  }
};

struct wavUnMute_0 : public RLOpcode<IntConstant_T> {
  void operator()(RLMachine& machine, int channel) {
    machine.system().sound().SetChannelVolume(channel, 255);
  }
};

struct wavUnMute_1 : public RLOpcode<IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, int channel, int fadein) {
    machine.system().sound().SetChannelVolume(channel, 255, fadein);
  }
};

struct wavMute_0 : public RLOpcode<IntConstant_T> {
  void operator()(RLMachine& machine, int channel) {
    machine.system().sound().SetChannelVolume(channel, 0);
  }
};

struct wavMute_1 : public RLOpcode<IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, int channel, int fadein) {
    machine.system().sound().SetChannelVolume(channel, 0, fadein);
  }
};

}  // namespace

// -----------------------------------------------------------------------

PcmModule::PcmModule() : RLModule("Pcm", 1, 21) {
  AddOpcode(0, 0, "wavPlay", new wavPlay_0);
  AddOpcode(0, 1, "wavPlay", new wavPlay_1);
  AddOpcode(0, 2, "wavPlay", new wavPlay_2);

  AddOpcode(1, 0, "wavPlayEx", new wavPlayEx_0);
  AddOpcode(1, 1, "wavPlayEx", new wavPlayEx_1);

  AddOpcode(2, 0, "wavLoop", new wavLoop_0);
  AddOpcode(2, 1, "wavLoop", new wavLoop_1);

  AddOpcode(3, 0, "wavWait", new wavWait);
  AddOpcode(4, 0, "wavPlaying", new wavPlaying);

  AddOpcode(5, 0, "wavStop", CallFunction(&SoundSystem::WavStop));
  AddOpcode(5, 1, "wavStop", CallFunction(&SoundSystem::WavStopAll));

  AddUnsupportedOpcode(7, 0, "wavPlaying2");
  AddUnsupportedOpcode(8, 0, "wavRewind");
  AddOpcode(9, 0, "wavStop3", CallFunction(&SoundSystem::WavStop));
  AddOpcode(10, 0, "wavStop4", CallFunction(&SoundSystem::WavStop));
  AddOpcode(11, 0, "wavVolume", new wavVolume);

  AddOpcode(12, 0, "wavSetVolume", new wavSetVolume_0);
  AddOpcode(12, 1, "wavSetVolume", new wavSetVolume_1);

  AddOpcode(13, 0, "wavUnMute", new wavUnMute_0);
  AddOpcode(13, 1, "wavUnMute", new wavUnMute_1);

  AddOpcode(14, 0, "wavMute", new wavMute_0);
  AddOpcode(14, 1, "wavMute", new wavMute_1);

  AddOpcode(20, 0, "wavStopAll", CallFunction(&SoundSystem::WavStopAll));

  AddOpcode(105, 0, "wavFadeOut", CallFunction(&SoundSystem::WavFadeOut));
  AddUnsupportedOpcode(106, 0, "wavFadeOut2");
  AddOpcode(106, 1, "wavFadeOut2", CallFunction(&SoundSystem::WavFadeOut));

  // Unknown/Undocumented function in this module
  //  fun <1:Pcm:00040, 0> (<intC, (strC, intC, intC)+)
}
