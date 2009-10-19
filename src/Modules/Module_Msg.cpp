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
 * @file   Module_Msg.cpp
 * @author Elliot Glaysher
 * @date   Sun Oct  1 22:18:39 2006
 *
 * @brief  Implements many textout related operations.
 */

#include <string>
#include <vector>

#include "Modules/Module_Msg.hpp"
#include "MachineBase/GeneralOperations.hpp"
#include "MachineBase/RLOperation.hpp"
#include "MachineBase/RLOperation/DefaultValue.hpp"

#include "MachineBase/RLMachine.hpp"
#include "MachineBase/RLModule.hpp"

#include "libReallive/gameexe.h"
#include "LongOperations/PauseLongOperation.hpp"

#include "Systems/Base/System.hpp"
#include "Systems/Base/TextSystem.hpp"
#include "Systems/Base/TextPage.hpp"
#include "Systems/Base/TextWindow.hpp"
#include "Utilities/StringUtilities.hpp"

#include <boost/shared_ptr.hpp>

using namespace std;
using boost::shared_ptr;

namespace {

struct par : public RLOp_Void_Void {
  void operator()(RLMachine& machine) {
    TextPage& page = machine.system().text().currentPage();
    page.resetIndentation();
    page.hardBrake();
  }
};

struct Msg_pause : public RLOp_Void_Void {
  void operator()(RLMachine& machine) {
    TextSystem& text = machine.system().text();
    int windowNum = text.activeWindow();
    boost::shared_ptr<TextWindow> textWindow =
      text.textWindow(windowNum);

    if (textWindow->actionOnPause()) {
      machine.pushLongOperation(
        new NewParagraphAfterLongop(new PauseLongOperation(machine)));
    } else {
      machine.pushLongOperation(
        new NewPageAfterLongop(new PauseLongOperation(machine)));
    }
  }
};

struct Msg_TextWindow : public RLOp_Void_1< DefaultIntValue_T< 0 > > {
  void operator()(RLMachine& machine, int window) {
    machine.system().text().setActiveWindow(window);
  }
};

struct FontColour : public RLOp_Void_2< DefaultIntValue_T< 0 >,
                                        DefaultIntValue_T< 0 > > {
  void operator()(RLMachine& machine, int textColorNum, int shadowColorNum) {
    machine.system().text().currentPage().fontColour(textColorNum);
  }
};

struct doruby_display : public RLOp_Void_1< StrConstant_T > {
  void operator()(RLMachine& machine, std::string cpStr) {
    std::string utf8str = cp932toUTF8(cpStr, machine.getTextEncoding());
    machine.system().text().currentPage().displayRubyText(utf8str);
  }
};

struct msgHide : public RLOp_Void_1< DefaultIntValue_T< 0 > > {
  void operator()(RLMachine& machine, int unknown) {
    int winNum = machine.system().text().activeWindow();
    machine.system().text().hideTextWindow(winNum);
    machine.system().text().newPageOnWindow(winNum);
  }
};

struct msgHideAll : public RLOp_Void_Void {
  void operator()(RLMachine& machine) {
    TextSystem& text = machine.system().text();

    vector<int> activeWindows = text.activeWindows();
    for (vector<int>::const_iterator it = activeWindows.begin();
        it != activeWindows.end(); ++it) {
      text.hideTextWindow(*it);
      text.newPageOnWindow(*it);
    }
  }
};

struct msgClear : public RLOp_Void_Void {
  void operator()(RLMachine& machine) {
    TextSystem& text = machine.system().text();
    int activeWindow = text.activeWindow();
    text.textWindow(activeWindow)->clearWin();
    text.newPageOnWindow(activeWindow);
  }
};

struct msgClearAll : public RLOp_Void_Void {
  void operator()(RLMachine& machine) {
    TextSystem& text = machine.system().text();
    vector<int> activeWindows = text.activeWindows();
    int activeWindow = text.activeWindow();

    for (vector<int>::const_iterator it = activeWindows.begin();
        it != activeWindows.end(); ++it) {
      text.textWindow(activeWindow)->clearWin();
      text.newPageOnWindow(*it);
    }
  }
};

struct spause : public RLOp_Void_Void {
  void operator()(RLMachine& machine) {
    machine.pushLongOperation(new PauseLongOperation(machine));
  }
};

struct page : public RLOp_Void_Void {
  void operator()(RLMachine& machine) {
    machine.pushLongOperation(
      new NewPageAfterLongop(new PauseLongOperation(machine)));
  }
};

struct TextPos : public RLOp_Void_2<IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, int x, int y) {
    TextPage& page = machine.system().text().currentPage();
    page.setInsertionPointX(x);
    page.setInsertionPointY(y);
  }
};

