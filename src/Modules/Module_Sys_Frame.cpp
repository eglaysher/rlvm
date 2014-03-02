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

#include "Modules/Module_Sys_Frame.hpp"
#include "Modules/Module_Sys.hpp"

#include <vector>

#include "MachineBase/GeneralOperations.hpp"
#include "MachineBase/RLMachine.hpp"
#include "MachineBase/RLModule.hpp"
#include "MachineBase/RLOperation.hpp"
#include "MachineBase/RLOperation/Argc_T.hpp"
#include "MachineBase/RLOperation/Complex_T.hpp"
#include "Systems/Base/EventSystem.hpp"
#include "Systems/Base/FrameCounter.hpp"
#include "Systems/Base/System.hpp"

using namespace std;

namespace {

template <typename FRAMECLASS>
struct InitFrame : public RLOp_Void_4<IntConstant_T,
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
    es.setFrameCounter(
        layer_, counter, new FRAMECLASS(es, frameMin, frameMax, time));
  }
};

struct ReadFrame : public RLOp_Store_1<IntConstant_T> {
  const int layer_;
  explicit ReadFrame(int layer) : layer_(layer) {}

  int operator()(RLMachine& machine, int counter) {
    EventSystem& es = machine.system().event();
    if (es.frameCounterExists(layer_, counter))
      return es.getFrameCounter(layer_, counter).readFrame();
    else
      return 0;
  }
};

struct FrameActive : public RLOp_Store_1<IntConstant_T> {
  const int layer_;
  explicit FrameActive(int layer) : layer_(layer) {}

  int operator()(RLMachine& machine, int counter) {
    EventSystem& es = machine.system().event();
    if (es.frameCounterExists(layer_, counter)) {
      return es.getFrameCounter(layer_, counter).isActive();
    } else {
      return 0;
    }
  }
};

struct AnyFrameActive : public RLOp_Store_1<IntConstant_T> {
  const int layer_;
  explicit AnyFrameActive(int layer) : layer_(layer) {}

  int operator()(RLMachine& machine, int counter) {
    EventSystem& es = machine.system().event();
    for (int i = 0; i < 255; ++i) {
      if (es.frameCounterExists(layer_, counter) &&
          es.getFrameCounter(layer_, counter).isActive()) {
        return 1;
      }
    }

    return 0;
  }
};

struct ClearFrame_0 : public RLOp_Void_1<IntConstant_T> {
  const int layer_;
  explicit ClearFrame_0(int layer) : layer_(layer) {}

  void operator()(RLMachine& machine, int counter) {
    EventSystem& es = machine.system().event();
    es.setFrameCounter(layer_, counter, NULL);
  }
};

struct ClearFrame_1 : public RLOp_Void_2<IntConstant_T, IntConstant_T> {
  const int layer_;
  explicit ClearFrame_1(int layer) : layer_(layer) {}

  void operator()(RLMachine& machine, int counter, int newValue) {
    FrameCounter& fc =
        machine.system().event().getFrameCounter(layer_, counter);
    fc.setActive(false);
    fc.setValue(newValue);
  }
};

struct ClearAllFrames_0 : public RLOp_Void_1<IntConstant_T> {
  const int layer_;
  explicit ClearAllFrames_0(int layer) : layer_(layer) {}

  void operator()(RLMachine& machine, int newValue) {
    EventSystem& es = machine.system().event();

    for (int i = 0; i < 255; ++i) {
      if (es.frameCounterExists(layer_, i)) {
        FrameCounter& fc = es.getFrameCounter(layer_, i);
        fc.setActive(false);
        fc.setValue(newValue);
      }
    }
  }
};

struct ClearAllFrames_1 : public RLOp_Void_Void {
  const int layer_;
  explicit ClearAllFrames_1(int layer) : layer_(layer) {}

  void operator()(RLMachine& machine) {
    EventSystem& es = machine.system().event();

    for (int i = 0; i < 255; ++i) {
      if (es.frameCounterExists(layer_, i)) {
        es.setFrameCounter(layer_, i, NULL);
      }
    }
  }
};

typedef Complex2_T<IntConstant_T, IntReference_T> FrameDataInReadFrames;

struct ReadFrames : public RLOp_Store_1<Argc_T<FrameDataInReadFrames>> {
  const int layer_;
  explicit ReadFrames(int layer) : layer_(layer) {}

