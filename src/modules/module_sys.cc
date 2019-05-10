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

struct title : public RLOpcode<StrConstant_T> {
  void operator()(RLMachine& machine, std::string subtitle) {
    machine.system().graphics().SetWindowSubtitle(subtitle,
                                                  machine.GetTextEncoding());
  }
};

struct GetTitle : public RLOpcode<StrReference_T> {
  void operator()(RLMachine& machine, StringReferenceIterator dest) {
    *dest = machine.system().graphics().window_subtitle();
  }
};

struct GetCursorPos_gc1 : public RLOpcode<IntReference_T,
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
    machine.system().event().GetCursorPos(pos, button1, button2);
    *xit = pos.x();
    *yit = pos.y();
    *button1It = button1;
    *button2It = button2;
  }
};

struct GetCursorPos_gc2 : public RLOpcode<IntReference_T, IntReference_T> {
  void operator()(RLMachine& machine,
                  IntReferenceIterator xit,
                  IntReferenceIterator yit) {
    Point pos = machine.system().event().GetCursorPos();
    *xit = pos.x();
    *yit = pos.y();
  }
};

struct CallStackPop : RLOpcode<DefaultIntValue_T<1>> {
  void operator()(RLMachine& machine, int frames_to_pop) {
    for (int i = 0; i < frames_to_pop; ++i)
      machine.PopStackFrame();
  }
};

struct CallStackSize : RLStoreOpcode<> {
  int operator()(RLMachine& machine) {
    return machine.GetStackSize();
  }
};

struct PauseCursor : public RLOpcode<IntConstant_T> {
  void operator()(RLMachine& machine, int newCursor) {
    machine.system().text().SetKeyCursor(newCursor);
  }
};

struct GetWakuAll : public RLStoreOpcode<> {
  int operator()(RLMachine& machine) {
    std::shared_ptr<TextWindow> window =
        machine.system().text().GetCurrentWindow();
    return window->waku_set();
  }
};

struct rnd_0 : public RLStoreOpcode<IntConstant_T> {
  unsigned int seedp_;
  rnd_0() : seedp_(time(NULL)) {}

  int operator()(RLMachine& machine, int maxVal) {
    return (int)(double(maxVal) * rand_r(&seedp_) / (RAND_MAX + 1.0));
  }
};

struct rnd_1 : public RLStoreOpcode<IntConstant_T, IntConstant_T> {
  unsigned int seedp_;
  rnd_1() : seedp_(time(NULL)) {}

  int operator()(RLMachine& machine, int minVal, int maxVal) {
    return minVal +
           (int)(double(maxVal - minVal) * rand_r(&seedp_) / (RAND_MAX + 1.0));
  }
};

struct pcnt : public RLStoreOpcode<IntConstant_T, IntConstant_T> {
  int operator()(RLMachine& machine, int numenator, int denominator) {
    return int(((float)numenator / (float)denominator) * 100);
  }
};

struct Sys_abs : public RLStoreOpcode<IntConstant_T> {
  int operator()(RLMachine& machine, int var) { return abs(var); }
};

struct power_0 : public RLStoreOpcode<IntConstant_T> {
  int operator()(RLMachine& machine, int var) { return var * var; }
};

struct power_1 : public RLStoreOpcode<IntConstant_T, IntConstant_T> {
  int operator()(RLMachine& machine, int var1, int var2) {
    return (int)std::pow((float)var1, var2);
  }
};

struct sin_0 : public RLStoreOpcode<IntConstant_T> {
  int operator()(RLMachine& machine, int var1) {
    return int(std::sin(var1 * (PI / 180)) * 32640);
  }
};

struct sin_1 : public RLStoreOpcode<IntConstant_T, IntConstant_T> {
  int operator()(RLMachine& machine, int var1, int var2) {
    return int(std::sin(var1 * (PI / 180)) * 32640 / var2);
  }
};

struct Sys_modulus : public RLStoreOpcode<IntConstant_T,
                                         IntConstant_T,
                                         IntConstant_T,
                                         IntConstant_T> {
  int operator()(RLMachine& machine, int var1, int var2, int var3, int var4) {
    return int(float(var1 - var3) / float(var2 - var4));
  }
};

struct angle : public RLStoreOpcode<IntConstant_T,
                                   IntConstant_T,
                                   IntConstant_T,
                                   IntConstant_T> {
  int operator()(RLMachine& machine, int var1, int var2, int var3, int var4) {
    return int(float(var1 - var3) / float(var2 - var4));
  }
};

