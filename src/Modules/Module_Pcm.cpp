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

#include "Modules/Module_Pcm.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/SoundSystem.hpp"
#include "MachineBase/LongOperation.hpp"
#include "MachineBase/GeneralOperations.hpp"

// -----------------------------------------------------------------------

struct LongOp_pcmWait : public LongOperation
{
  int channel_;
  LongOp_pcmWait(int channel) : channel_(channel) {
    // TODO: Make it reset the number of loops, or this could
    // theoretically go on forever.
  }

  bool operator()(RLMachine& machine) {
    return !machine.system().sound().wavPlaying(machine, channel_);
  }
};

// -----------------------------------------------------------------------

struct Pcm_wavPlay_0 : public RLOp_Void_1<StrConstant_T> {
  void operator()(RLMachine& machine, std::string fileName) {
    machine.system().sound().wavPlay(machine, fileName, false);
  }
};

// -----------------------------------------------------------------------

struct Pcm_wavPlay_1 : public RLOp_Void_2<StrConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, std::string fileName, int channel) {
    machine.system().sound().wavPlay(machine, fileName, false, channel);
  }
};

// -----------------------------------------------------------------------

struct Pcm_wavPlay_2 : public RLOp_Void_3<StrConstant_T, IntConstant_T,
                                          IntConstant_T> {
  void operator()(RLMachine& machine, std::string fileName, int channel,
                  int fadein) {
    machine.system().sound().wavPlay(machine, fileName, false, channel, fadein);
  }
};

// -----------------------------------------------------------------------

struct Pcm_wavPlayEx_0 : public RLOp_Void_2<StrConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, std::string fileName, int channel) {
    machine.system().sound().wavPlay(machine, fileName, false, channel);
    machine.pushLongOperation(new LongOp_pcmWait(channel));
  }
};

// -----------------------------------------------------------------------

struct Pcm_wavPlayEx_1 : public RLOp_Void_3<StrConstant_T, IntConstant_T,
                                            IntConstant_T> {
  void operator()(RLMachine& machine, std::string fileName, int channel,
                  int fadein) {
    machine.system().sound().wavPlay(machine, fileName, false, channel, fadein);
    machine.pushLongOperation(new LongOp_pcmWait(channel));
  }
};

// -----------------------------------------------------------------------

struct Pcm_wavLoop_0 : public RLOp_Void_2<StrConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, std::string fileName, int channel) {
    machine.system().sound().wavPlay(machine, fileName, true, channel);
  }
};

// -----------------------------------------------------------------------

struct Pcm_wavLoop_1 : public RLOp_Void_3<StrConstant_T, IntConstant_T,
                                          IntConstant_T> {
  void operator()(RLMachine& machine, std::string fileName, int channel,
                  int fadein) {
    machine.system().sound().wavPlay(machine, fileName, true, channel, fadein);
  }
};

// -----------------------------------------------------------------------

struct Pcm_wavWait : public RLOp_Void_1<IntConstant_T> {
  void operator()(RLMachine& machine, int channel) {
    // Start waiting on the channel
    machine.pushLongOperation(new LongOp_pcmWait(channel));
  }
};

// -----------------------------------------------------------------------

struct Pcm_wavPlaying : public RLOp_Store_1<IntConstant_T> {
  int operator()(RLMachine& machine, int channel) {
    return machine.system().sound().wavPlaying(machine, channel);
  }
};

// -----------------------------------------------------------------------

struct Pcm_wavVolume : public RLOp_Store_1<IntConstant_T> {
  int operator()(RLMachine& machine, int channel) {
    return machine.system().sound().channelVolume(channel);
  }
};

// -----------------------------------------------------------------------

struct Pcm_wavSetVolume_0 : public RLOp_Void_2<IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, int channel, int level) {
    machine.system().sound().setChannelVolume(channel, level);
  }
};

// -----------------------------------------------------------------------

