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

#include "modules/module_msg.h"

#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>

#include "libreallive/gameexe.h"
#include "long_operations/pause_long_operation.h"
#include "machine/general_operations.h"
#include "machine/rlmachine.h"
#include "machine/rlmodule.h"
#include "machine/rloperation.h"
#include "machine/rloperation/default_value.h"
#include "systems/base/system.h"
#include "systems/base/text_page.h"
#include "systems/base/text_system.h"
#include "systems/base/text_window.h"
#include "utilities/string_utilities.h"

namespace {

struct par : public RLOp_Void_Void {
  void operator()(RLMachine& machine) {
    TextPage& page = machine.system().text().currentPage();
    page.ResetIndentation();
    page.HardBrake();
  }
};

struct Msg_pause : public RLOp_Void_Void {
  void operator()(RLMachine& machine) {
    TextSystem& text = machine.system().text();
    int windowNum = text.activeWindow();
    boost::shared_ptr<TextWindow> textWindow = text.textWindow(windowNum);

    if (textWindow->actionOnPause()) {
      machine.PushLongOperation(
          new NewParagraphAfterLongop(new PauseLongOperation(machine)));
    } else {
      machine.PushLongOperation(
          new NewPageOnAllAfterLongop(new PauseLongOperation(machine)));
    }
  }
};

struct Msg_TextWindow : public RLOp_Void_1<DefaultIntValue_T<0>> {
  void operator()(RLMachine& machine, int window) {
    machine.system().text().setActiveWindow(window);
  }
};

struct FontColour
    : public RLOp_Void_2<DefaultIntValue_T<0>, DefaultIntValue_T<0>> {
  void operator()(RLMachine& machine, int textColorNum, int shadowColorNum) {
    machine.system().text().currentPage().FontColour(textColorNum);
  }
};

struct SetFontColour : public RLOp_Void_1<DefaultIntValue_T<0>> {
  void operator()(RLMachine& machine, int textColorNum) {
    Gameexe& gexe = machine.system().gameexe();
    if (gexe("COLOR_TABLE", textColorNum).Exists()) {
      machine.system().text().currentWindow()->setDefaultTextColor(
          gexe("COLOR_TABLE", textColorNum));
    }
  }
};

struct doruby_display : public RLOp_Void_1<StrConstant_T> {
  void operator()(RLMachine& machine, std::string cpStr) {
    std::string utf8str = cp932toUTF8(cpStr, machine.GetTextEncoding());
    machine.system().text().currentPage().DisplayRubyText(utf8str);
  }
};

struct msgHide : public RLOp_Void_1<DefaultIntValue_T<0>> {
  void operator()(RLMachine& machine, int unknown) {
    TextSystem& text = machine.system().text();
    int winNum = text.activeWindow();
    text.hideTextWindow(winNum);
    text.newPageOnWindow(winNum);
  }
};

struct msgHideAll : public RLOp_Void_Void {
  void operator()(RLMachine& machine) {
    TextSystem& text = machine.system().text();

    for (int window : text.activeWindows()) {
      text.hideTextWindow(window);
      text.newPageOnWindow(window);
    }
  }
};

struct msgClear : public RLOp_Void_Void {
  void operator()(RLMachine& machine) {
    TextSystem& text = machine.system().text();
    int activeWindow = text.activeWindow();
    text.snapshot();
    text.textWindow(activeWindow)->clearWin();
    text.newPageOnWindow(activeWindow);
  }
};

struct msgClearAll : public RLOp_Void_Void {
  void operator()(RLMachine& machine) {
    TextSystem& text = machine.system().text();
    std::vector<int> activeWindows = text.activeWindows();
    int activeWindow = text.activeWindow();

    text.snapshot();
    for (int window : activeWindows) {
      text.textWindow(activeWindow)->clearWin();
      text.newPageOnWindow(window);
    }
  }
};

struct spause : public RLOp_Void_Void {
  void operator()(RLMachine& machine) {
    machine.PushLongOperation(new PauseLongOperation(machine));
  }
};

struct page : public RLOp_Void_Void {
  void operator()(RLMachine& machine) {
    machine.PushLongOperation(
        new NewPageAfterLongop(new PauseLongOperation(machine)));
  }
};

struct TextPos : public RLOp_Void_2<IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, int x, int y) {
    TextPage& page = machine.system().text().currentPage();
    page.SetInsertionPointX(x);
    page.SetInsertionPointY(y);
  }
};

struct GetTextPos : public RLOp_Void_2<IntReference_T, IntReference_T> {
  void operator()(RLMachine& machine,
                  IntReferenceIterator x,
                  IntReferenceIterator y) {
    boost::shared_ptr<TextWindow> textWindow =
        machine.system().text().currentWindow();

    if (textWindow) {
      *x = textWindow->insertionPointX();
      *y = textWindow->insertionPointY();
    }
  }
};

