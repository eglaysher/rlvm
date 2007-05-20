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
#include "Modules/cp932toUnicode.hpp"

#include "MachineBase/RLOperation.hpp"
#include "MachineBase/LongOperation.hpp"
#include "MachineBase/GeneralOperations.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/EventSystem.hpp"
#include "Systems/Base/EventHandler.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/TextSystem.hpp"

#include "boost/date_time/gregorian/gregorian.hpp"
#include "boost/date_time/posix_time/posix_time_types.hpp"    

#include <cmath>
#include <iostream>

const float PI = 3.14159265;

using namespace std;

//  fun title                   <1:Sys:00000, 0> (res 'sub-title')
struct Sys_title : public RLOp_Void_1< StrConstant_T > {
  void operator()(RLMachine& machine, std::string subtitle) {
    std::string utf8sub = cp932toUTF8(subtitle, machine.getTextEncoding());
    machine.system().graphics().setWindowSubtitle(utf8sub);
  }
};

// -----------------------------------------------------------------------

struct LongOp_wait : public NiceLongOperation, public EventHandler
{
  const unsigned int m_targetTime;
  const bool m_breakOnClicks;
  int m_buttonPressed;

  bool m_ctrlPressed;
  bool m_breakOnCtrlPressed;

  LongOp_wait(RLMachine& machine, int time, bool breakOnClicks)
    : NiceLongOperation(machine), EventHandler(machine),
      m_targetTime(machine.system().event().getTicks() + time),
      m_breakOnClicks(breakOnClicks), m_buttonPressed(0),
      m_ctrlPressed(false), 
      m_breakOnCtrlPressed(machine.system().text().ctrlKeySkip())
  {}

  /** 
   * Listen for mouseclicks (provided by EventHandler).
   */
  void mouseButtonStateChanged(MouseButton mouseButton, bool pressed)
  {
    if(pressed && m_breakOnClicks)
    {
      if(mouseButton == MOUSE_LEFT)
        m_buttonPressed = 1;
      else if(mouseButton == MOUSE_RIGHT)
        m_buttonPressed = -1;
    }
  }

  void keyStateChanged(KeyCode keyCode, bool pressed)
  {
    if(pressed && m_breakOnCtrlPressed && 
       (keyCode == RLKEY_RCTRL || keyCode == RLKEY_LCTRL))
      m_ctrlPressed = true;
  }

