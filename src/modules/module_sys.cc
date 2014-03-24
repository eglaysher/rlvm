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

// Implements various commands that don't fit in other modules.
//
// VisualArts appears to have used this as a dumping ground for any operations
// that don't otherwise fit into other categories. Because of this, the
// implementation has been split along themes into the different
// Module_Sys_*.cpp files.

#include "modules/module_sys.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

#include "effects/fade_effect.h"
#include "libreallive/gameexe.h"
#include "machine/general_operations.h"
#include "machine/rloperation.h"
#include "machine/rloperation/default_value.h"
#include "modules/module_sys_date.h"
#include "modules/module_sys_frame.h"
#include "modules/module_sys_index_series.h"
#include "modules/module_sys_name.h"
#include "modules/module_sys_save.h"
#include "modules/module_sys_syscom.h"
#include "modules/module_sys_timer.h"
#include "modules/module_sys_timetable2.h"
#include "modules/module_sys_wait.h"
#include "systems/base/cgm_table.h"
#include "systems/base/event_system.h"
#include "systems/base/graphics_system.h"
#include "systems/base/sound_system.h"
#include "systems/base/surface.h"
#include "systems/base/system.h"
#include "systems/base/text_system.h"
#include "systems/base/text_window.h"
#include "utilities/string_utilities.h"

const float PI = 3.14159265;

namespace {

struct title : public RLOp_Void_1<StrConstant_T> {
  void operator()(RLMachine& machine, std::string subtitle) {
    machine.system().graphics().setWindowSubtitle(subtitle,
                                                  machine.GetTextEncoding());
  }
};

struct GetTitle : public RLOp_Void_1<StrReference_T> {
  void operator()(RLMachine& machine, StringReferenceIterator dest) {
    *dest = machine.system().graphics().windowSubtitle();
  }
};

struct GetCursorPos_gc1 : public RLOp_Void_4<IntReference_T,
                                             IntReference_T,
                                             IntReference_T,
                                             IntReference_T> {
  void operator()(RLMachine& machine,
                  IntReferenceIterator xit,
                  IntReferenceIterator yit,
                  IntReferenceIterator button1It,
                  IntReferenceIterator button2It) {
    Point pos;
    int button1, button2;
    machine.system().event().getCursorPos(pos, button1, button2);
    *xit = pos.x();
    *yit = pos.y();
    *button1It = button1;
    *button2It = button2;
  }
};

struct GetCursorPos_gc2 : public RLOp_Void_2<IntReference_T, IntReference_T> {
  void operator()(RLMachine& machine,
                  IntReferenceIterator xit,
                  IntReferenceIterator yit) {
    Point pos = machine.system().event().getCursorPos();
    *xit = pos.x();
    *yit = pos.y();
  }
};

struct CallStackPop : RLOp_Void_1<DefaultIntValue_T<1>> {
  void operator()(RLMachine& machine, int frames_to_pop) {
    for (int i = 0; i < frames_to_pop; ++i)
      machine.PopStackFrame();
  }
};

struct PauseCursor : public RLOp_Void_1<IntConstant_T> {
  void operator()(RLMachine& machine, int newCursor) {
    machine.system().text().setKeyCursor(newCursor);
  }
};

struct GetWakuAll : public RLOp_Store_Void {
  int operator()(RLMachine& machine) {
    boost::shared_ptr<TextWindow> window =
        machine.system().text().currentWindow();
    return window->wakuSet();
  }
};

struct rnd_0 : public RLOp_Store_1<IntConstant_T> {
  unsigned int seedp_;
  rnd_0() : seedp_(time(NULL)) {}

  int operator()(RLMachine& machine, int maxVal) {
    return (int)(double(maxVal) * rand_r(&seedp_) / (RAND_MAX + 1.0));
  }
};

struct rnd_1 : public RLOp_Store_2<IntConstant_T, IntConstant_T> {
  unsigned int seedp_;
  rnd_1() : seedp_(time(NULL)) {}

