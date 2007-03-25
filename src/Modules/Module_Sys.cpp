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

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

/**
 * @file   Module_Sys.cpp
 * @author Elliot Glaysher
 * @date   Tue Nov 14 19:35:15 2006
 * 
 * @brief  Implements various commands that don't fit in other modules.
 *
 * VisualArts appears to have used this as a dumping ground for any
 * operations that don't otherwise fit into other categories.
 */

#include "libReallive/gameexe.h"

#include "Modules/Module_Sys.hpp"
#include "Modules/Module_Sys_Frame.hpp"
#include "Modules/Module_Sys_Timer.hpp"

#include "MachineBase/RLOperation.hpp"
#include "MachineBase/LongOperation.hpp"
#include "MachineBase/GeneralOperations.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/EventSystem.hpp"
#include "Systems/Base/GraphicsSystem.hpp"

#include "boost/date_time/gregorian/gregorian.hpp"
#include "boost/date_time/posix_time/posix_time_types.hpp"    

#include <cmath>
#include <iostream>

const float PI = 3.14159265;

using namespace std;

//  fun title                   <1:Sys:00000, 0> (res 'sub-title')
struct Sys_title : public RLOp_Void_1< StrConstant_T > {
  void operator()(RLMachine& machine, std::string subtitle) {
    machine.system().graphics().setWindowSubtitle(subtitle);
  }
};

// -----------------------------------------------------------------------

struct Sys_wait : public RLOp_Void_1< IntConstant_T > {
  struct LongOp_wait : public LongOperation 
  {
    unsigned int m_targetTime;

    LongOp_wait(RLMachine& machine, int time)
      : m_targetTime(machine.system().event().getTicks() + time)
    {}

    bool operator()(RLMachine& machine) 
    {
      machine.system().event().wait(10);
      return machine.system().event().getTicks() > m_targetTime;
    }
  };

  /// Simply set the long operation
  void operator()(RLMachine& machine, int time) {
    machine.setLongOperation(new LongOp_wait(machine, time));
  }
};

// -----------------------------------------------------------------------

struct Sys_waitC : public RLOp_Void_1< IntConstant_T > {
  struct LongOp_waitC : public LongOperation 
  {
    unsigned int m_targetTime;

    LongOp_waitC(RLMachine& machine, int time)
      : m_targetTime(machine.system().event().getTicks() + time)
    {}

    /** 
     * 
     * @todo Make this respond to clicks by the user.
     */
    bool operator()(RLMachine& machine) 
    {
      machine.system().event().wait(10);
      return machine.system().event().getTicks() > m_targetTime;
    }
  };

  /// Simply set the long operation
  void operator()(RLMachine& machine, int time) {
    machine.setLongOperation(new LongOp_waitC(machine, time));
  }
};

// -----------------------------------------------------------------------

struct Sys_FlushClick : public RLOp_Void_Void {
  void operator()(RLMachine& machine) {
    machine.system().event().flushMouseClicks();
  }
};

// -----------------------------------------------------------------------

struct Sys_GetCursorPos_gc1 
  : public RLOp_Void_4< IntReference_T, IntReference_T, IntReference_T, IntReference_T>
{
  void operator()(RLMachine& machine, IntReferenceIterator xit, IntReferenceIterator yit,
                  IntReferenceIterator button1It, IntReferenceIterator button2It) 
  {
    int x, y, button1, button2;
    machine.system().event().getCursorPos(x, y, button1, button2);
    *xit = x;
    *yit = y;
    *button1It = button1;
    *button2It = button2;
  }
};

// -----------------------------------------------------------------------

struct Sys_GetCursorPos_gc2
  : public RLOp_Void_2< IntReference_T, IntReference_T>
{
  void operator()(RLMachine& machine, IntReferenceIterator xit, IntReferenceIterator yit)
  {
    int x, y;
    machine.system().event().getCursorPos(x, y);
    *xit = x;
    *yit = y;
  }
};

// -----------------------------------------------------------------------

struct Sys_rnd_0 : public RLOp_Store_1< IntConstant_T > {
  int operator()(RLMachine& machine, int var1) {
    return rand() % var1;
  }
};

// -----------------------------------------------------------------------

struct Sys_rnd_1 : public RLOp_Store_2< IntConstant_T, IntConstant_T > {
  int operator()(RLMachine& machine, int var1, int var2) {
    return rand() % (var2 - var1) + var1;
  }
};

// -----------------------------------------------------------------------

struct Sys_pcnt : public RLOp_Store_2< IntConstant_T, IntConstant_T > {
  int operator()(RLMachine& machine, int numenator, int denominator) {
    return int( ((float)numenator / (float)denominator) * 100 );
  }
};