/// We ignore fadein because we'll never get that effect with the
/// current mixing library.
struct Pcm_wavSetVolume_1 : public RLOp_Void_3<IntConstant_T, IntConstant_T,
                                               IntConstant_T> {
  void operator()(RLMachine& machine, int channel, int level, int fadeInMs) {
    machine.system().sound().setChannelVolume(machine, channel, level, fadeInMs);
  }
};

// -----------------------------------------------------------------------

struct Pcm_wavUnMute_0 : public RLOp_Void_1<IntConstant_T> {
  void operator()(RLMachine& machine, int channel) {
    machine.system().sound().setChannelVolume(channel, 255);
  }
};

// -----------------------------------------------------------------------

struct Pcm_wavUnMute_1 : public RLOp_Void_2<IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, int channel, int fadein) {
    machine.system().sound().setChannelVolume(machine, channel, 255, fadein);
  }
};

// -----------------------------------------------------------------------

struct Pcm_wavMute_0 : public RLOp_Void_1<IntConstant_T> {
  void operator()(RLMachine& machine, int channel) {
    machine.system().sound().setChannelVolume(channel, 0);
  }
};

// -----------------------------------------------------------------------

struct Pcm_wavMute_1 : public RLOp_Void_2<IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, int channel, int fadein) {
    machine.system().sound().setChannelVolume(machine, channel, 0, fadein);
  }
};

// -----------------------------------------------------------------------

PcmModule::PcmModule()
  : RLModule("Pcm", 1, 21)
{
  addOpcode(0, 0, "wavPlay", new Pcm_wavPlay_0);
  addOpcode(0, 1, "wavPlay", new Pcm_wavPlay_1);
  addOpcode(0, 2, "wavPlay", new Pcm_wavPlay_2);

  addOpcode(1, 0, "wavPlayEx", new Pcm_wavPlayEx_0);
  addOpcode(1, 1, "wavPlayEx", new Pcm_wavPlayEx_1);

  addOpcode(2, 0, "wavLoop", new Pcm_wavLoop_0);
  addOpcode(2, 1, "wavLoop", new Pcm_wavLoop_1);

  addOpcode(3, 0, "wavWait", new Pcm_wavWait);
  addOpcode(4, 0, "wavPlaying", new Pcm_wavPlaying);

  addOpcode(5, 0, "wavStop", callFunction(&SoundSystem::wavStop));
  addOpcode(5, 1, "wavStop", callFunction(&SoundSystem::wavStopAll));

  addUnsupportedOpcode(7, 0, "wavPlaying2");
  addUnsupportedOpcode(8, 0, "wavRewind");
  addOpcode(9, 0, "wavStop3", callFunction(&SoundSystem::wavStop));
  addOpcode(10, 0, "wavStop4", callFunction(&SoundSystem::wavStop));
  addOpcode(11, 0, "wavVolume", new Pcm_wavVolume);

  addOpcode(12, 0, "wavSetVolume", new Pcm_wavSetVolume_0);
  addOpcode(12, 1, "wavSetVolume", new Pcm_wavSetVolume_1);

  addOpcode(13, 0, "wavUnMute", new Pcm_wavUnMute_0);
  addOpcode(13, 1, "wavUnMute", new Pcm_wavUnMute_1);

  addOpcode(14, 0, "wavMute", new Pcm_wavMute_0);
  addOpcode(14, 1, "wavMute", new Pcm_wavMute_0);

  addOpcode(20, 0, "wavStopAll", callFunction(&SoundSystem::wavStopAll));

  addOpcode(105, 0, "wavFadeOut", callFunction(&SoundSystem::wavFadeOut));
  addUnsupportedOpcode(106, 0, "wavFadeOut2");
  addOpcode(106, 1, "wavFadeOut2", callFunction(&SoundSystem::wavFadeOut));

  // Unknown/Undocumented function in this module
//  fun <1:Pcm:00040, 0> (<intC, (strC, intC, intC)+)
}