  int operator()(RLMachine& machine, int minVal, int maxVal) {
    return minVal +
           (int)(double(maxVal - minVal) * rand_r(&seedp_) / (RAND_MAX + 1.0));
  }
};

struct pcnt : public RLOp_Store_2<IntConstant_T, IntConstant_T> {
  int operator()(RLMachine& machine, int numenator, int denominator) {
    return int(((float)numenator / (float)denominator) * 100);
  }
};

struct Sys_abs : public RLOp_Store_1<IntConstant_T> {
  int operator()(RLMachine& machine, int var) { return abs(var); }
};

struct power_0 : public RLOp_Store_1<IntConstant_T> {
  int operator()(RLMachine& machine, int var) { return var * var; }
};

struct power_1 : public RLOp_Store_2<IntConstant_T, IntConstant_T> {
  int operator()(RLMachine& machine, int var1, int var2) {
    return (int)std::pow((float)var1, var2);
  }
};

struct sin_0 : public RLOp_Store_1<IntConstant_T> {
  int operator()(RLMachine& machine, int var1) {
    return int(std::sin(var1 * (PI / 180)) * 32640);
  }
};

struct sin_1 : public RLOp_Store_2<IntConstant_T, IntConstant_T> {
  int operator()(RLMachine& machine, int var1, int var2) {
    return int(std::sin(var1 * (PI / 180)) * 32640 / var2);
  }
};

struct Sys_modulus : public RLOp_Store_4<IntConstant_T,
                                         IntConstant_T,
                                         IntConstant_T,
                                         IntConstant_T> {
  int operator()(RLMachine& machine, int var1, int var2, int var3, int var4) {
    return int(float(var1 - var3) / float(var2 - var4));
  }
};

struct angle : public RLOp_Store_4<IntConstant_T,
                                   IntConstant_T,
                                   IntConstant_T,
                                   IntConstant_T> {
  int operator()(RLMachine& machine, int var1, int var2, int var3, int var4) {
    return int(float(var1 - var3) / float(var2 - var4));
  }
};

struct Sys_min : public RLOp_Store_2<IntConstant_T, IntConstant_T> {
  int operator()(RLMachine& machine, int var1, int var2) {
    return std::min(var1, var2);
  }
};

struct Sys_max : public RLOp_Store_2<IntConstant_T, IntConstant_T> {
  int operator()(RLMachine& machine, int var1, int var2) {
    return std::max(var1, var2);
  }
};

struct constrain
    : public RLOp_Store_3<IntConstant_T, IntConstant_T, IntConstant_T> {
  int operator()(RLMachine& machine, int var1, int var2, int var3) {
    if (var2 < var1)
      return var1;
    else if (var2 > var3)
      return var3;
    else
      return var2;
  }
};

struct cos_0 : public RLOp_Store_1<IntConstant_T> {
  int operator()(RLMachine& machine, int var1) {
    return int(std::cos(var1 * (PI / 180)) * 32640);
  }
};

struct cos_1 : public RLOp_Store_2<IntConstant_T, IntConstant_T> {
  int operator()(RLMachine& machine, int var1, int var2) {
    return int(std::cos(var1 * (PI / 180)) * 32640 / var2);
  }
};

// Implements op<0:Sys:01203, 0>, ReturnMenu.
//
// Jumps the instruction pointer to the begining of the scenario defined in the
// Gameexe key #SEEN_MENU.
//
// This method also resets a LOT of the game state, though this isn't mentioned
// in the rldev manual.
struct ReturnMenu : public RLOp_Void_Void {
  virtual bool AdvanceInstructionPointer() override { return false; }

  void operator()(RLMachine& machine) {
    int scenario = machine.system().gameexe()("SEEN_MENU").to_int();
    machine.LocalReset();
    machine.Jump(scenario, 0);
  }
};

struct ReturnPrevSelect : public RLOp_Void_Void {
  virtual bool AdvanceInstructionPointer() override { return false; }