struct GetTextPos : public RLOp_Void_2<IntReference_T, IntReference_T> {
  void operator()(RLMachine& machine, IntReferenceIterator x,
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
    page.offsetInsertionPointX(x);
    page.offsetInsertionPointY(y);
  }
};

struct FaceOpen : public RLOp_Void_2<StrConstant_T, DefaultIntValue_T<0> > {
  void operator()(RLMachine& machine, string file, int index) {
    TextPage& page = machine.system().text().currentPage();
    page.faceOpen(file, index);
  }
};

struct FaceClose : public RLOp_Void_1<DefaultIntValue_T<0> > {
  void operator()(RLMachine& machine, int index) {
    TextPage& page = machine.system().text().currentPage();
    page.faceClose(index);
  }
};

}  // namespace

// -----------------------------------------------------------------------

MsgModule::MsgModule()
  : RLModule("Msg", 0, 003) {
  addOpcode(3, 0, "par", new par);
//  addOpcode(15, 0, /* spause3 */ );
  addOpcode(17, 0, "pause", new Msg_pause);

  addUnsupportedOpcode(100, 0, "SetFontColour");
  addUnsupportedOpcode(100, 1, "SetFontColour");
  addUnsupportedOpcode(100, 2, "SetFontColour");
  addOpcode(101, 0, "FontSize", callFunction(&TextPage::fontSize));
  addOpcode(101, 1, "FontSize", callFunction(&TextPage::defaultFontSize));

  addOpcode(102, 0, "TextWindow", new Msg_TextWindow);
  addOpcode(102, 1, "TextWindow", new Msg_TextWindow);

  addOpcode(103, 0, "FastText", setToConstant(&TextSystem::setFastTextMode, 1));
  addOpcode(104, 0, "NormalText",
            setToConstant(&TextSystem::setFastTextMode, 0));

  addOpcode(105, 0, "FontColor", new FontColour);
  addOpcode(105, 1, "FontColor", new FontColour);
  addOpcode(105, 2, "FontColor", new FontColour);

  addUnsupportedOpcode(106, 0, "SetFontColourAll");
  addUnsupportedOpcode(106, 1, "SetFontColourAll");
  addUnsupportedOpcode(106, 2, "SetFontColourAll");

  addUnsupportedOpcode(107, 0, "FontSizeAll");

  addOpcode(120, 0, "__doruby_on", new doruby_display);
  addOpcode(120, 1, "__doruby_off", callFunction(&TextPage::markRubyBegin));

  addOpcode(151, 0, "msgHide", new msgHide);
  addOpcode(152, 0, "msgClear", new msgClear);

  addOpcode(161, 0, "msgHideAll", new msgHideAll);
  addOpcode(162, 0, "msgClearAll", new msgClearAll);
  addUnsupportedOpcode(170, 0, "msgHideAllTemp");
  addOpcode(201, 0, "br", callFunction(&TextPage::hardBrake));
  addOpcode(205, 0, "spause", new spause);
  addUnsupportedOpcode(206, 0, "spause2");
  addUnsupportedOpcode(207, 0, "pause_all");
  addOpcode(210, 0, "page", new page);

  addOpcode(300, 0, "SetIndent", callFunction(&TextPage::setIndentation));
  addOpcode(301, 0, "ClearIndent", callFunction(&TextPage::resetIndentation));

  addOpcode(310, 0, "TextPos", new TextPos);
  addOpcode(311, 0, "TextPosX", callFunction(&TextPage::setInsertionPointX));
  addOpcode(312, 0, "TextPosY", callFunction(&TextPage::setInsertionPointY));
  addOpcode(320, 0, "TextOffset", new TextOffset);
  addOpcode(321, 0, "TextOffsetX",
            callFunction(&TextPage::offsetInsertionPointX));
  addOpcode(322, 0, "TextOffsetY",
            callFunction(&TextPage::offsetInsertionPointY));
  addOpcode(330, 0, "GetTextPos", new GetTextPos);

  addUnsupportedOpcode(340, 0, "WindowLen");
  addUnsupportedOpcode(340, 1, "WindowLen");
  addUnsupportedOpcode(341, 0, "WindowLenAll");

  addOpcode(1000, 0, "FaceOpen", new FaceOpen);
  addOpcode(1000, 1, "FaceOpen", new FaceOpen);
  addOpcode(1001, 0, "FaceClose", new FaceClose);
  addOpcode(1001, 1, "FaceClose", new FaceClose);
}
