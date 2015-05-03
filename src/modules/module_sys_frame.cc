// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006, 2007 Elliot Glaysher
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

#include "modules/module_sys_frame.h"

#include <vector>

#include "machine/general_operations.h"
#include "machine/rlmachine.h"
#include "machine/rlmodule.h"
#include "machine/rloperation.h"
#include "machine/rloperation/argc_t.h"
#include "machine/rloperation/complex_t.h"
#include "modules/module_sys.h"
#include "systems/base/event_system.h"
#include "systems/base/frame_counter.h"
#include "systems/base/system.h"

using std::get;

namespace {

template <typename FRAMECLASS>
struct InitFrame : public RLOpcode<IntConstant_T,
                                      IntConstant_T,
                                      IntConstant_T,
                                      IntConstant_T> {
  const int layer_;
  explicit InitFrame(int layer) : layer_(layer) {}

  void operator()(RLMachine& machine,
                  int counter,
                  int frameMin,
                  int frameMax,
                  int time) {
    EventSystem& es = machine.system().event();
    es.SetFrameCounter(
        layer_, counter, new FRAMECLASS(es, frameMin, frameMax, time));
  }
};

struct ReadFrame : public RLStoreOpcode<IntConstant_T> {
  const int layer_;
  explicit ReadFrame(int layer) : layer_(layer) {}

  int operator()(RLMachine& machine, int counter) {
    EventSystem& es = machine.system().event();
    if (es.FrameCounterExists(layer_, counter))
      return es.GetFrameCounter(layer_, counter).ReadFrame();
    else
      return 0;
  }
};

struct FrameActive : public RLStoreOpcode<IntConstant_T> {
  const int layer_;
  explicit FrameActive(int layer) : layer_(layer) {}

  int operator()(RLMachine& machine, int counter) {
    EventSystem& es = machine.system().event();
    if (es.FrameCounterExists(layer_, counter)) {
      return es.GetFrameCounter(layer_, counter).IsActive();
    } else {
      return 0;
    }
  }
};

struct AnyFrameActive : public RLStoreOpcode<IntConstant_T> {
  const int layer_;
  explicit AnyFrameActive(int layer) : layer_(layer) {}

  int operator()(RLMachine& machine, int counter) {
    EventSystem& es = machine.system().event();
    for (int i = 0; i < 255; ++i) {
      if (es.FrameCounterExists(layer_, counter) &&
          es.GetFrameCounter(layer_, counter).IsActive()) {
        return 1;
      }
    }

    return 0;
  }
};

struct ClearFrame_0 : public RLOpcode<IntConstant_T> {
  const int layer_;
  explicit ClearFrame_0(int layer) : layer_(layer) {}

  void operator()(RLMachine& machine, int counter) {
    EventSystem& es = machine.system().event();
    es.SetFrameCounter(layer_, counter, NULL);
  }
};

struct ClearFrame_1 : public RLOpcode<IntConstant_T, IntConstant_T> {
  const int layer_;
  explicit ClearFrame_1(int layer) : layer_(layer) {}

  void operator()(RLMachine& machine, int counter, int new_value) {
    FrameCounter& fc =
        machine.system().event().GetFrameCounter(layer_, counter);
    fc.set_active(false);
    fc.set_value(new_value);
  }
};

struct ClearAllFrames_0 : public RLOpcode<IntConstant_T> {
  const int layer_;
  explicit ClearAllFrames_0(int layer) : layer_(layer) {}

  void operator()(RLMachine& machine, int new_value) {
    EventSystem& es = machine.system().event();

    for (int i = 0; i < 255; ++i) {
      if (es.FrameCounterExists(layer_, i)) {
        FrameCounter& fc = es.GetFrameCounter(layer_, i);
        fc.set_active(false);
        fc.set_value(new_value);
      }
    }
  }
};

struct ClearAllFrames_1 : public RLOpcode<> {
  const int layer_;
  explicit ClearAllFrames_1(int layer) : layer_(layer) {}

  void operator()(RLMachine& machine) {
    EventSystem& es = machine.system().event();

    for (int i = 0; i < 255; ++i) {
      if (es.FrameCounterExists(layer_, i)) {
        es.SetFrameCounter(layer_, i, NULL);
      }
    }
  }
};

typedef Complex_T<IntConstant_T, IntReference_T> FrameDataInReadFrames;

struct ReadFrames : public RLStoreOpcode<Argc_T<FrameDataInReadFrames>> {
  const int layer_;
  explicit ReadFrames(int layer) : layer_(layer) {}