struct Sys_min : public RLStoreOpcode<IntConstant_T, IntConstant_T> {
  int operator()(RLMachine& machine, int var1, int var2) {
    return std::min(var1, var2);
  }
};

struct Sys_max : public RLStoreOpcode<IntConstant_T, IntConstant_T> {
  int operator()(RLMachine& machine, int var1, int var2) {
    return std::max(var1, var2);
  }
};

struct constrain
    : public RLStoreOpcode<IntConstant_T, IntConstant_T, IntConstant_T> {
  int operator()(RLMachine& machine, int var1, int var2, int var3) {
    if (var2 < var1)
      return var1;
    else if (var2 > var3)
      return var3;
    else
      return var2;
  }
};

struct cos_0 : public RLStoreOpcode<IntConstant_T> {
  int operator()(RLMachine& machine, int var1) {
    return int(std::cos(var1 * (PI / 180)) * 32640);
  }
};

struct cos_1 : public RLStoreOpcode<IntConstant_T, IntConstant_T> {
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
struct ReturnMenu : public RLOpcode<> {
  virtual bool AdvanceInstructionPointer() override { return false; }

  void operator()(RLMachine& machine) {
    int scenario = machine.system().gameexe()("SEEN_MENU").ToInt();
    machine.LocalReset();
    machine.Jump(scenario, 0);
  }
};

struct ReturnPrevSelect : public RLOpcode<> {
  virtual bool AdvanceInstructionPointer() override { return false; }

  void operator()(RLMachine& machine) {
    machine.system().RestoreSelectionSnapshot(machine);
  }
};

struct SetWindowAttr : public RLOpcode<IntConstant_T,
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

    machine.system().text().SetDefaultWindowAttr(attr);
  }
};

struct GetWindowAttr : public RLOpcode<IntReference_T,
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

    *r = text.window_attr_r();
    *g = text.window_attr_g();
    *b = text.window_attr_b();
    *a = text.window_attr_a();
    *f = text.window_attr_f();
  }
};

struct DefWindowAttr : public RLOpcode<IntReference_T,
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
  std::shared_ptr<Surface> dc0 = graphics.GetDC(0);
  std::shared_ptr<Surface> before = graphics.RenderToSurface();

  // Clear everything
  machine.LocalReset();

  std::shared_ptr<Surface> after = graphics.RenderToSurface();

  // First, we jump the instruction pointer to the new location.
  int scenario = machine.system().gameexe()("SEEN_MENU").ToInt();
  machine.Jump(scenario, 0);

  // Now we push a LongOperation on top of the stack; when this
  // ends, we'll be at SEEN_MENU.
  LongOperation* effect =
      new FadeEffect(machine, after, before, after->GetSize(), 1000);
  machine.PushLongOperation(effect);
}