  int operator()(RLMachine& machine,
                 std::vector<FrameDataInReadFrames::type> frames) {
    static int callNum = 0;
    callNum++;
    EventSystem& es = machine.system().event();

    bool storeValue = false;

    for (auto& frame : frames) {
      int counter = get<0>(frame);

      if (es.frameCounterExists(layer_, counter)) {
        int val = es.getFrameCounter(layer_, counter).readFrame();
        *(get<1>(frame)) = val;

        if (es.getFrameCounter(layer_, counter).isActive())
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

void addSysFrameOpcodes(RLModule& m) {
  // Normal frame counter operations
  m.addOpcode(500, 0, "InitFrame", new InitFrame<SimpleFrameCounter>(0));
  m.addOpcode(501, 0, "InitFrameLoop", new InitFrame<LoopFrameCounter>(0));
  m.addOpcode(502, 0, "InitFrameTurn", new InitFrame<TurnFrameCounter>(0));
  m.addOpcode(
      503, 0, "InitFrameAccel", new InitFrame<AcceleratingFrameCounter>(0));
  m.addOpcode(
      504, 0, "InitFrameDecel", new InitFrame<DeceleratingFrameCounter>(0));
  m.addOpcode(510, 0, "ReadFrame", new ReadFrame(0));
  m.addOpcode(511, 0, "FrameActive", new FrameActive(0));
  m.addOpcode(512, 0, "AnyFrameActive", new AnyFrameActive(0));
  m.addOpcode(513, 0, "ClearFrame", new ClearFrame_0(0));
  m.addOpcode(513, 1, "ClearFrame", new ClearFrame_1(0));
  m.addOpcode(514, 0, "ClearAllFrames", new ClearAllFrames_0(0));
  m.addOpcode(514, 1, "ClearAllFrames", new ClearAllFrames_1(0));

  // Extended frame counter operations
  m.addOpcode(520, 0, "InitExFrame", new InitFrame<SimpleFrameCounter>(1));
  m.addOpcode(521, 0, "InitExFrameLoop", new InitFrame<LoopFrameCounter>(1));
  m.addOpcode(522, 0, "InitExFrameTurn", new InitFrame<TurnFrameCounter>(1));
  m.addOpcode(
      523, 0, "InitExFrameAccel", new InitFrame<AcceleratingFrameCounter>(1));
  m.addOpcode(
      524, 0, "InitExFrameDecel", new InitFrame<DeceleratingFrameCounter>(1));
  m.addOpcode(530, 0, "ReadExFrame", new ReadFrame(1));
  m.addOpcode(531, 0, "ExFrameActive", new FrameActive(1));
  m.addOpcode(532, 0, "AnyExFrameActive", new AnyFrameActive(1));
  m.addOpcode(533, 0, "ClearExFrame", new ClearFrame_0(1));
  m.addOpcode(533, 1, "ClearExFrame", new ClearFrame_1(1));
  m.addOpcode(534, 0, "ClearAllExFrames", new ClearAllFrames_0(1));
  m.addOpcode(534, 1, "ClearAllExFrames", new ClearAllFrames_1(1));

  // Multiple dispatch operations on normal frame counters
  m.addOpcode(600,
              0,
              "InitFrames",
              new MultiDispatch(new InitFrame<SimpleFrameCounter>(0)));
  m.addOpcode(601,
              0,
              "InitFramesLoop",
              new MultiDispatch(new InitFrame<LoopFrameCounter>(0)));
  m.addOpcode(602,
              0,
              "InitFramesTurn",
              new MultiDispatch(new InitFrame<TurnFrameCounter>(0)));
  m.addOpcode(603,
              0,
              "InitFramesAccel",
              new MultiDispatch(new InitFrame<AcceleratingFrameCounter>(0)));
  m.addOpcode(604,
              0,
              "InitFramesDecel",
              new MultiDispatch(new InitFrame<DeceleratingFrameCounter>(0)));
  m.addOpcode(610, 0, "ReadFrames", new ReadFrames(0));

  // Multiple dispatch operations on normal frame counters
  m.addOpcode(620,
              0,
              "InitExFrames",
              new MultiDispatch(new InitFrame<SimpleFrameCounter>(1)));
  m.addOpcode(621,
              0,
              "InitExFramesLoop",
              new MultiDispatch(new InitFrame<LoopFrameCounter>(1)));
  m.addOpcode(622,
              0,
              "InitExFramesTurn",
              new MultiDispatch(new InitFrame<TurnFrameCounter>(1)));
  m.addOpcode(623,
              0,
              "InitExFramesAccel",
              new MultiDispatch(new InitFrame<AcceleratingFrameCounter>(1)));
  m.addOpcode(624,
              0,
              "InitExFramesDecel",
              new MultiDispatch(new InitFrame<DeceleratingFrameCounter>(1)));
  m.addOpcode(630, 0, "ReadExFrames", new ReadFrames(1));
}
