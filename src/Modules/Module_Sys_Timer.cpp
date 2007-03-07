// This file is part of RLVM, a RealLive virtual machine clone.
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

//#include "Modules/Module_Sys.hpp"
#include "MachineBase/RLOperation.hpp"
#include "MachineBase/RLMachine.hpp"
#include "MachineBase/RLModule.hpp"
//#include "MachineBase/GeneralOperations.hpp"

#include "Systems/Base/System.hpp"
#include "Systems/Base/EventSystem.hpp"
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
  m.addOpcode(110, 0, new Sys_ResetTimer(0));
  m.addOpcode(110, 1, new Sys_ResetTimer(0));
  m.addOpcode(114, 0, new Sys_Timer(0));
  m.addOpcode(114, 1, new Sys_Timer(0));
  m.addOpcode(115, 0, new Sys_CmpTimer(0));
  m.addOpcode(115, 1, new Sys_CmpTimer(0));
  m.addOpcode(116, 0, new Sys_SetTimer(0));
  m.addOpcode(116, 1, new Sys_SetTimer(0));

  m.addOpcode(120, 0, new Sys_ResetTimer(1));
  m.addOpcode(120, 1, new Sys_ResetTimer(1));
  m.addOpcode(124, 0, new Sys_Timer(1));
  m.addOpcode(124, 1, new Sys_Timer(1));
  m.addOpcode(125, 0, new Sys_CmpTimer(1));
  m.addOpcode(125, 1, new Sys_CmpTimer(1));
  m.addOpcode(126, 0, new Sys_SetTimer(1));
  m.addOpcode(126, 1, new Sys_SetTimer(1));
}