  void operator()(RLMachine& machine) {
    machine.system().restoreSelectionSnapshot(machine);
  }
};

struct SetWindowAttr : public RLOp_Void_5<IntConstant_T,
                                          IntConstant_T,
                                          IntConstant_T,
                                          IntConstant_T,
                                          IntConstant_T> {
  void operator()(RLMachine& machine, int r, int g, int b, int a, int f) {
    std::vector<int> attr(5);
    attr[0] = r;
    attr[1] = g;
    attr[2] = b;
    attr[3] = a;
    attr[4] = f;

    machine.system().text().setDefaultWindowAttr(attr);
  }
};

struct GetWindowAttr : public RLOp_Void_5<IntReference_T,
                                          IntReference_T,
                                          IntReference_T,
                                          IntReference_T,
                                          IntReference_T> {
  void operator()(RLMachine& machine,
                  IntReferenceIterator r,
                  IntReferenceIterator g,
                  IntReferenceIterator b,
                  IntReferenceIterator a,
                  IntReferenceIterator f) {
    TextSystem& text = machine.system().text();

    *r = text.windowAttrR();
    *g = text.windowAttrG();
    *b = text.windowAttrB();
    *a = text.windowAttrA();
    *f = text.windowAttrF();
  }
};

struct DefWindowAttr : public RLOp_Void_5<IntReference_T,
                                          IntReference_T,
                                          IntReference_T,
                                          IntReference_T,
                                          IntReference_T> {
  void operator()(RLMachine& machine,
                  IntReferenceIterator r,
                  IntReferenceIterator g,
                  IntReferenceIterator b,
                  IntReferenceIterator a,
                  IntReferenceIterator f) {
    Gameexe& gexe = machine.system().gameexe();
    std::vector<int> attr = gexe("WINDOW_ATTR");

    *r = attr.at(0);
    *g = attr.at(1);
    *b = attr.at(2);
    *a = attr.at(3);
    *f = attr.at(4);
  }
};

}  // namespace

bool Sys_MenuReturn::AdvanceInstructionPointer() { return false; }

// Implementation isn't in anonymous namespace since it is used elsewhere.
void Sys_MenuReturn::operator()(RLMachine& machine) {
  GraphicsSystem& graphics = machine.system().graphics();

  // Render the screen as is.
  boost::shared_ptr<Surface> dc0 = graphics.getDC(0);
  boost::shared_ptr<Surface> before = graphics.renderToSurface();

  // Clear everything
  machine.LocalReset();

  boost::shared_ptr<Surface> after = graphics.renderToSurface();

  // First, we jump the instruction pointer to the new location.
  int scenario = machine.system().gameexe()("SEEN_MENU").to_int();
  machine.Jump(scenario, 0);

  // Now we push a LongOperation on top of the stack; when this
  // ends, we'll be at SEEN_MENU.
  LongOperation* effect =
      new FadeEffect(machine, after, before, after->size(), 1000);
  machine.PushLongOperation(effect);
}