  int operator()(RLMachine& machine,
                 std::vector<FrameDataInReadFrames::type> frames) {
    EventSystem& es = machine.system().event();

    bool storeValue = false;

    for (auto& frame : frames) {
      int counter = get<0>(frame);

      if (es.FrameCounterExists(layer_, counter)) {
        int val = es.GetFrameCounter(layer_, counter).ReadFrame();
        *(get<1>(frame)) = val;

        if (es.GetFrameCounter(layer_, counter).IsActive())
          storeValue = true;
      } else {
        *(get<1>(frame)) = 0;
      }
    }

    return storeValue;
  }
};

}  // namespace

// -----------------------------------------------------------------------

void AddSysFrameOpcodes(RLModule& m) {
  // Normal frame counter operations
  m.AddOpcode(500, 0, "InitFrame", new InitFrame<SimpleFrameCounter>(0));
  m.AddOpcode(501, 0, "InitFrameLoop", new InitFrame<LoopFrameCounter>(0));
  m.AddOpcode(502, 0, "InitFrameTurn", new InitFrame<TurnFrameCounter>(0));
  m.AddOpcode(
      503, 0, "InitFrameAccel", new InitFrame<AcceleratingFrameCounter>(0));
  m.AddOpcode(
      504, 0, "InitFrameDecel", new InitFrame<DeceleratingFrameCounter>(0));
  m.AddOpcode(510, 0, "ReadFrame", new ReadFrame(0));
  m.AddOpcode(511, 0, "FrameActive", new FrameActive(0));
  m.AddOpcode(512, 0, "AnyFrameActive", new AnyFrameActive(0));
  m.AddOpcode(513, 0, "ClearFrame", new ClearFrame_0(0));
  m.AddOpcode(513, 1, "ClearFrame", new ClearFrame_1(0));
  m.AddOpcode(514, 0, "ClearAllFrames", new ClearAllFrames_0(0));
  m.AddOpcode(514, 1, "ClearAllFrames", new ClearAllFrames_1(0));

  // Extended frame counter operations
  m.AddOpcode(520, 0, "InitExFrame", new InitFrame<SimpleFrameCounter>(1));
  m.AddOpcode(521, 0, "InitExFrameLoop", new InitFrame<LoopFrameCounter>(1));
  m.AddOpcode(522, 0, "InitExFrameTurn", new InitFrame<TurnFrameCounter>(1));
  m.AddOpcode(
      523, 0, "InitExFrameAccel", new InitFrame<AcceleratingFrameCounter>(1));
  m.AddOpcode(
      524, 0, "InitExFrameDecel", new InitFrame<DeceleratingFrameCounter>(1));
  m.AddOpcode(530, 0, "ReadExFrame", new ReadFrame(1));
  m.AddOpcode(531, 0, "ExFrameActive", new FrameActive(1));
  m.AddOpcode(532, 0, "AnyExFrameActive", new AnyFrameActive(1));
  m.AddOpcode(533, 0, "ClearExFrame", new ClearFrame_0(1));
  m.AddOpcode(533, 1, "ClearExFrame", new ClearFrame_1(1));
  m.AddOpcode(534, 0, "ClearAllExFrames", new ClearAllFrames_0(1));
  m.AddOpcode(534, 1, "ClearAllExFrames", new ClearAllFrames_1(1));

  // Multiple Dispatch operations on normal frame counters
  m.AddOpcode(600,
              0,
              "InitFrames",
              new MultiDispatch(new InitFrame<SimpleFrameCounter>(0)));
  m.AddOpcode(601,
              0,
              "InitFramesLoop",
              new MultiDispatch(new InitFrame<LoopFrameCounter>(0)));
  m.AddOpcode(602,
              0,
              "InitFramesTurn",
              new MultiDispatch(new InitFrame<TurnFrameCounter>(0)));
  m.AddOpcode(603,
              0,
              "InitFramesAccel",
              new MultiDispatch(new InitFrame<AcceleratingFrameCounter>(0)));
  m.AddOpcode(604,
              0,
              "InitFramesDecel",
              new MultiDispatch(new InitFrame<DeceleratingFrameCounter>(0)));
  m.AddOpcode(610, 0, "ReadFrames", new ReadFrames(0));

  // Multiple Dispatch operations on normal frame counters
  m.AddOpcode(620,
              0,
              "InitExFrames",
              new MultiDispatch(new InitFrame<SimpleFrameCounter>(1)));
  m.AddOpcode(621,
              0,
              "InitExFramesLoop",
              new MultiDispatch(new InitFrame<LoopFrameCounter>(1)));
  m.AddOpcode(622,
              0,
              "InitExFramesTurn",
              new MultiDispatch(new InitFrame<TurnFrameCounter>(1)));
  m.AddOpcode(623,
              0,
              "InitExFramesAccel",
              new MultiDispatch(new InitFrame<AcceleratingFrameCounter>(1)));
  m.AddOpcode(624,
              0,
              "InitExFramesDecel",
              new MultiDispatch(new InitFrame<DeceleratingFrameCounter>(1)));
  m.AddOpcode(630, 0, "ReadExFrames", new ReadFrames(1));
}
