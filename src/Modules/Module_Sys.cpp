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

#include "Effects/FadeEffect.hpp"
#include "Modules/Module_Sys_Frame.hpp"
#include "Modules/Module_Sys_Timer.hpp"
#include "Modules/Module_Sys_Save.hpp"
#include "Modules/Module_Sys_Syscom.hpp"
#include "Modules/Module_Sys_Date.hpp"
#include "Modules/Module_Sys_Name.hpp"
#include "Modules/Module_Sys_index_series.hpp"
#include "Modules/Module_Sys_Wait.hpp"
#include "MachineBase/RLOperation.hpp"
#include "MachineBase/GeneralOperations.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/EventSystem.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/TextSystem.hpp"
#include "Systems/Base/TextWindow.hpp"
#include "Systems/Base/SoundSystem.hpp"
#include "Systems/Base/Surface.hpp"
#include "Systems/Base/CGMTable.hpp"
#include "Utilities/StringUtilities.hpp"

#include <cmath>
#include <iostream>

const float PI = 3.14159265;

using namespace std;
using boost::shared_ptr;

//  fun title                   <1:Sys:00000, 0> (res 'sub-title')
struct Sys_title : public RLOp_Void_1< StrConstant_T > {
  void operator()(RLMachine& machine, std::string subtitle) {
    machine.system().graphics().setWindowSubtitle(
      subtitle, machine.getTextEncoding());
  }
};

// -----------------------------------------------------------------------

struct Sys_GetCursorPos_gc1
  : public RLOp_Void_4< IntReference_T, IntReference_T, IntReference_T,
						IntReference_T>
{
  void operator()(RLMachine& machine,
				  IntReferenceIterator xit, IntReferenceIterator yit,
                  IntReferenceIterator button1It, IntReferenceIterator button2It)
  {
    Point pos;
    int button1, button2;
    machine.system().event().getCursorPos(pos, button1, button2);
    *xit = pos.x();
    *yit = pos.y();
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
    Point pos = machine.system().event().getCursorPos();
    *xit = pos.x();
    *yit = pos.y();
  }
};

// -----------------------------------------------------------------------

struct Sys_PauseCursor : public RLOp_Void_1< IntConstant_T > {
  void operator()(RLMachine& machine, int newCursor) {
    machine.system().text().setKeyCursor(newCursor);
  }
};

// -----------------------------------------------------------------------

struct Sys_GetWakuAll : public RLOp_Store_Void {
  int operator()(RLMachine& machine) {
    boost::shared_ptr<TextWindow> window =
        machine.system().text().currentWindow();
    return window->wakuSet();
  }
};

// -----------------------------------------------------------------------

struct Sys_rnd_0 : public RLOp_Store_1< IntConstant_T > {
  int operator()(RLMachine& machine, int maxVal) {
    return (int)( double(maxVal) * rand()/(RAND_MAX + 1.0));
  }
};

// -----------------------------------------------------------------------

