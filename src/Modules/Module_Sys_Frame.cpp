// This file is part of RLVM, a RealLive virutal machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006 El Riot
//  
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
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

#include "Module_Sys_Frame.hpp"

#include "Modules/Module_Sys.hpp"
#include "MachineBase/RLOperation.hpp"
#include "MachineBase/RLMachine.hpp"
#include "MachineBase/RLModule.hpp"
#include "MachineBase/GeneralOperations.hpp"

#include "Systems/Base/System.hpp"
#include "Systems/Base/EventSystem.hpp"
#include "Systems/Base/FrameCounter.hpp"

//#include <iostream>
using namespace std;

template<typename FRAMECLASS>
struct Sys_InitFrame 
  : public RLOp_Void_4<IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T>
{
  const int m_layer;
  Sys_InitFrame(int layer) : m_layer(layer) {}

  void operator()(RLMachine& machine, int counter, int frameMin, int frameMax, 
                  int time)
  {
    EventSystem& es = machine.system().event();
    es.setFrameCounter(m_layer, counter, new FRAMECLASS(es, frameMin, frameMax, time));
  }
};

// -----------------------------------------------------------------------

struct Sys_ReadFrame : public RLOp_Store_1<IntConstant_T>
{
  const int m_layer;
  Sys_ReadFrame(int layer) : m_layer(layer) {}

  int operator()(RLMachine& machine, int counter)
  {
    EventSystem& es = machine.system().event();
    if(es.frameCounterExists(m_layer, counter))
      return es.getFrameCounter(m_layer, counter).readFrame(es);
    else
      return 0;
  }
};

// -----------------------------------------------------------------------

struct Sys_FrameActive : public RLOp_Store_1<IntConstant_T>
{
  const int m_layer;
  Sys_FrameActive(int layer) : m_layer(layer) {}

  int operator()(RLMachine& machine, int counter)
  {
    EventSystem& es = machine.system().event();
    if(es.frameCounterExists(m_layer, counter))
      return es.getFrameCounter(m_layer, counter).isActive();
    else
      return 0;
  }
};

// -----------------------------------------------------------------------

struct Sys_AnyFrameActive : public RLOp_Store_1<IntConstant_T>
{
  const int m_layer;
  Sys_AnyFrameActive(int layer) : m_layer(layer) {}

  int operator()(RLMachine& machine, int counter)
  {
    EventSystem& es = machine.system().event();
    for(int i = 0; i < 255; ++i) 
    {
      if(es.frameCounterExists(m_layer, counter) && 
         es.getFrameCounter(m_layer, counter).isActive())
      {
        return 1;
      }
    }

    return 0;
  }
};

// -----------------------------------------------------------------------

struct Sys_ClearFrame_0 : public RLOp_Void_1<IntConstant_T>
{
  const int m_layer;
  Sys_ClearFrame_0(int layer) : m_layer(layer) {}

  void operator()(RLMachine& machine, int counter)
  {
    EventSystem& es = machine.system().event();
    es.setFrameCounter(m_layer, counter, NULL);
  }
};

// -----------------------------------------------------------------------

struct Sys_ClearFrame_1 : public RLOp_Void_2<IntConstant_T, IntConstant_T>
{
  const int m_layer;
  Sys_ClearFrame_1(int layer) : m_layer(layer) {}

  void operator()(RLMachine& machine, int counter, int newValue)
  {
    FrameCounter& fc = machine.system().event().getFrameCounter(m_layer, counter);
    fc.setActive(false);
    fc.setValue(newValue);
  }
};

// -----------------------------------------------------------------------

struct Sys_ClearAllFrames_0 : public RLOp_Void_1<IntConstant_T>
{
  const int m_layer;
  Sys_ClearAllFrames_0(int layer) : m_layer(layer) {}

  void operator()(RLMachine& machine, int newValue)
  {
    EventSystem& es = machine.system().event();

    for(int i = 0; i < 255; ++i)
    {
      if(es.frameCounterExists(m_layer, i))
      {
        FrameCounter& fc = es.getFrameCounter(m_layer, i);
        fc.setActive(false);
        fc.setValue(newValue);
      }
    }
  }
};

// -----------------------------------------------------------------------

struct Sys_ClearAllFrames_1 : public RLOp_Void_Void
{
  const int m_layer;
  Sys_ClearAllFrames_1(int layer) : m_layer(layer) {}

  void operator()(RLMachine& machine)
  {
    EventSystem& es = machine.system().event();

    for(int i = 0; i < 255; ++i)
    {
      if(es.frameCounterExists(m_layer, i))
      {
        es.setFrameCounter(m_layer, i, NULL);
      }
    }
  }
};

// -----------------------------------------------------------------------

typedef Complex2_T<IntConstant_T, IntReference_T> FrameDataInReadFrames;

struct Sys_ReadFrames : public RLOp_Store_1< Argc_T<FrameDataInReadFrames> >
{
  const int m_layer;
  Sys_ReadFrames(int layer) : m_layer(layer) {}