struct TextOffset : public RLOp_Void_2<IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, int x, int y) {
    TextPage& page = machine.system().text().currentPage();
    page.OffsetInsertionPointX(x);
    page.OffsetInsertionPointY(y);
  }
};

struct FaceOpen : public RLOp_Void_2<StrConstant_T, DefaultIntValue_T<0>> {
  void operator()(RLMachine& machine, std::string file, int index) {
    TextPage& page = machine.system().text().currentPage();
    page.FaceOpen(file, index);
  }
};

struct FaceClose : public RLOp_Void_1<DefaultIntValue_T<0>> {
  void operator()(RLMachine& machine, int index) {
    TextPage& page = machine.system().text().currentPage();
    page.FaceClose(index);
  }
};

}  // namespace

MsgModule::MsgModule() : RLModule("Msg", 0, 003) {
  AddOpcode(3, 0, "par", new par);
  //  AddOpcode(15, 0, /* spause3 */ );
  AddOpcode(17, 0, "pause", new Msg_pause);

  AddUnsupportedOpcode(100, 0, "SetFontColour");
  AddOpcode(100, 1, "SetFontColour", new SetFontColour);
  AddOpcode(100, 2, "SetFontColour", new SetFontColour);
  AddOpcode(101, 0, "FontSize", CallFunction(&TextPage::FontSize));
  AddOpcode(101, 1, "FontSize", CallFunction(&TextPage::DefaultFontSize));

  AddOpcode(102, 0, "TextWindow", new Msg_TextWindow);
  AddOpcode(102, 1, "TextWindow", new Msg_TextWindow);

  AddOpcode(
      103, 0, "FastText", CallFunctionWith(&TextSystem::setFastTextMode, 1));
  AddOpcode(
      104, 0, "NormalText", CallFunctionWith(&TextSystem::setFastTextMode, 0));

  AddOpcode(105, 0, "FontColor", new FontColour);
  AddOpcode(105, 1, "FontColor", new FontColour);
  AddOpcode(105, 2, "FontColor", new FontColour);

  AddUnsupportedOpcode(106, 0, "SetFontColourAll");
  AddUnsupportedOpcode(106, 1, "SetFontColourAll");
  AddUnsupportedOpcode(106, 2, "SetFontColourAll");

  AddUnsupportedOpcode(107, 0, "FontSizeAll");

  AddOpcode(109,
            0,
            "messageNoWaitOn",
            CallFunctionWith(&TextSystem::setScriptMessageNowait, 1));
  AddOpcode(110,
            0,
            "messageNoWaitOff",
            CallFunctionWith(&TextSystem::setScriptMessageNowait, 0));

  AddOpcode(111, 0, "activeWindow", ReturnIntValue(&TextSystem::activeWindow));

  AddOpcode(120, 0, "__doruby_on", new doruby_display);
  AddOpcode(120, 1, "__doruby_off", CallFunction(&TextPage::MarkRubyBegin));

  AddOpcode(151, 0, "msgHide", new msgHide);
  AddOpcode(152, 0, "msgClear", new msgClear);

  AddOpcode(161, 0, "msgHideAll", new msgHideAll);
  AddOpcode(162, 0, "msgClearAll", new msgClearAll);
  AddUnsupportedOpcode(170, 0, "msgHideAllTemp");
  AddOpcode(201, 0, "br", CallFunction(&TextPage::HardBrake));
  AddOpcode(205, 0, "spause", new spause);
  AddUnsupportedOpcode(206, 0, "spause2");
  AddUnsupportedOpcode(207, 0, "pause_all");
  AddOpcode(210, 0, "page", new page);

  AddOpcode(300, 0, "SetIndent", CallFunction(&TextPage::SetIndentation));
  AddOpcode(301, 0, "ClearIndent", CallFunction(&TextPage::ResetIndentation));

  AddOpcode(310, 0, "TextPos", new TextPos);
  AddOpcode(311, 0, "TextPosX", CallFunction(&TextPage::SetInsertionPointX));
  AddOpcode(312, 0, "TextPosY", CallFunction(&TextPage::SetInsertionPointY));
  AddOpcode(320, 0, "TextOffset", new TextOffset);
  AddOpcode(
      321, 0, "TextOffsetX", CallFunction(&TextPage::OffsetInsertionPointX));
  AddOpcode(
      322, 0, "TextOffsetY", CallFunction(&TextPage::OffsetInsertionPointY));
  AddOpcode(330, 0, "GetTextPos", new GetTextPos);

  AddUnsupportedOpcode(340, 0, "WindowLen");
  AddUnsupportedOpcode(340, 1, "WindowLen");
  AddUnsupportedOpcode(341, 0, "WindowLenAll");

  AddOpcode(1000, 0, "FaceOpen", new FaceOpen);
  AddOpcode(1000, 1, "FaceOpen", new FaceOpen);
  AddOpcode(1001, 0, "FaceClose", new FaceClose);
  AddOpcode(1001, 1, "FaceClose", new FaceClose);
}
