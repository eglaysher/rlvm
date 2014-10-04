// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006 Elliot Glaysher
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

#include "modules/module_event_loop.h"

#include "long_operations/pause_long_operation.h"
#include "machine/general_operations.h"
#include "machine/rlmachine.h"
#include "systems/base/graphics_system.h"
#include "systems/base/sound_system.h"
#include "systems/base/system.h"
#include "systems/base/text_system.h"
#include "systems/base/text_window.h"

namespace {

struct setOverride : public RLOpcode<IntConstant_T> {
  bool value_;
  explicit setOverride(int value) : value_(value) {}

  void operator()(RLMachine& machine, int window) {
    machine.system().text().SetVisualOverride(window, value_);
  }
};

// The pause() replacement in RealLiveMax. I don't know how this differs, so
// I'm just copying this for now.
//
// This might be more like page, par or spause? Test on Planetarian.
struct rlm_pause : public RLOpcode<> {
  void operator()(RLMachine& machine) {
    TextSystem& text = machine.system().text();
    int windowNum = text.active_window();
    std::shared_ptr<TextWindow> textWindow = text.GetTextWindow(windowNum);

    if (textWindow->action_on_pause()) {
      machine.PushLongOperation(
          new NewParagraphAfterLongop(new PauseLongOperation(machine)));
    } else {
      machine.PushLongOperation(
          new NewPageOnAllAfterLongop(new PauseLongOperation(machine)));
    }
  }
};

// The page() replacement in RealLiveMax. If it has different semantics, they
// don't show up in planetarian.
struct rlm_page : public RLOpcode<> {
  void operator()(RLMachine& machine) {
    machine.PushLongOperation(
        new NewPageAfterLongop(new PauseLongOperation(machine)));
  }
};

struct rlm_bgm_loop_0 : public RLOpcode<StrConstant_T> {
  void operator()(RLMachine& machine, string filename) {
    machine.system().sound().BgmPlay(filename, true);
  }
};

struct rlm_bgm_loop_1 : public RLOpcode<StrConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, string filename, int fadein) {
    machine.system().sound().BgmPlay(filename, true, fadein);
  }
};

struct rlm_bgm_loop_2
    : public RLOpcode<StrConstant_T, IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine,
                  string filename,
                  int fadein,
                  int fadeout) {
    machine.system().sound().BgmPlay(filename, true, fadein, fadeout);
  }
};

struct rlm_wav_play_0 : public RLOpcode<StrConstant_T> {
  void operator()(RLMachine& machine, std::string fileName) {
    machine.system().sound().WavPlay(fileName, false);
  }
};

struct rlm_wav_play_1 : public RLOpcode<StrConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, std::string fileName, int channel) {
    machine.system().sound().WavPlay(fileName, false, channel);
  }
};

struct rlm_wav_play_2
    : public RLOpcode<StrConstant_T, IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine,
                  std::string fileName,
                  int channel,
                  int fadein) {
    machine.system().sound().WavPlay(fileName, false, channel, fadein);
  }
};

}  // namespace

EventLoopModule::EventLoopModule() : RLModule("EventLoop", 0, 4) {
  AddOpcode(48, 0, "rlm_pause", new rlm_pause);

  AddUnsupportedOpcode(120, 0, "SetInterrupt");
  AddUnsupportedOpcode(121, 0, "ClearInterrupt");

  AddOpcode(163, 0, "rlm_page", new rlm_page);

  AddOpcode(289, 0, "rlm_bgm_loop", new rlm_bgm_loop_0);
  AddOpcode(289, 1, "rlm_bgm_loop", new rlm_bgm_loop_1);
  AddOpcode(289, 2, "rlm_bgm_loop", new rlm_bgm_loop_2);

  AddUnsupportedOpcode(303, 0, "yield");

  // Theoretically the same as rtl, but we don't really know.
  AddOpcode(300, 0, "rtlButton", CallFunction(&RLMachine::ReturnFromFarcall));
  AddOpcode(301, 0, "rtlCancel", CallFunction(&RLMachine::ReturnFromFarcall));
  AddOpcode(302, 0, "rtlSystem", CallFunction(&RLMachine::ReturnFromFarcall));

  AddOpcode(371, 0, "rlm_wav_play", new rlm_wav_play_0);
  AddOpcode(371, 1, "rlm_wav_play", new rlm_wav_play_1);
  AddOpcode(371, 2, "rlm_wav_play", new rlm_wav_play_2);

  AddOpcode(1000,
            0,
            "ShowBackground",
            CallFunction(&GraphicsSystem::ToggleInterfaceHidden));
  AddOpcode(
      1100, 0, "SetSkipMode", CallFunctionWith(&TextSystem::SetSkipMode, 1));
  AddOpcode(
      1101, 0, "ClearSkipMode", CallFunctionWith(&TextSystem::SetSkipMode, 0));
  AddOpcode(1102, 0, "SkipMode", ReturnIntValue(&TextSystem::skip_mode));

  // opcode<0:4:1202, 0> and opcode<0:4:1200, 0> are used in the CLANNAD menu
  // system; no idea what they do.
  AddOpcode(1200, 0, "TextwindowOverrideShow", new setOverride(true));
  AddOpcode(1200,
            2,
            "TextwindowOverrideShow",
            CallFunctionWith(&TextSystem::SetVisualOverrideAll, true));
  AddOpcode(1201, 0, "TextwindowOverrideHide", new setOverride(false));
  AddOpcode(1201,
            2,
            "TextwindowOverrideHide",
            CallFunctionWith(&TextSystem::SetVisualOverrideAll, false));
  AddOpcode(1202,
            0,
            "ClearTextwindowOverrides",
            CallFunction(&TextSystem::ClearVisualOverrides));
}