SysModule::SysModule() : RLModule("Sys", 1, 004) {
  addOpcode(0, 0, "title", new title);
  addOpcode(2, 0, "GetTitle", new GetTitle);

  addOpcode(130, 0, "FlushClick", CallFunction(&EventSystem::flushMouseClicks));
  addOpcode(133, 0, "GetCursorPos", new GetCursorPos_gc1);

  addOpcode(202, 0, "GetCursorPos", new GetCursorPos_gc2);

  addUnsupportedOpcode(320, 0, "CallStackClear");
  addUnsupportedOpcode(321, 0, "CallStackNop");
  addUnsupportedOpcode(321, 1, "CallStackNop");
  addOpcode(322, 0, "CallStackPop", new CallStackPop);
  addOpcode(322, 1, "CallStackPop", new CallStackPop);
  addUnsupportedOpcode(323, 0, "CallStackSize");
  addUnsupportedOpcode(324, 0, "CallStackTrunc");

  addOpcode(204,
            0,
            "ShowCursor",
            CallFunctionWith(&GraphicsSystem::setShowCursorFromBytecode, 1));
  addOpcode(205,
            0,
            "HideCursor",
            CallFunctionWith(&GraphicsSystem::setShowCursorFromBytecode, 0));
  addOpcode(206, 0, "GetMouseCursor", ReturnIntValue(&GraphicsSystem::cursor));
  addOpcode(207, 0, "MouseCursor", CallFunction(&GraphicsSystem::setCursor));

  addUnsupportedOpcode(330, 0, "EnableSkipMode");
  addUnsupportedOpcode(331, 0, "DisableSkipMode");
  addOpcode(332, 0, "LocalSkipMode", ReturnIntValue(&TextSystem::skipMode));
  addOpcode(333,
            0,
            "SetLocalSkipMode",
            CallFunctionWith(&TextSystem::setSkipMode, 1));
  addOpcode(334,
            0,
            "ClearLocalSkipMode",
            CallFunctionWith(&TextSystem::setSkipMode, 0));

  addOpcode(350, 0, "CtrlKeyShip", ReturnIntValue(&TextSystem::ctrlKeySkip));
  addOpcode(351,
            0,
            "CtrlKeySkipOn",
            CallFunctionWith(&TextSystem::setCtrlKeySkip, 1));
  addOpcode(352,
            0,
            "CtrlKeySkipOff",
            CallFunctionWith(&TextSystem::setCtrlKeySkip, 0));
  addOpcode(353, 0, "CtrlPressed", ReturnIntValue(&EventSystem::ctrlPressed));
  addOpcode(354, 0, "ShiftPressed", ReturnIntValue(&EventSystem::shiftPressed));

  addOpcode(364, 0, "PauseCursor", new PauseCursor);

  addUnsupportedOpcode(400, 0, "GetWindowPos");
  addUnsupportedOpcode(401, 0, "SetWindowPos");
  addUnsupportedOpcode(402, 0, "WindowResetPos");
  addUnsupportedOpcode(403, 0, "GetDefaultWindowPos");
  addUnsupportedOpcode(404, 0, "SetDefaultWindowPos");
  addUnsupportedOpcode(405, 0, "DefaultWindowResetPos");

  addOpcode(410, 0, "GetWakuAll", new GetWakuAll);
  addUnsupportedOpcode(411, 0, "SetWakuAll");
  addUnsupportedOpcode(412, 0, "GetWaku");
  addUnsupportedOpcode(413, 0, "SetWaku");
  addUnsupportedOpcode(414, 0, "GetWakuMod");
  addUnsupportedOpcode(415, 0, "SetWakuMod__dwm");
  addUnsupportedOpcode(416, 0, "SetWakuMod__ewm");

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

  addOpcode(1000, 0, "rnd", new rnd_0);
  addOpcode(1000, 1, "rnd", new rnd_1);
  addOpcode(1001, 0, "pcnt", new pcnt);
  addOpcode(1002, 0, "abs", new Sys_abs);
  addOpcode(1003, 0, "power", new power_0);
  addOpcode(1003, 1, "power", new power_1);
  addOpcode(1004, 0, "sin", new sin_0);
  addOpcode(1004, 1, "sin", new sin_1);
  addOpcode(1005, 0, "modulus", new Sys_modulus);
  addOpcode(1006, 0, "angle", new angle);
  addOpcode(1007, 0, "min", new Sys_min);
  addOpcode(1008, 0, "max", new Sys_max);
  addOpcode(1009, 0, "constrain", new constrain);
  addOpcode(1010, 0, "cos", new cos_0);
  addOpcode(1010, 1, "cos", new cos_1);
  // sign 01011
  // (unknown) 01012
  // (unknown) 01013

  addOpcode(1120, 0, "SceneNum", ReturnIntValue(&RLMachine::SceneNumber));

  addOpcode(1200, 0, "end", CallFunction(&RLMachine::Halt));
  addOpcode(1201, 0, "MenuReturn", new Sys_MenuReturn);
  addOpcode(1202, 0, "MenuReturn2", new Sys_MenuReturn);
  addOpcode(1203, 0, "ReturnMenu", new ReturnMenu);
  addOpcode(1204, 0, "ReturnPrevSelect", new ReturnPrevSelect);
  addOpcode(1205, 0, "ReturnPrevSelect2", new ReturnPrevSelect);

  addOpcode(1130,
            0,
            "DefaultGrp",
            returnStringValue(&GraphicsSystem::defaultGrpName));
  addOpcode(1131,
            0,
            "SetDefaultGrp",
            CallFunction(&GraphicsSystem::setDefaultGrpName));
  addOpcode(1132,
            0,
            "DefaultBgr",
            returnStringValue(&GraphicsSystem::defaultBgrName));
  addOpcode(1133,
            0,
            "SetDefaultBgr",
            CallFunction(&GraphicsSystem::setDefaultBgrName));

  addUnsupportedOpcode(1302, 0, "nwSingle");
  addUnsupportedOpcode(1303, 0, "nwMulti");
  addUnsupportedOpcode(1312, 0, "nwSingleLocal");
  addUnsupportedOpcode(1313, 0, "nwMultiLocal");

  addOpcode(1500, 0, "cgGetTotal", ReturnIntValue(&CGMTable::getTotal));
  addOpcode(1501, 0, "cgGetViewed", ReturnIntValue(&CGMTable::getViewed));
  addOpcode(1502, 0, "cgGetViewedPcnt", ReturnIntValue(&CGMTable::getPercent));
  addOpcode(1503, 0, "cgGetFlag", ReturnIntValue(&CGMTable::getFlag));
  addOpcode(1504, 0, "cgStatus", ReturnIntValue(&CGMTable::getStatus));

  addUnsupportedOpcode(2050, 0, "SetCursorMono");
  addUnsupportedOpcode(2000, 0, "CursorMono");
  addOpcode(2051,
            0,
            "SetSkipAnimations",
            CallFunction(&GraphicsSystem::setSkipAnimations));
  addOpcode(2001,
            0,
            "SkipAnimations",
            ReturnIntValue(&GraphicsSystem::skipAnimations));
  addOpcode(2052, 0, "SetLowPriority", CallFunction(&System::setLowPriority));
  addOpcode(2002, 0, "LowPriority", ReturnIntValue(&System::lowPriority));

  addOpcode(
      2223, 0, "SetMessageSpeed", CallFunction(&TextSystem::setMessageSpeed));
  addOpcode(2323, 0, "MessageSpeed", ReturnIntValue(&TextSystem::messageSpeed));
  addOpcode(2600,
            0,
            "DefaultMessageSpeed",
            new ReturnGameexeInt("INIT_MESSAGE_SPEED", 0));

  addOpcode(
      2224, 0, "SetMessageNoWait", CallFunction(&TextSystem::setMessageNoWait));
  addOpcode(
      2324, 0, "MessageNoWait", ReturnIntValue(&TextSystem::messageNoWait));
  addOpcode(2601,
            0,
            "DefMessageNoWait",
            new ReturnGameexeInt("INIT_MESSAGE_SPEED_MOD", 0));

  addOpcode(2250, 0, "SetAutoMode", CallFunction(&TextSystem::setAutoMode));
  addOpcode(2350, 0, "AutoMode", ReturnIntValue(&TextSystem::autoMode));
  addOpcode(
      2604, 0, "DefAutoMode", new ReturnGameexeInt("MESSAGE_KEY_WAIT_USE", 0));

  addOpcode(
      2251, 0, "SetAutoCharTime", CallFunction(&TextSystem::setAutoCharTime));
  addOpcode(2351, 0, "AutoCharTime", ReturnIntValue(&TextSystem::autoCharTime));
  addOpcode(2605,
            0,
            "DefAutoCharTime",
            new ReturnGameexeInt("INIT_MESSAGE_SPEED", 0));

  addOpcode(
      2252, 0, "SetAutoBaseTime", CallFunction(&TextSystem::setAutoBaseTime));
  addOpcode(2352, 0, "AutoBaseTime", ReturnIntValue(&TextSystem::autoBaseTime));
  addOpcode(2606,
            0,
            "DefAutoBaseTime",
            new ReturnGameexeInt("MESSAGE_KEY_WAIT_TIME", 0));

  addOpcode(2225, 0, "SetKoeMode", CallFunction(&SoundSystem::setKoeMode));
  addOpcode(2325, 0, "KoeMode", ReturnIntValue(&SoundSystem::koeMode));
  addOpcode(2226,
            0,
            "SetBgmKoeFadeVol",
            CallFunction(&SoundSystem::setBgmKoeFadeVolume));
  addOpcode(
      2326, 0, "BgmKoeFadeVol", ReturnIntValue(&SoundSystem::bgmKoeFadeVolume));
  addUnsupportedOpcode(2602, 0, "DefBgmKoeFadeVol");
  addOpcode(
      2227, 0, "SetBgmKoeFade", CallFunction(&SoundSystem::setBgmKoeFade));
  addOpcode(2327, 0, "BgmKoeFade", ReturnIntValue(&SoundSystem::bgmKoeFade));
  addUnsupportedOpcode(2603, 0, "DefBgmKoeFade");
  addOpcode(
      2230, 0, "SetBgmVolMod", CallFunction(&SoundSystem::setBgmVolumeMod));
  addOpcode(2330, 0, "BgmVolMod", ReturnIntValue(&SoundSystem::bgmVolumeMod));
  addOpcode(
      2231, 0, "SetKoeVolMod", CallFunction(&SoundSystem::setKoeVolumeMod));
  addOpcode(2331, 0, "KoeVolMod", ReturnIntValue(&SoundSystem::koeVolumeMod));
  addOpcode(
      2232, 0, "SetPcmVolMod", CallFunction(&SoundSystem::setPcmVolumeMod));
  addOpcode(2332, 0, "PcmVolMod", ReturnIntValue(&SoundSystem::pcmVolumeMod));
  addOpcode(2233, 0, "SetSeVolMod", CallFunction(&SoundSystem::setSeVolumeMod));
  addOpcode(2333, 0, "SeVolMod", ReturnIntValue(&SoundSystem::seVolumeMod));
  addOpcode(
      2240, 0, "SetBgmEnabled", CallFunction(&SoundSystem::setBgmEnabled));
  addOpcode(2340, 0, "BgmEnabled", ReturnIntValue(&SoundSystem::bgmEnabled));
  addOpcode(
      2241, 0, "SetKoeEnabled", CallFunction(&SoundSystem::setKoeEnabled));
  addOpcode(2341, 0, "KoeEnabled", ReturnIntValue(&SoundSystem::koeEnabled));
  addOpcode(
      2242, 0, "SetPcmEnabled", CallFunction(&SoundSystem::setPcmEnabled));
  addOpcode(2342, 0, "PcmEnabled", ReturnIntValue(&SoundSystem::pcmEnabled));
  addOpcode(2243, 0, "SetSeEnabled", CallFunction(&SoundSystem::setSeEnabled));
  addOpcode(2343, 0, "SeEnabled", ReturnIntValue(&SoundSystem::seEnabled));

  addOpcode(2256, 0, "SetFontWeight", CallFunction(&TextSystem::setFontWeight));
  addOpcode(2356, 0, "FontWeight", ReturnIntValue(&TextSystem::fontWeight));
  addOpcode(2257, 0, "SetFontShadow", CallFunction(&TextSystem::setFontShadow));
  addOpcode(2357, 0, "FontShadow", ReturnIntValue(&TextSystem::fontShadow));

  addUnsupportedOpcode(2054, 0, "SetReduceDistortion");
  addUnsupportedOpcode(2004, 0, "ReduceDistortion");
  addUnsupportedOpcode(2059, 0, "SetSoundQuality");
  addUnsupportedOpcode(2009, 0, "SoundQuality");

  addOpcode(2221, 0, "SetGeneric1", CallFunction(&EventSystem::setGeneric1));
  addOpcode(2620,
            0,
            "DefGeneric1",
            new ReturnGameexeInt("INIT_ORIGINALSETING1_MOD", 0));
  addOpcode(2321, 0, "Generic1", ReturnIntValue(&EventSystem::generic1));
  addOpcode(2222, 0, "SetGeneric2", CallFunction(&EventSystem::setGeneric2));
  addOpcode(2621,
            0,
            "DefGeneric2",
            new ReturnGameexeInt("INIT_ORIGINALSETING2_MOD", 0));
  addOpcode(2322, 0, "Generic2", ReturnIntValue(&EventSystem::generic2));

  addOpcode(
      2260, 0, "SetWindowAttrR", CallFunction(&TextSystem::setWindowAttrR));
  addOpcode(
      2261, 0, "SetWindowAttrG", CallFunction(&TextSystem::setWindowAttrG));
  addOpcode(
      2262, 0, "SetWindowAttrB", CallFunction(&TextSystem::setWindowAttrB));
  addOpcode(
      2263, 0, "SetWindowAttrA", CallFunction(&TextSystem::setWindowAttrA));
  addOpcode(
      2264, 0, "SetWindowAttrF", CallFunction(&TextSystem::setWindowAttrF));

  addOpcode(2267, 0, "SetWindowAttr", new SetWindowAttr);

  addUnsupportedOpcode(2273, 0, "SetClassifyText");
  addUnsupportedOpcode(2373, 0, "ClassifyText");
  addOpcode(
      2274, 0, "SetUseKoe", CallFunction(&SoundSystem::setUseKoeForCharacter));
  // Note: I don't understand how this overload differs, but CLANNAD_FV treats
  // it just like the previous one.
  addOpcode(
      2274, 1, "SetUseKoe", CallFunction(&SoundSystem::setUseKoeForCharacter));
  addOpcode(
      2374, 0, "UseKoe", ReturnIntValue(&SoundSystem::useKoeForCharacter));
  addOpcode(
      2275, 0, "SetScreenMode", CallFunction(&GraphicsSystem::setScreenMode));
  addOpcode(2375, 0, "ScreenMode", ReturnIntValue(&GraphicsSystem::screenMode));

  addOpcode(2360, 0, "WindowAttrR", ReturnIntValue(&TextSystem::windowAttrR));
  addOpcode(2361, 0, "WindowAttrG", ReturnIntValue(&TextSystem::windowAttrG));
  addOpcode(2362, 0, "WindowAttrB", ReturnIntValue(&TextSystem::windowAttrB));
  addOpcode(2363, 0, "WindowAttrA", ReturnIntValue(&TextSystem::windowAttrA));
  addOpcode(2364, 0, "WindowAttrF", ReturnIntValue(&TextSystem::windowAttrF));

  addOpcode(2367, 0, "GetWindowAttr", new GetWindowAttr);

  addOpcode(2610, 0, "DefWindowAttrR", new ReturnGameexeInt("WINDOW_ATTR", 0));
  addOpcode(2611, 0, "DefWindowAttrG", new ReturnGameexeInt("WINDOW_ATTR", 1));
  addOpcode(2612, 0, "DefWindowAttrB", new ReturnGameexeInt("WINDOW_ATTR", 2));
  addOpcode(2613, 0, "DefWindowAttrA", new ReturnGameexeInt("WINDOW_ATTR", 3));
  addOpcode(2614, 0, "DefWindowAttrF", new ReturnGameexeInt("WINDOW_ATTR", 4));

  addOpcode(2617, 0, "DefWindowAttr", new DefWindowAttr);

  addOpcode(
      2270, 0, "SetShowObject1", CallFunction(&GraphicsSystem::setShowObject1));
  addOpcode(
      2370, 0, "ShowObject1", ReturnIntValue(&GraphicsSystem::showObject1));
  addOpcode(
      2271, 0, "SetShowObject2", CallFunction(&GraphicsSystem::setShowObject2));
  addOpcode(
      2371, 0, "ShowObject2", ReturnIntValue(&GraphicsSystem::showObject2));
  addOpcode(
      2272, 0, "SetShowWeather", CallFunction(&GraphicsSystem::setShowWeather));
  addOpcode(
      2372, 0, "ShowWeather", ReturnIntValue(&GraphicsSystem::showWeather));

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
  addTimetable2Opcode(*this);
}