// -----------------------------------------------------------------------

struct Sys_abs : public RLOp_Store_1< IntConstant_T > {
  int operator()(RLMachine& machine, int var) {
    return abs(var);
  }
};

// -----------------------------------------------------------------------

struct Sys_power_0 : public RLOp_Store_1< IntConstant_T > {
  int operator()(RLMachine& machine, int var) {
    return var * var;
  }
};

// -----------------------------------------------------------------------

struct Sys_power_1 : public RLOp_Store_2< IntConstant_T, IntConstant_T > {
  int operator()(RLMachine& machine, int var1, int var2) {
    return (int)std::pow((float)var1, var2);
  }
};

// -----------------------------------------------------------------------

struct Sys_sin_0 : public RLOp_Store_1< IntConstant_T > {
  int operator()(RLMachine& machine, int var1) {
    return int( std::sin(var1 * (PI/180)) * 32640 );
  }
};

// -----------------------------------------------------------------------

struct Sys_sin_1 : public RLOp_Store_2< IntConstant_T, IntConstant_T > {
  int operator()(RLMachine& machine, int var1, int var2) {
    return int( std::sin(var1 * (PI/180)) * 32640 / var2 );
  }
};

// -----------------------------------------------------------------------

struct Sys_modulus : public RLOp_Store_4< IntConstant_T, IntConstant_T,
                                          IntConstant_T, IntConstant_T > {
  int operator()(RLMachine& machine, int var1, int var2, 
                 int var3, int var4) {
    return int( float(var1 - var3) / float(var2 - var4) );
  }
};

// -----------------------------------------------------------------------

struct Sys_angle : public RLOp_Store_4< IntConstant_T, IntConstant_T,
                                        IntConstant_T, IntConstant_T > {
  int operator()(RLMachine& machine, int var1, int var2, 
                 int var3, int var4) {
    return int( float(var1 - var3) / float(var2 - var4) );
  }
};

// -----------------------------------------------------------------------

struct Sys_min : public RLOp_Store_2< IntConstant_T, IntConstant_T > {
  int operator()(RLMachine& machine, int var1, int var2) {
    return std::min(var1, var2);
  }
};

// -----------------------------------------------------------------------

struct Sys_max : public RLOp_Store_2< IntConstant_T, IntConstant_T > {
  int operator()(RLMachine& machine, int var1, int var2) {
    return std::max(var1, var2);
  }
};

// -----------------------------------------------------------------------

struct Sys_constrain : public RLOp_Store_3< IntConstant_T, IntConstant_T, IntConstant_T > {
  int operator()(RLMachine& machine, int var1, int var2, int var3) {
    if(var2 < var1)
      return var1;
    else if(var2 > var3)
      return var3;
    else 
      return var2;
  }
};

// ----------------------------------------------------------- Date & Time Functions

/** 
 * Implements op<1:Sys:01100, 0>, fun GetYear().
 * 
 * Returns the current four digit year.
 */
struct Sys_GetYear : public RLOp_Store_Void {
  int operator()(RLMachine& machine) {
    return boost::gregorian::day_clock::local_day().year();
  }
};

// -----------------------------------------------------------------------

struct Sys_GetMonth : public RLOp_Store_Void {
  int operator()(RLMachine& machine) {
    return boost::gregorian::day_clock::local_day().month();
  }
};

// -----------------------------------------------------------------------

struct Sys_GetDay : public RLOp_Store_Void {
  int operator()(RLMachine& machine) {
    return boost::gregorian::day_clock::local_day().day();
  }
};

// -----------------------------------------------------------------------

struct Sys_GetDayOfWeek : public RLOp_Store_Void {
  int operator()(RLMachine& machine) {
    return boost::gregorian::day_clock::local_day().day_of_week();
  }
};

// -----------------------------------------------------------------------

struct Sys_GetHour : public RLOp_Store_Void {
  int operator()(RLMachine& machine) {
    return boost::posix_time::second_clock::local_time().time_of_day().hours();
  }
};

// -----------------------------------------------------------------------

struct Sys_GetMinute : public RLOp_Store_Void {
  int operator()(RLMachine& machine) {
    return boost::posix_time::second_clock::local_time().time_of_day().minutes();
  }
};

// -----------------------------------------------------------------------

struct Sys_GetSecond : public RLOp_Store_Void {
  int operator()(RLMachine& machine) {
    return boost::posix_time::second_clock::local_time().time_of_day().seconds();
  }
};

// -----------------------------------------------------------------------

struct Sys_GetMs : public RLOp_Store_Void {
  int operator()(RLMachine& machine) {
    return boost::posix_time::second_clock::local_time().time_of_day().fractional_seconds();
  }
};

