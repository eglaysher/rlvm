// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006 Elliot Glaysher
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

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

#include "Module_Sys_Frame.hpp"

#include "MachineBase/RLOperation.hpp"
#include "MachineBase/RLMachine.hpp"
#include "MachineBase/RLModule.hpp"
#include "MachineBase/LongOperation.hpp"

#include "Systems/Base/System.hpp"
#include "Systems/Base/EventSystem.hpp"
#include "Systems/Base/EventHandler.hpp"
#include "Systems/Base/FrameCounter.hpp"

//#include <iostream>
using namespace std;

struct Sys_ResetTimer : public RLOp_Void_1< DefaultIntValue_T< 0 > >
{
  const int m_layer;
  Sys_ResetTimer(const int in) : m_layer(in) {}

  void operator()(RLMachine& machine, int counter)
  {
    EventSystem& es = machine.system().event();
    es.getTimer(m_layer, counter).set(es);
  }
};

// -----------------------------------------------------------------------

struct LongOp_time : public LongOperation, public EventHandler
{
  const int m_layer;
  const int m_counter;
  const unsigned int m_targetTime;
  const bool m_cancelOnClick;

  int m_buttonPressed;
  
  LongOp_time(RLMachine& machine, int layer, int counter, int time,
              bool cancelOnClick)
    : EventHandler(machine),
      m_layer(layer), m_counter(counter), m_targetTime(time),
      m_cancelOnClick(cancelOnClick), m_buttonPressed(0)
  {}

  /** 
   * Listen for mouseclicks (provided by EventHandler).
   */
  void mouseButtonStateChanged(MouseButton mouseButton, bool pressed)
  {
    if(pressed)
    {
      if(mouseButton == MOUSE_LEFT)
        m_buttonPressed = 1;
      else if(mouseButton == MOUSE_RIGHT)
        m_buttonPressed = -1;
    }
  }

  bool operator()(RLMachine& machine) 
  {
    EventSystem& es = machine.system().event();
    bool done = false;

    // First check to see if we're done because of time.
    if(es.getTimer(m_layer, m_counter).read(es) > m_targetTime)
      done = true;

    if(m_cancelOnClick)
    {
      // The underlying timeC returns a value. Manually set that
      // value here.
      if(m_buttonPressed)
      {
        done = true;
        machine.setStoreRegister(m_buttonPressed);
        cerr << "Returning value of " << m_buttonPressed << endl;
      }
      else if(done)
        machine.setStoreRegister(0);
    }

    return done;
  }
};

// -----------------------------------------------------------------------

struct Sys_time : public RLOp_Void_2< IntConstant_T, DefaultIntValue_T< 0 > >
{
  const int m_layer;
  const bool m_inTimeC;
  Sys_time(const int in, const bool timeC) : m_layer(in), m_inTimeC(timeC) {}

  void operator()(RLMachine& machine, int time, int counter)
  {
    EventSystem& es = machine.system().event();

    if(es.getTimer(m_layer, counter).read(es) < time)
    {
      machine.pushLongOperation(new LongOp_time(machine, m_layer, counter, 
                                                time, m_inTimeC));
    }
  }
};

// -----------------------------------------------------------------------

struct Sys_Timer : public RLOp_Store_1< DefaultIntValue_T<0> >
{
  const int m_layer;
  Sys_Timer(const int in) : m_layer(in) {}

  int operator()(RLMachine& machine, int counter) 
  {
    EventSystem& es = machine.system().event();
    return es.getTimer(m_layer, counter).read(es);
  }
};

// -----------------------------------------------------------------------

struct Sys_CmpTimer : public RLOp_Store_2< IntConstant_T, DefaultIntValue_T<0> >
{
  const int m_layer;
  Sys_CmpTimer(const int in) : m_layer(in) {}

  int operator()(RLMachine& machine, int val, int counter)
  {
    EventSystem& es = machine.system().event();
    return es.getTimer(m_layer, counter).read(es) > val;
  }
};

// -----------------------------------------------------------------------

struct Sys_SetTimer : public RLOp_Void_2< IntConstant_T, DefaultIntValue_T<0> >
{
  const int m_layer;
  Sys_SetTimer(const int in) : m_layer(in) {}

  void operator()(RLMachine& machine, int val, int counter)
  {
    EventSystem& es = machine.system().event();
    es.getTimer(m_layer, counter).set(es, val);
  }
};


// -----------------------------------------------------------------------

void addSysTimerOpcodes(RLModule& m)
{
  m.addOpcode(110, 0, "ResetTimer", new Sys_ResetTimer(0));
  m.addOpcode(110, 1, "ResetTimer", new Sys_ResetTimer(0));
  m.addOpcode(111, 0, "time", new Sys_time(0, false));
  m.addOpcode(111, 1, "time", new Sys_time(0, false));
  m.addOpcode(112, 0, "timeC", new Sys_time(0, true));
  m.addOpcode(112, 1, "timeC", new Sys_time(0, true));
  m.addOpcode(114, 0, "Timer", new Sys_Timer(0));
  m.addOpcode(114, 1, "Timer", new Sys_Timer(0));
  m.addOpcode(115, 0, "CmpTimer", new Sys_CmpTimer(0));
  m.addOpcode(115, 1, "CmpTimer", new Sys_CmpTimer(0));
  m.addOpcode(116, 0, "CmpTimer", new Sys_SetTimer(0));
  m.addOpcode(116, 1, "CmpTimer", new Sys_SetTimer(0));

  m.addOpcode(120, 0, "ResetExTimer", new Sys_ResetTimer(1));
  m.addOpcode(120, 1, "ResetExTimer", new Sys_ResetTimer(1));
  m.addOpcode(121, 0, "timeEx", new Sys_time(1, false));
  m.addOpcode(121, 1, "timeEx", new Sys_time(1, false));
  m.addOpcode(122, 0, "timeExC", new Sys_time(1, true));
  m.addOpcode(122, 1, "timeExC", new Sys_time(1, true));
  m.addOpcode(124, 0, "ExTimer", new Sys_Timer(1));
  m.addOpcode(124, 1, "ExTimer", new Sys_Timer(1));
  m.addOpcode(125, 0, "CmpExTimer", new Sys_CmpTimer(1));
  m.addOpcode(125, 1, "CmpExTimer", new Sys_CmpTimer(1));
  m.addOpcode(126, 0, "SetExTimer", new Sys_SetTimer(1));
  m.addOpcode(126, 1, "SetExTimer", new Sys_SetTimer(1));
}