SysModule::SysModule() : RLModule("Sys", 1, 004) {
  AddOpcode(0, 0, "title", new title);
  AddOpcode(2, 0, "GetTitle", new GetTitle);

  AddOpcode(130, 0, "FlushClick", CallFunction(&EventSystem::FlushMouseClicks));
  AddOpcode(133, 0, "GetCursorPos", new GetCursorPos_gc1);

  AddOpcode(202, 0, "GetCursorPos", new GetCursorPos_gc2);

  AddUnsupportedOpcode(320, 0, "CallStackClear");
  AddUnsupportedOpcode(321, 0, "CallStackNop");
  AddUnsupportedOpcode(321, 1, "CallStackNop");
  AddOpcode(322, 0, "CallStackPop", new CallStackPop);
  AddOpcode(322, 1, "CallStackPop", new CallStackPop);
  AddOpcode(323, 0, "CallStackSize", new CallStackSize);
  AddUnsupportedOpcode(324, 0, "CallStackTrunc");

  AddOpcode(204,
            0,
            "ShowCursor",
            CallFunctionWith(&GraphicsSystem::set_show_cursor_from_bytecode, 1));
  AddOpcode(205,
            0,
            "HideCursor",
            CallFunctionWith(&GraphicsSystem::set_show_cursor_from_bytecode, 0));
  AddOpcode(206, 0, "GetMouseCursor", ReturnIntValue(&GraphicsSystem::cursor));
  AddOpcode(207, 0, "MouseCursor", CallFunction(&GraphicsSystem::SetCursor));

  AddUnsupportedOpcode(330, 0, "EnableSkipMode");
  AddUnsupportedOpcode(331, 0, "DisableSkipMode");
  AddOpcode(332, 0, "LocalSkipMode", ReturnIntValue(&TextSystem::skip_mode));
  AddOpcode(333,
            0,
            "SetLocalSkipMode",
            CallFunctionWith(&TextSystem::SetSkipMode, 1));
  AddOpcode(334,
            0,
            "ClearLocalSkipMode",
            CallFunctionWith(&TextSystem::SetSkipMode, 0));

  AddOpcode(350, 0, "CtrlKeyShip", ReturnIntValue(&TextSystem::ctrl_key_skip));
  AddOpcode(351,
            0,
            "CtrlKeySkipOn",
            CallFunctionWith(&TextSystem::set_ctrl_key_skip, 1));
  AddOpcode(352,
            0,
            "CtrlKeySkipOff",
            CallFunctionWith(&TextSystem::set_ctrl_key_skip, 0));
  AddOpcode(353, 0, "CtrlPressed", ReturnIntValue(&EventSystem::CtrlPressed));
  AddOpcode(354, 0, "ShiftPressed", ReturnIntValue(&EventSystem::ShiftPressed));

  AddOpcode(364, 0, "PauseCursor", new PauseCursor);

  AddUnsupportedOpcode(400, 0, "GetWindowPos");
  AddUnsupportedOpcode(401, 0, "SetWindowPos");
  AddUnsupportedOpcode(402, 0, "WindowResetPos");
  AddUnsupportedOpcode(403, 0, "GetDefaultWindowPos");
  AddUnsupportedOpcode(404, 0, "SetDefaultWindowPos");
  AddUnsupportedOpcode(405, 0, "DefaultWindowResetPos");

  AddOpcode(410, 0, "GetWakuAll", new GetWakuAll);
  AddUnsupportedOpcode(411, 0, "SetWakuAll");
  AddUnsupportedOpcode(412, 0, "GetWaku");
  AddUnsupportedOpcode(413, 0, "SetWaku");
  AddUnsupportedOpcode(414, 0, "GetWakuMod");
  AddUnsupportedOpcode(415, 0, "SetWakuMod__dwm");
  AddUnsupportedOpcode(416, 0, "SetWakuMod__ewm");

  AddUnsupportedOpcode(460, 0, "EnableWindowAnm");
  AddUnsupportedOpcode(461, 0, "DisableWindowAnm");
  AddUnsupportedOpcode(462, 0, "GetOpenAnmMod");
  AddUnsupportedOpcode(463, 0, "SetOpenAnmMod");
  AddUnsupportedOpcode(464, 0, "GetOpenAnmTime");
  AddUnsupportedOpcode(465, 0, "SetOpenAnmTime");
  AddUnsupportedOpcode(466, 0, "GetCloseAnmMod");
  AddUnsupportedOpcode(467, 0, "SetCloseAnmMod");
  AddUnsupportedOpcode(468, 0, "GetCloseAnmTime");
  AddUnsupportedOpcode(469, 0, "SetCloseAnmTime");

  AddOpcode(1000, 0, "rnd", new rnd_0);
  AddOpcode(1000, 1, "rnd", new rnd_1);
  AddOpcode(1001, 0, "pcnt", new pcnt);
  AddOpcode(1002, 0, "abs", new Sys_abs);
  AddOpcode(1003, 0, "power", new power_0);
  AddOpcode(1003, 1, "power", new power_1);
  AddOpcode(1004, 0, "sin", new sin_0);
  AddOpcode(1004, 1, "sin", new sin_1);
  AddOpcode(1005, 0, "modulus", new Sys_modulus);
  AddOpcode(1006, 0, "angle", new angle);
  AddOpcode(1007, 0, "min", new Sys_min);
  AddOpcode(1008, 0, "max", new Sys_max);
  AddOpcode(1009, 0, "constrain", new constrain);
  AddOpcode(1010, 0, "cos", new cos_0);
  AddOpcode(1010, 1, "cos", new cos_1);
  // sign 01011
  // (unknown) 01012
  // (unknown) 01013

  AddOpcode(1120, 0, "SceneNum", ReturnIntValue(&RLMachine::SceneNumber));

  AddOpcode(1200, 0, "end", CallFunction(&RLMachine::Halt));
  AddOpcode(1201, 0, "MenuReturn", new Sys_MenuReturn);
  AddOpcode(1202, 0, "MenuReturn2", new Sys_MenuReturn);
  AddOpcode(1203, 0, "ReturnMenu", new ReturnMenu);
  AddOpcode(1204, 0, "ReturnPrevSelect", new ReturnPrevSelect);
  AddOpcode(1205, 0, "ReturnPrevSelect2", new ReturnPrevSelect);

  AddOpcode(1130,
            0,
            "DefaultGrp",
            returnStringValue(&GraphicsSystem::default_grp_name));
  AddOpcode(1131,
            0,
            "SetDefaultGrp",
            CallFunction(&GraphicsSystem::set_default_grp_name));
  AddOpcode(1132,
            0,
            "DefaultBgr",
            returnStringValue(&GraphicsSystem::default_bgr_name));
  AddOpcode(1133,
            0,
            "SetDefaultBgr",
            CallFunction(&GraphicsSystem::set_default_bgr_name));

  AddUnsupportedOpcode(1302, 0, "nwSingle");
  AddUnsupportedOpcode(1303, 0, "nwMulti");
  AddUnsupportedOpcode(1312, 0, "nwSingleLocal");
  AddUnsupportedOpcode(1313, 0, "nwMultiLocal");

  AddOpcode(1500, 0, "cgGetTotal", ReturnIntValue(&CGMTable::GetTotal));
  AddOpcode(1501, 0, "cgGetViewed", ReturnIntValue(&CGMTable::GetViewed));
  AddOpcode(1502, 0, "cgGetViewedPcnt", ReturnIntValue(&CGMTable::GetPercent));
  AddOpcode(1503, 0, "cgGetFlag", ReturnIntValue(&CGMTable::GetFlag));
  AddOpcode(1504, 0, "cgStatus", ReturnIntValue(&CGMTable::GetStatus));

  AddUnsupportedOpcode(2050, 0, "SetCursorMono");
  AddUnsupportedOpcode(2000, 0, "CursorMono");
  AddOpcode(2051,
            0,
            "SetSkipAnimations",
            CallFunction(&GraphicsSystem::set_should_skip_animations));
  AddOpcode(2001,
            0,
            "SkipAnimations",
            ReturnIntValue(&GraphicsSystem::should_skip_animations));
  AddOpcode(2052, 0, "SetLowPriority", CallFunction(&System::set_low_priority));
  AddOpcode(2002, 0, "LowPriority", ReturnIntValue(&System::low_priority));

  AddOpcode(
      2223, 0, "SetMessageSpeed", CallFunction(&TextSystem::set_message_speed));
  AddOpcode(2323, 0, "MessageSpeed", ReturnIntValue(&TextSystem::message_speed));
  AddOpcode(2600,
            0,
            "DefaultMessageSpeed",
            new ReturnGameexeInt("INIT_MESSAGE_SPEED", 0));

  AddOpcode(
      2224, 0, "SetMessageNoWait", CallFunction(&TextSystem::set_message_no_wait));
  AddOpcode(
      2324, 0, "MessageNoWait", ReturnIntValue(&TextSystem::message_no_wait));
  AddOpcode(2601,
            0,
            "DefMessageNoWait",
            new ReturnGameexeInt("INIT_MESSAGE_SPEED_MOD", 0));

  AddOpcode(2250, 0, "SetAutoMode", CallFunction(&TextSystem::SetAutoMode));
  AddOpcode(2350, 0, "AutoMode", ReturnIntValue(&TextSystem::auto_mode));
  AddOpcode(
      2604, 0, "DefAutoMode", new ReturnGameexeInt("MESSAGE_KEY_WAIT_USE", 0));

  AddOpcode(
      2251, 0, "SetAutoCharTime", CallFunction(&TextSystem::set_auto_char_time));
  AddOpcode(2351, 0, "AutoCharTime", ReturnIntValue(&TextSystem::autoCharTime));
  AddOpcode(2605,
            0,
            "DefAutoCharTime",
            new ReturnGameexeInt("INIT_MESSAGE_SPEED", 0));

  AddOpcode(
      2252, 0, "SetAutoBaseTime", CallFunction(&TextSystem::set_auto_base_time));
  AddOpcode(2352, 0, "AutoBaseTime", ReturnIntValue(&TextSystem::auto_base_time));
  AddOpcode(2606,
            0,
            "DefAutoBaseTime",
            new ReturnGameexeInt("MESSAGE_KEY_WAIT_TIME", 0));

  AddOpcode(2225, 0, "SetKoeMode", CallFunction(&SoundSystem::setKoeMode));
  AddOpcode(2325, 0, "KoeMode", ReturnIntValue(&SoundSystem::koe_mode));
  AddOpcode(2226,
            0,
            "SetBgmKoeFadeVol",
            CallFunction(&SoundSystem::set_bgm_koe_fadeVolume));
  AddOpcode(
      2326, 0, "BgmKoeFadeVol", ReturnIntValue(&SoundSystem::bgm_koe_fadeVolume));
  AddUnsupportedOpcode(2602, 0, "DefBgmKoeFadeVol");
  AddOpcode(
      2227, 0, "SetBgmKoeFade", CallFunction(&SoundSystem::set_bgm_koe_fade));
  AddOpcode(2327, 0, "BgmKoeFade", ReturnIntValue(&SoundSystem::bgm_koe_fade));
  AddUnsupportedOpcode(2603, 0, "DefBgmKoeFade");
  AddOpcode(
      2230, 0, "SetBgmVolMod", CallFunction(&SoundSystem::SetBgmVolumeMod));
  AddOpcode(2330, 0, "BgmVolMod", ReturnIntValue(&SoundSystem::bgm_volume_mod));
  AddOpcode(
      2231, 0, "SetKoeVolMod", CallFunction(&SoundSystem::SetKoeVolumeMod));
  AddOpcode(2331, 0, "KoeVolMod", ReturnIntValue(&SoundSystem::GetKoeVolume_mod));
  AddOpcode(
      2232, 0, "SetPcmVolMod", CallFunction(&SoundSystem::SetPcmVolumeMod));
  AddOpcode(2332, 0, "PcmVolMod", ReturnIntValue(&SoundSystem::pcm_volume_mod));
  AddOpcode(2233, 0, "SetSeVolMod", CallFunction(&SoundSystem::SetSeVolumeMod));
  AddOpcode(2333, 0, "SeVolMod", ReturnIntValue(&SoundSystem::se_volume_mod));
  AddOpcode(
      2240, 0, "SetBgmEnabled", CallFunction(&SoundSystem::SetBgmEnabled));
  AddOpcode(2340, 0, "BgmEnabled", ReturnIntValue(&SoundSystem::bgm_enabled));
  AddOpcode(
      2241, 0, "SetKoeEnabled", CallFunction(&SoundSystem::SetKoeEnabled));
  AddOpcode(2341, 0, "KoeEnabled", ReturnIntValue(&SoundSystem::is_koe_enabled));
  AddOpcode(
      2242, 0, "SetPcmEnabled", CallFunction(&SoundSystem::SetIsPcmEnabled));
  AddOpcode(2342, 0, "PcmEnabled", ReturnIntValue(&SoundSystem::is_pcm_enabled));
  AddOpcode(2243, 0, "SetSeEnabled", CallFunction(&SoundSystem::SetIsSeEnabled));
  AddOpcode(2343, 0, "SeEnabled", ReturnIntValue(&SoundSystem::is_se_enabled));

  AddOpcode(2256, 0, "SetFontWeight", CallFunction(&TextSystem::set_font_weight));
  AddOpcode(2356, 0, "FontWeight", ReturnIntValue(&TextSystem::font_weight));
  AddOpcode(2257, 0, "SetFontShadow", CallFunction(&TextSystem::set_font_shadow));
  AddOpcode(2357, 0, "FontShadow", ReturnIntValue(&TextSystem::font_shadow));

  AddUnsupportedOpcode(2054, 0, "SetReduceDistortion");
  AddUnsupportedOpcode(2004, 0, "ReduceDistortion");
  AddUnsupportedOpcode(2059, 0, "SetSoundQuality");
  AddUnsupportedOpcode(2009, 0, "SoundQuality");

  AddOpcode(2221, 0, "SetGeneric1", CallFunction(&EventSystem::set_generic1));
  AddOpcode(2620,
            0,
            "DefGeneric1",
            new ReturnGameexeInt("INIT_ORIGINALSETING1_MOD", 0));
  AddOpcode(2321, 0, "Generic1", ReturnIntValue(&EventSystem::generic1));
  AddOpcode(2222, 0, "SetGeneric2", CallFunction(&EventSystem::set_generic2));
  AddOpcode(2621,
            0,
            "DefGeneric2",
            new ReturnGameexeInt("INIT_ORIGINALSETING2_MOD", 0));
  AddOpcode(2322, 0, "Generic2", ReturnIntValue(&EventSystem::generic2));

  AddOpcode(
      2260, 0, "SetWindowAttrR", CallFunction(&TextSystem::SetWindowAttrR));
  AddOpcode(
      2261, 0, "SetWindowAttrG", CallFunction(&TextSystem::SetWindowAttrG));
  AddOpcode(
      2262, 0, "SetWindowAttrB", CallFunction(&TextSystem::SetWindowAttrB));
  AddOpcode(
      2263, 0, "SetWindowAttrA", CallFunction(&TextSystem::SetWindowAttrA));
  AddOpcode(
      2264, 0, "SetWindowAttrF", CallFunction(&TextSystem::SetWindowAttrF));

  AddOpcode(2267, 0, "SetWindowAttr", new SetWindowAttr);

  AddUnsupportedOpcode(2273, 0, "SetClassifyText");
  AddUnsupportedOpcode(2373, 0, "ClassifyText");
  AddOpcode(
      2274, 0, "SetUseKoe", CallFunction(&SoundSystem::SetUseKoeForCharacter));
  // Note: I don't understand how this overload differs, but CLANNAD_FV treats
  // it just like the previous one.
  AddOpcode(
      2274, 1, "SetUseKoe", CallFunction(&SoundSystem::SetUseKoeForCharacter));
  AddOpcode(
      2374, 0, "UseKoe", ReturnIntValue(&SoundSystem::ShouldUseKoeForCharacter));
  AddOpcode(
      2275, 0, "SetScreenMode", CallFunction(&GraphicsSystem::SetScreenMode));
  AddOpcode(2375, 0, "ScreenMode", ReturnIntValue(&GraphicsSystem::screen_mode));

  AddOpcode(2360, 0, "WindowAttrR", ReturnIntValue(&TextSystem::window_attr_r));
  AddOpcode(2361, 0, "WindowAttrG", ReturnIntValue(&TextSystem::window_attr_g));
  AddOpcode(2362, 0, "WindowAttrB", ReturnIntValue(&TextSystem::window_attr_b));
  AddOpcode(2363, 0, "WindowAttrA", ReturnIntValue(&TextSystem::window_attr_a));
  AddOpcode(2364, 0, "WindowAttrF", ReturnIntValue(&TextSystem::window_attr_f));

  AddOpcode(2367, 0, "GetWindowAttr", new GetWindowAttr);

  AddOpcode(2610, 0, "DefWindowAttrR", new ReturnGameexeInt("WINDOW_ATTR", 0));
  AddOpcode(2611, 0, "DefWindowAttrG", new ReturnGameexeInt("WINDOW_ATTR", 1));
  AddOpcode(2612, 0, "DefWindowAttrB", new ReturnGameexeInt("WINDOW_ATTR", 2));
  AddOpcode(2613, 0, "DefWindowAttrA", new ReturnGameexeInt("WINDOW_ATTR", 3));
  AddOpcode(2614, 0, "DefWindowAttrF", new ReturnGameexeInt("WINDOW_ATTR", 4));

  AddOpcode(2617, 0, "DefWindowAttr", new DefWindowAttr);

  AddOpcode(
      2270, 0, "SetShowObject1", CallFunction(&GraphicsSystem::set_should_show_object1));
  AddOpcode(
      2370, 0, "ShowObject1", ReturnIntValue(&GraphicsSystem::should_show_object1));
  AddOpcode(
      2271, 0, "SetShowObject2", CallFunction(&GraphicsSystem::set_should_show_object2));
  AddOpcode(
      2371, 0, "ShowObject2", ReturnIntValue(&GraphicsSystem::should_show_object2));
  AddOpcode(
      2272, 0, "SetShowWeather", CallFunction(&GraphicsSystem::set_should_show_weather));
  AddOpcode(
      2372, 0, "ShowWeather", ReturnIntValue(&GraphicsSystem::should_show_weather));

  // Sys is hueg liek xbox, so lets group some of the operations by
  // what they do.
  AddWaitAndMouseOpcodes(*this);
  AddSysTimerOpcodes(*this);
  AddSysFrameOpcodes(*this);
  AddSysSaveOpcodes(*this);
  AddSysSyscomOpcodes(*this);
  AddSysDateOpcodes(*this);
  AddSysNameOpcodes(*this);
  AddIndexSeriesOpcode(*this);
  AddTimetable2Opcode(*this);
}