  bool operator()(RLMachine& machine)
  {
    EventSystem& es = machine.system().event();
    bool done = machine.system().event().getTicks() > m_targetTime ||
      m_ctrlPressed;

    if(m_breakOnClicks)
    {
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

struct Sys_wait : public RLOp_Void_1< IntConstant_T > {
  const bool m_cancelable;

  Sys_wait(bool cancelable) : m_cancelable(cancelable) {}

  /// Simply set the long operation
  void operator()(RLMachine& machine, int time) {
    machine.pushLongOperation(new LongOp_wait(machine, time, m_cancelable));
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

struct Sys_PauseCursor : public RLOp_Void_1< IntConstant_T > {
  void operator()(RLMachine& machine, int newCursor) {
    TextSystem& text = machine.system().text();
    text.setKeyCursor(machine, newCursor);
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

struct Sys_SetWindowAttr : public RLOp_Void_5<
  IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T, 
  IntConstant_T>
{
  void operator()(RLMachine& machine, int r, int g, int b, int a, int f)
  {
    vector<int> attr(5);
    attr[0] = r; 
    attr[1] = g;
    attr[2] = b;
    attr[3] = a;
    attr[4] = f;

    machine.system().text().setDefaultWindowAttr(attr);
  }
};

// -----------------------------------------------------------------------

struct Sys_GetWindowAttr : public RLOp_Void_5<
  IntReference_T, IntReference_T, IntReference_T, IntReference_T,
  IntReference_T>
{
  void operator()(RLMachine& machine, IntReferenceIterator r,
                  IntReferenceIterator g, IntReferenceIterator b,
                  IntReferenceIterator a, IntReferenceIterator f)
  {
    TextSystem& text = machine.system().text();

    *r = text.windowAttrR();
    *g = text.windowAttrG();
    *b = text.windowAttrB();
    *a = text.windowAttrA();
    *f = text.windowAttrF();
  }
};

// -----------------------------------------------------------------------

struct Sys_DefWindowAttr : public RLOp_Void_5<
  IntReference_T, IntReference_T, IntReference_T, IntReference_T,
  IntReference_T>
{
  void operator()(RLMachine& machine, IntReferenceIterator r,
                  IntReferenceIterator g, IntReferenceIterator b,
                  IntReferenceIterator a, IntReferenceIterator f)
  {
    Gameexe& gexe = machine.system().gameexe();
    vector<int> attr = gexe("WINDOW_ATTR");

    *r = attr.at(0);
    *g = attr.at(1);
    *b = attr.at(2);
    *a = attr.at(3);
    *f = attr.at(4);
  }
};

// -----------------------------------------------------------------------

SysModule::SysModule(System& system)
  : RLModule("Sys", 1, 004)
{
  GraphicsSystem& graphics = system.graphics();
  EventSystem& event = system.event();
  TextSystem& text = system.text();

  addOpcode(   0, 0, "title", new Sys_title);
  addOpcode( 100, 0, "wait", new Sys_wait(false));
  addOpcode( 101, 0, "waitC", new Sys_wait(true));

  addSysTimerOpcodes(*this);

  addOpcode( 130, 0, "FlushClick", new Sys_FlushClick);
  addOpcode( 133, 0, "GetCursorPos", new Sys_GetCursorPos_gc1);

  addOpcode( 202, 0, "GetCursorPos", new Sys_GetCursorPos_gc2);

  addOpcode( 350, 0, "CtrlKeyShip", 
             returnIntValue(text, &TextSystem::ctrlKeySkip));
  addOpcode( 351, 0, "CtrlKeySkipOn",
             setToConstant(text, &TextSystem::setCtrlKeySkip, 1));
  addOpcode( 352, 0, "CtrlKeySkipOff",
             setToConstant(text, &TextSystem::setCtrlKeySkip, 0));
  addOpcode( 353, 0, "CtrlPressed",
             returnIntValue(event, &EventSystem::ctrlPressed));
  addOpcode( 354, 0, "ShiftPressed",
             returnIntValue(event, &EventSystem::shiftPressed));

  addOpcode( 364, 0, "PauseCursor", new Sys_PauseCursor);
  
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

  addOpcode(1120, 0, "SceneNum", new Sys_SceneNum);

  addOpcode(1200, 0, "end", new Sys_end);
  addOpcode(1203, 0, "ReturnMenu", new Sys_ReturnMenu);

  addOpcode(1130, 0, "DefaultGrp", 
            returnStringValue(graphics, &GraphicsSystem::defaultGrpName));
  addOpcode(1131, 0, "SetDefaultGrp", 
            setToIncomingString(graphics, &GraphicsSystem::setDefaultGrpName));
  addOpcode(1132, 0, "DefaultBgr", 
            returnStringValue(graphics, &GraphicsSystem::defaultBgrName));
  addOpcode(1133, 0, "SetDefaultBgr",
            setToIncomingString(graphics, &GraphicsSystem::setDefaultBgrName));

  addUnsupportedOpcode(1210, 0, "ContextMenu");
  addUnsupportedOpcode(1211, 0, "EnableSyscom");
  addUnsupportedOpcode(1211, 1, "EnableSyscom");
  addUnsupportedOpcode(1212, 0, "HideSyscom");
  addUnsupportedOpcode(1212, 1, "HideSyscom");
  addUnsupportedOpcode(1213, 0, "DisableSyscom");
  addUnsupportedOpcode(1214, 0, "SyscomEnabled");

  addOpcode(2224, 0, "SetMessageNoWait",
            setToIncomingInt(text, &TextSystem::setMessageNoWait));
  addOpcode(2250, 0, "SetAutoMode",
            setToIncomingInt(text, &TextSystem::setAutoMode));
  addOpcode(2251, 0, "SetAutoCharTime",
            setToIncomingInt(text, &TextSystem::setAutoCharTime));
  addOpcode(2252, 0, "SetAutoBaseTime",
            setToIncomingInt(text, &TextSystem::setAutoCharTime));

  addOpcode(2221, 0, "SetGeneric1",
            setToIncomingInt(event, &EventSystem::setGeneric1));
  addOpcode(2620, 0, "DefGeneric1",
            new ReturnGameexeInt("INIT_ORIGINALSETING1_MOD", 0));
  addOpcode(2321, 0, "Generic1",
            returnIntValue(event, &EventSystem::generic1));
  addOpcode(2222, 0, "SetGeneric2",
            setToIncomingInt(event, &EventSystem::setGeneric2));
  addOpcode(2621, 0, "DefGeneric2",
            new ReturnGameexeInt("INIT_ORIGINALSETING2_MOD", 0));
  addOpcode(2322, 0, "Generic2",
            returnIntValue(event, &EventSystem::generic2));

  addOpcode(2260, 0, "SetWindowAttrR",
            setToIncomingInt(text, &TextSystem::setWindowAttrR));
  addOpcode(2261, 0, "SetWindowAttrG",
            setToIncomingInt(text, &TextSystem::setWindowAttrG));
  addOpcode(2262, 0, "SetWindowAttrB",
            setToIncomingInt(text, &TextSystem::setWindowAttrB));
  addOpcode(2263, 0, "SetWindowAttrA",
            setToIncomingInt(text, &TextSystem::setWindowAttrA));
  addOpcode(2264, 0, "SetWindowAttrF",
            setToIncomingInt(text, &TextSystem::setWindowAttrF));

  addOpcode(2267, 0, new Sys_SetWindowAttr);

  addUnsupportedOpcode(2375, 0, "ScreenMode");

  addOpcode(2350, 0, "AutoMode",
            returnIntValue(text, &TextSystem::autoMode));
  addOpcode(2351, 0, "AutoCharTime",
            returnIntValue(text, &TextSystem::autoCharTime));
  addOpcode(2352, 0, "AutoBaseTime",
            returnIntValue(text, &TextSystem::autoBaseTime));

  addOpcode(2360, 0, returnIntValue(text, &TextSystem::windowAttrR));
  addOpcode(2361, 0, returnIntValue(text, &TextSystem::windowAttrG));
  addOpcode(2362, 0, returnIntValue(text, &TextSystem::windowAttrB));
  addOpcode(2363, 0, returnIntValue(text, &TextSystem::windowAttrA));
  addOpcode(2364, 0, returnIntValue(text, &TextSystem::windowAttrF));

  addOpcode(2367, 0, new Sys_GetWindowAttr);

  addOpcode(2610, 0, new ReturnGameexeInt("WINDOW_ATTR", 0));
  addOpcode(2611, 0, new ReturnGameexeInt("WINDOW_ATTR", 1));
  addOpcode(2612, 0, new ReturnGameexeInt("WINDOW_ATTR", 2));
  addOpcode(2613, 0, new ReturnGameexeInt("WINDOW_ATTR", 3));
  addOpcode(2614, 0, new ReturnGameexeInt("WINDOW_ATTR", 4));

  addOpcode(2617, 0, new Sys_DefWindowAttr);

  addOpcode(2324, 0, returnIntValue(text, &TextSystem::messageNoWait));
  addOpcode(2350, 0, returnIntValue(text, &TextSystem::autoMode));

  // Sys is hueg liek xbox, so lets group some of the operations by
  // what they do.
  addSysFrameOpcodes(*this);
}