struct Sys_rnd_1 : public RLOp_Store_2< IntConstant_T, IntConstant_T > {
  int operator()(RLMachine& machine, int minVal, int maxVal) {
    return minVal + (int)( double(maxVal - minVal) * rand()/(RAND_MAX + 1.0));
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

// -----------------------------------------------------------------------

/**
 * Implements op<0:Sys:01203, 0>, ReturnMenu.
 *
 * Jumps the instruction pointer to the begining of the scenario
 * defined in the Gameexe key \#SEEN_MENU.
 *
 * This method also resets a LOT of the game state, though this isn't mentioned
 * in the rldev manual.
 */
struct Sys_ReturnMenu : public RLOp_Void_Void {
  virtual bool advanceInstructionPointer() { return false; }

  void operator()(RLMachine& machine) {
    int scenario = machine.system().gameexe()("SEEN_MENU").to_int();
    machine.localReset();
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

void Sys_MenuReturn::operator()(RLMachine& machine)
{
  GraphicsSystem& graphics = machine.system().graphics();

  // Render the screen as is.
  shared_ptr<Surface> dc0 = graphics.getDC(0);
  shared_ptr<Surface> before = graphics.renderToSurfaceWithBg(dc0);

  // Clear everything
  machine.localReset();

  shared_ptr<Surface> after = graphics.renderToSurfaceWithBg(dc0);

  // First, we jump the instruction pointer to the new location.
  int scenario = machine.system().gameexe()("SEEN_MENU").to_int();
  machine.jump(scenario);

  // Now we push a LongOperation on top of the stack; when this
  // ends, we'll be at SEEN_MENU.
  LongOperation* effect =
    new FadeEffect(machine, after, before, after->size(), 1000);
  machine.pushLongOperation(effect);
}


// -----------------------------------------------------------------------

SysModule::SysModule()
  : RLModule("Sys", 1, 004)
{
  addOpcode(   0, 0, "title", new Sys_title);

  addOpcode( 130, 0, "FlushClick", callFunction(&EventSystem::flushMouseClicks));
  addOpcode( 133, 0, "GetCursorPos", new Sys_GetCursorPos_gc1);

  addOpcode( 202, 0, "GetCursorPos", new Sys_GetCursorPos_gc2);

  addOpcode(204, 0, "ShowCursor", setToConstant(&GraphicsSystem::setShowCursor, 1));
  addOpcode(205, 0, "HideCursor", setToConstant(&GraphicsSystem::setShowCursor, 0));
  addOpcode(206, 0, "GetMouseCursor", returnIntValue(&GraphicsSystem::cursor));
  addOpcode(207, 0, "MouseCursor", callFunction(&GraphicsSystem::setCursor));

  addUnsupportedOpcode(330, 0, "EnableSkipMode");
  addUnsupportedOpcode(331, 0, "DisableSkipMode");
  addOpcode(332, 0, "LocalSkipMode", returnIntValue(&TextSystem::skipMode));
  addOpcode(333, 0, "SetLocalSkipMode",
            setToConstant(&TextSystem::setSkipMode, 1));
  addOpcode(334, 0, "ClearLocalSkipMode",
            setToConstant(&TextSystem::setSkipMode, 0));

  addOpcode( 350, 0, "CtrlKeyShip",
             returnIntValue(&TextSystem::ctrlKeySkip));
  addOpcode( 351, 0, "CtrlKeySkipOn",
             setToConstant(&TextSystem::setCtrlKeySkip, 1));
  addOpcode( 352, 0, "CtrlKeySkipOff",
             setToConstant(&TextSystem::setCtrlKeySkip, 0));
  addOpcode( 353, 0, "CtrlPressed",
             returnIntValue(&EventSystem::ctrlPressed));
  addOpcode( 354, 0, "ShiftPressed",
             returnIntValue(&EventSystem::shiftPressed));

  addOpcode( 364, 0, "PauseCursor", new Sys_PauseCursor);

  addOpcode( 410, 0, "GetWakuAll", new Sys_GetWakuAll);

  addUnsupportedOpcode(460, 0, "EnableWindowAnm");
  addUnsupportedOpcode(461, 0, "DisableWindowAnm");
  addUnsupportedOpcode(462, 0, "GetOpenAnmMod");
  addUnsupportedOpcode(463, 0, "SetOpenAnmMod");
  addUnsupportedOpcode(464, 0, "GetOpenAnmTime");
  addUnsupportedOpcode(465, 0, "SetOpenAnmTime");
  addUnsupportedOpcode(466, 0, "GetCloseAnmMod");
  addUnsupportedOpcode(467, 0, "SetCloseAnmMod");
  addUnsupportedOpcode(468, 0, "GetCloseAnmTime");
  addUnsupportedOpcode(469, 0, "SetCloseAnmTime");

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

  addOpcode(1120, 0, "SceneNum", returnIntValue(&RLMachine::sceneNumber));

  addOpcode(1200, 0, "end", callFunction(&RLMachine::halt));
  addOpcode(1201, 0, "MenuReturn", new Sys_MenuReturn);
  addOpcode(1202, 0, "MenuReturn2", new Sys_MenuReturn);
  addOpcode(1203, 0, "ReturnMenu", new Sys_ReturnMenu);
  addUnsupportedOpcode(1204, 0, "ReturnPrevSelect");
  addUnsupportedOpcode(1205, 0, "ReturnPrevSelect2");
  addOpcode(1203, 0, "ReturnMenu", new Sys_ReturnMenu);

  addOpcode(1130, 0, "DefaultGrp",
            returnStringValue(&GraphicsSystem::defaultGrpName));
  addOpcode(1131, 0, "SetDefaultGrp",
            callFunction(&GraphicsSystem::setDefaultGrpName));
  addOpcode(1132, 0, "DefaultBgr",
            returnStringValue(&GraphicsSystem::defaultBgrName));
  addOpcode(1133, 0, "SetDefaultBgr",
            callFunction(&GraphicsSystem::setDefaultBgrName));


  addUnsupportedOpcode(1302, 0, "nwSingle");
  addUnsupportedOpcode(1303, 0, "nwMulti");
  addUnsupportedOpcode(1312, 0, "nwSingleLocal");
  addUnsupportedOpcode(1313, 0, "nwMultiLocal");

  addOpcode(1500, 0, "cgGetTotal", returnIntValue(&CGMTable::getTotal));
  addOpcode(1501, 0, "cgGetViewed", returnIntValue(&CGMTable::getViewed));
  addOpcode(1502, 0, "cgGetViewedPcnt", returnIntValue(&CGMTable::getPercent));
  addOpcode(1503, 0, "cgGetFlag", returnIntValue(&CGMTable::getFlag));
  addOpcode(1504, 0, "cgStatus", returnIntValue(&CGMTable::getStatus));

  addUnsupportedOpcode(2050, 0, "SetCursorMono");
  addUnsupportedOpcode(2000, 0, "CursorMono");
  addOpcode(2051, 0, "SetSkipAnimations",
            callFunction(&GraphicsSystem::setSkipAnimations));
  addOpcode(2001, 0, "SkipAnimations",
            returnIntValue(&GraphicsSystem::skipAnimations));
  addOpcode(2052, 0, "SetLowPriority",
            callFunction(&System::setLowPriority));
  addOpcode(2002, 0, "LowPriority",
            returnIntValue(&System::lowPriority));

  addOpcode(2223, 0, "SetMessageSpeed",
            callFunction(&TextSystem::setMessageSpeed));
  addOpcode(2323, 0, "MessageSpeed",
            returnIntValue(&TextSystem::messageSpeed));
  addOpcode(2600, 0, "DefaultMessageSpeed",
            new ReturnGameexeInt("INIT_MESSAGE_SPEED", 0));

  addOpcode(2224, 0, "SetMessageNoWait",
            callFunction(&TextSystem::setMessageNoWait));
  addOpcode(2250, 0, "SetAutoMode",
            callFunction(&TextSystem::setAutoMode));
  addOpcode(2251, 0, "SetAutoCharTime",
            callFunction(&TextSystem::setAutoCharTime));
  addOpcode(2252, 0, "SetAutoBaseTime",
            callFunction(&TextSystem::setAutoCharTime));

  addUnsupportedOpcode(2225, 0, "SetKoeMode");
  addUnsupportedOpcode(2325, 0, "KoeMode");
  addUnsupportedOpcode(2226, 0, "SetBgmKoeFadeVol");
  addUnsupportedOpcode(2326, 0, "BgmKoeFadeVol");
  addUnsupportedOpcode(2602, 0, "DefBgmKoeFadeVol");
  addUnsupportedOpcode(2227, 0, "SetBgmKoeFade");
  addUnsupportedOpcode(2327, 0, "BgmKoeFade");
  addUnsupportedOpcode(2603, 0, "DefBgmKoeFade");
  addOpcode(2230, 0, "SetBgmVolMod",
            callFunction(&SoundSystem::setBgmVolume));
  addOpcode(2330, 0, "BgmVolMod",
            returnIntValue(&SoundSystem::bgmVolume));
  addUnsupportedOpcode(2231, 0, "SetKoeVolMod");
  addUnsupportedOpcode(2331, 0, "KoeVolMod");
  addOpcode(2232, 0, "SetPcmVolMod",
            callFunction(&SoundSystem::setPcmVolume));
  addOpcode(2332, 0, "PcmVolMod",
            returnIntValue(&SoundSystem::pcmVolume));
  addOpcode(2233, 0, "SetSeVolMod",
            callFunction(&SoundSystem::setSeVolume));
  addOpcode(2333, 0, "SeVolMod",
            returnIntValue(&SoundSystem::seVolume));
  addOpcode(2240, 0, "SetBgmEnabled",
            callFunction(&SoundSystem::setBgmEnabled));
  addOpcode(2340, 0, "BgmEnabled",
            returnIntValue(&SoundSystem::bgmEnabled));
  addUnsupportedOpcode(2241, 0, "SetKoeEnabled");
  addUnsupportedOpcode(2341, 0, "KoeEnabled");
  addOpcode(2242, 0, "SetPcmEnabled",
            callFunction(&SoundSystem::setPcmEnabled));
  addOpcode(2342, 0, "PcmEnabled",
            returnIntValue(&SoundSystem::pcmEnabled));
  addOpcode(2243, 0, "SetSeEnabled",
            callFunction(&SoundSystem::setSeEnabled));
  addOpcode(2343, 0, "SeEnabled",
            returnIntValue(&SoundSystem::seEnabled));

  addUnsupportedOpcode(2054, 0, "SetReduceDistortion");
  addUnsupportedOpcode(2004, 0, "ReduceDistortion");
  addUnsupportedOpcode(2059, 0, "SetSoundQuality");
  addUnsupportedOpcode(2009, 0, "SoundQuality");

  addOpcode(2221, 0, "SetGeneric1",
            callFunction(&EventSystem::setGeneric1));
  addOpcode(2620, 0, "DefGeneric1",
            new ReturnGameexeInt("INIT_ORIGINALSETING1_MOD", 0));
  addOpcode(2321, 0, "Generic1",
            returnIntValue(&EventSystem::generic1));
  addOpcode(2222, 0, "SetGeneric2",
            callFunction(&EventSystem::setGeneric2));
  addOpcode(2621, 0, "DefGeneric2",
            new ReturnGameexeInt("INIT_ORIGINALSETING2_MOD", 0));
  addOpcode(2322, 0, "Generic2",
            returnIntValue(&EventSystem::generic2));

  addOpcode(2260, 0, "SetWindowAttrR",
            callFunction(&TextSystem::setWindowAttrR));
  addOpcode(2261, 0, "SetWindowAttrG",
            callFunction(&TextSystem::setWindowAttrG));
  addOpcode(2262, 0, "SetWindowAttrB",
            callFunction(&TextSystem::setWindowAttrB));
  addOpcode(2263, 0, "SetWindowAttrA",
            callFunction(&TextSystem::setWindowAttrA));
  addOpcode(2264, 0, "SetWindowAttrF",
            callFunction(&TextSystem::setWindowAttrF));

  addOpcode(2267, 0, new Sys_SetWindowAttr);

  addUnsupportedOpcode(2273, 0, "SetClassifyText");
  addUnsupportedOpcode(2373, 0, "ClassifyText");
  addUnsupportedOpcode(2274, 0, "SetUseKoe");
  addUnsupportedOpcode(2374, 0, "UseKoe");
  addUnsupportedOpcode(2275, 0, "SetScreenMode");
  addUnsupportedOpcode(2375, 0, "ScreenMode");

  addOpcode(2350, 0, "AutoMode",
            returnIntValue(&TextSystem::autoMode));
  addOpcode(2351, 0, "AutoCharTime",
            returnIntValue(&TextSystem::autoCharTime));
  addOpcode(2352, 0, "AutoBaseTime",
            returnIntValue(&TextSystem::autoBaseTime));

  addOpcode(2360, 0, returnIntValue(&TextSystem::windowAttrR));
  addOpcode(2361, 0, returnIntValue(&TextSystem::windowAttrG));
  addOpcode(2362, 0, returnIntValue(&TextSystem::windowAttrB));
  addOpcode(2363, 0, returnIntValue(&TextSystem::windowAttrA));
  addOpcode(2364, 0, returnIntValue(&TextSystem::windowAttrF));

  addOpcode(2367, 0, new Sys_GetWindowAttr);

  addOpcode(2610, 0, new ReturnGameexeInt("WINDOW_ATTR", 0));
  addOpcode(2611, 0, new ReturnGameexeInt("WINDOW_ATTR", 1));
  addOpcode(2612, 0, new ReturnGameexeInt("WINDOW_ATTR", 2));
  addOpcode(2613, 0, new ReturnGameexeInt("WINDOW_ATTR", 3));
  addOpcode(2614, 0, new ReturnGameexeInt("WINDOW_ATTR", 4));

  addOpcode(2617, 0, new Sys_DefWindowAttr);

  addOpcode(2270, 0, "SetShowObject1",
			callFunction(&GraphicsSystem::setShowObject1));
  addOpcode(2370, 0, "ShowObject1",
			returnIntValue(&GraphicsSystem::showObject1));
  addOpcode(2271, 0, "SetShowObject2",
			callFunction(&GraphicsSystem::setShowObject2));
  addOpcode(2371, 0, "ShowObject2",
			returnIntValue(&GraphicsSystem::showObject2));
  addOpcode(2272, 0, "SetShowWeather",
            callFunction(&GraphicsSystem::setShowWeather));
  addOpcode(2372, 0, "ShowWeather",
            returnIntValue(&GraphicsSystem::showWeather));

  addOpcode(2324, 0, returnIntValue(&TextSystem::messageNoWait));
  addOpcode(2350, 0, returnIntValue(&TextSystem::autoMode));

  // Sys is hueg liek xbox, so lets group some of the operations by
  // what they do.
  addWaitAndMouseOpcodes(*this);
  addSysTimerOpcodes(*this);
  addSysFrameOpcodes(*this);
  addSysSaveOpcodes(*this);
  addSysSyscomOpcodes(*this);
  addSysDateOpcodes(*this);
  addSysNameOpcodes(*this);
  addIndexSeriesOpcode(*this);
}