// -----------------------------------------------------------------------

struct Sys_GetDate : public RLOp_Void_4< IntReference_T, IntReference_T,
                                         IntReference_T, IntReference_T> {
  void operator()(RLMachine& machine, IntReferenceIterator y, 
                  IntReferenceIterator m, IntReferenceIterator d,
                  IntReferenceIterator wd) {
    *y = (int)Sys_GetYear()(machine);
    *m = (int)Sys_GetMonth()(machine);
    *d = (int)Sys_GetDay()(machine);
    *wd = (int)Sys_GetDayOfWeek()(machine);
  }
};


// -----------------------------------------------------------------------

struct Sys_SceneNum : public RLOp_Store_Void {
  int operator()(RLMachine& machine) {
    return machine.sceneNumber();
  }
};

// -----------------------------------------------------------------------

struct Sys_end : public RLOp_Void_Void {
  void operator()(RLMachine& machine) {
    machine.halt();
  }
};

// -----------------------------------------------------------------------

/** 
 * Implements op<0:Sys:01203, 0>, ReturnMenu.
 * 
 * Jumps the instruction pointer to the begining of the scenario
 * defined in the Gameexe key \#SEEN_MENU.
 */
struct Sys_ReturnMenu : public RLOp_Void_Void {
  virtual bool advanceInstructionPointer() { return false; }

  void operator()(RLMachine& machine) {
    int scenario = machine.system().gameexe()("SEEN_MENU").to_int();
    machine.jump(scenario);
  }
};

// -----------------------------------------------------------------------

SysModule::SysModule(GraphicsSystem& system)
  : RLModule("Sys", 1, 004)
{
  addOpcode(   0, 0, "title", new Sys_title);
  addOpcode( 100, 0, "wait", new Sys_wait);
  addOpcode( 101, 0, "waitC", new Sys_waitC);

  addSysTimerOpcodes(*this);

  addOpcode( 130, 0, "FlushClick", new Sys_FlushClick);
  addOpcode( 133, 0, "GetCursorPos", new Sys_GetCursorPos_gc1);

  addOpcode( 202, 0, "GetCursorPos", new Sys_GetCursorPos_gc2);

  addOpcode(1000, 0, "rnd", new Sys_rnd_0);
  addOpcode(1000, 1, new Sys_rnd_1);
  addOpcode(1001, 0, new Sys_pcnt);
  addOpcode(1002, 0, new Sys_abs);
  addOpcode(1003, 0, new Sys_power_0);
  addOpcode(1003, 1, new Sys_power_1);
  addOpcode(1004, 0, new Sys_sin_0);
  addOpcode(1004, 1, new Sys_sin_1);
  addOpcode(1005, 0, new Sys_modulus);
  addOpcode(1006, 0, new Sys_angle);
  addOpcode(1007, 0, new Sys_min);
  addOpcode(1008, 0, new Sys_max);
  addOpcode(1009, 0, new Sys_constrain);
  // cos 01010
  // sign 01011
  // (unknown) 01012
  // (unknown) 01013

  addOpcode(1100, 0, new Sys_GetYear);
  addOpcode(1101, 0, new Sys_GetMonth);
  addOpcode(1102, 0, new Sys_GetDay);
  addOpcode(1103, 0, new Sys_GetDayOfWeek);
  addOpcode(1104, 0, new Sys_GetHour);
  addOpcode(1105, 0, new Sys_GetMinute);
  addOpcode(1106, 0, new Sys_GetSecond);
  addOpcode(1107, 0, new Sys_GetMs);
//  addOpcode(1110, 0, new Sys_GetDate);
//  addOpcode(1111, 0, new Sys_GetTime);
//  addOpcode(1112, 0, new Sys_GetDateTime);

  addOpcode(1120, 0, new Sys_SceneNum);

  addOpcode(1200, 0, "end", new Sys_end);
  addOpcode(1203, 0, "ReturnMenu", new Sys_ReturnMenu);

  addOpcode(1130, 0, new Op_ReturnStringValue<GraphicsSystem>(
              system, &GraphicsSystem::defaultGrpName));
  addOpcode(1131, 0, new Op_SetToIncomingString<GraphicsSystem>(
              system, &GraphicsSystem::setDefaultGrpName));
  addOpcode(1132, 0, new Op_ReturnStringValue<GraphicsSystem>(
              system, &GraphicsSystem::defaultBgrName));
  addOpcode(1133, 0, new Op_SetToIncomingString<GraphicsSystem>(
              system, &GraphicsSystem::setDefaultBgrName));

  // Sys is hueg liek xbox, so lets group some of the operations by
  // what they do.
  addSysFrameOpcodes(*this);
}