  int operator()(RLMachine& machine, vector<FrameDataInReadFrames::type> frames)
  {
    static int callNum = 0;
    callNum++;
    EventSystem& es = machine.system().event();

    bool storeValue = false;

    for(vector<FrameDataInReadFrames::type>::iterator it = frames.begin();
        it != frames.end(); ++it)
    {
      int counter = it->get<0>();

      if(es.frameCounterExists(m_layer, counter))
      {
        int val = es.getFrameCounter(m_layer, counter).readFrame(es);
        *(it->get<1>()) = val;

        if(es.getFrameCounter(m_layer, counter).isActive())
          storeValue = true;
      }
      else
        *(it->get<1>()) = 0;

//       cerr << "Read frame: {" << it->get<0>() << ", " << *(it->get<1>())
//            << "}" << endl;
    }

    return storeValue;
  }
};

// -----------------------------------------------------------------------

void addSysFrameOpcodes(RLModule& m)
{
  // Normal frame counter operations
  m.addOpcode(500, 0, "InitFrame", new Sys_InitFrame<SimpleFrameCounter>(0));
  m.addOpcode(501, 0, "InitFrameLoop", new Sys_InitFrame<LoopFrameCounter>(0));
  m.addOpcode(502, 0, "InitFrameTurn", new Sys_InitFrame<TurnFrameCounter>(0));
  m.addOpcode(503, 0, "InitFrameAccel", new Sys_InitFrame<AcceleratingFrameCounter>(0));
  m.addOpcode(504, 0, "InitFrameDecel", new Sys_InitFrame<DeceleratingFrameCounter>(0));
  m.addOpcode(510, 0, "ReadFrame", new Sys_ReadFrame(0));
  m.addOpcode(511, 0, "FrameActive", new Sys_FrameActive(0));
  m.addOpcode(512, 0, "AnyFrameActive", new Sys_AnyFrameActive(0));
  m.addOpcode(513, 0, "ClearFrame", new Sys_ClearFrame_0(0));
  m.addOpcode(513, 1, "ClearFrame", new Sys_ClearFrame_1(0));
  m.addOpcode(514, 0, "ClearAllFrames", new Sys_ClearAllFrames_0(0));
  m.addOpcode(514, 1, "ClearAllFrames", new Sys_ClearAllFrames_1(0));

  // Extended frame counter operations
  m.addOpcode(520, 0, "InitExFrame", new Sys_InitFrame<SimpleFrameCounter>(1));
  m.addOpcode(521, 0, "InitExFrameLoop", new Sys_InitFrame<LoopFrameCounter>(1));
  m.addOpcode(522, 0, "InitExFrameTurn", new Sys_InitFrame<TurnFrameCounter>(1));
  m.addOpcode(523, 0, "InitExFrameAccel", new Sys_InitFrame<AcceleratingFrameCounter>(1));
  m.addOpcode(524, 0, "InitExFrameDecel", new Sys_InitFrame<DeceleratingFrameCounter>(1));
  m.addOpcode(530, 0, "ReadExFrame", new Sys_ReadFrame(1));
  m.addOpcode(531, 0, "ExFrameActive", new Sys_FrameActive(1));
  m.addOpcode(532, 0, "AnyExFrameActive", new Sys_AnyFrameActive(1));
  m.addOpcode(533, 0, "ClearExFrame", new Sys_ClearFrame_0(1));
  m.addOpcode(533, 1, "ClearExFrame", new Sys_ClearFrame_1(1));
  m.addOpcode(534, 0, "ClearAllExFrames", new Sys_ClearAllFrames_0(1));
  m.addOpcode(534, 1, "ClearAllExFrames", new Sys_ClearAllFrames_1(1));

  // Multiple dispatch operations on normal frame counters
  m.addOpcode(600, 0, "InitFrames", 
              new MultiDispatch(new Sys_InitFrame<SimpleFrameCounter>(0)));
  m.addOpcode(601, 0, "InitFramesLoop",
              new MultiDispatch(new Sys_InitFrame<LoopFrameCounter>(0)));
  m.addOpcode(602, 0, "InitFramesTurn",
              new MultiDispatch(new Sys_InitFrame<TurnFrameCounter>(0)));
  m.addOpcode(603, 0, "InitFramesAccel",
              new MultiDispatch(new Sys_InitFrame<AcceleratingFrameCounter>(0)));
  m.addOpcode(604, 0, "InitFramesDecel",
              new MultiDispatch(new Sys_InitFrame<DeceleratingFrameCounter>(0)));
  m.addOpcode(610, 0, "ReadFrames", new Sys_ReadFrames(0));

  // Multiple dispatch operations on normal frame counters
  m.addOpcode(620, 0, "InitExFrames", 
              new MultiDispatch(new Sys_InitFrame<SimpleFrameCounter>(1)));
  m.addOpcode(621, 0, "InitExFramesLoop",
              new MultiDispatch(new Sys_InitFrame<LoopFrameCounter>(1)));
  m.addOpcode(622, 0, "InitExFramesTurn",
              new MultiDispatch(new Sys_InitFrame<TurnFrameCounter>(1)));
  m.addOpcode(623, 0, "InitExFramesAccel",
              new MultiDispatch(new Sys_InitFrame<AcceleratingFrameCounter>(1)));
  m.addOpcode(624, 0, "InitExFramesDecel",
              new MultiDispatch(new Sys_InitFrame<DeceleratingFrameCounter>(1)));
  m.addOpcode(630, 0, "ReadExFrames", new Sys_ReadFrames(1));
}
