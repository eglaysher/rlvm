// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2009 Elliot Glaysher
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
// -----------------------------------------------------------------------

#ifndef TEST_TEST_SYSTEM_MOCK_TEXT_WINDOW_H_
#define TEST_TEST_SYSTEM_MOCK_TEXT_WINDOW_H_

#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "test_system/test_text_window.h"

// A TextWindow that acts as a mock, but delegates to a TestTextWindow.
class MockTextWindow : public TestTextWindow {
 public:
  MockTextWindow(System& system, int win);
  virtual ~MockTextWindow();

  MOCK_METHOD1(SetFontColor, void(const std::vector<int>&));
  MOCK_METHOD2(DisplayCharacter, bool(const std::string&, const std::string&));
  MOCK_METHOD0(GetTextSurface, std::shared_ptr<Surface>());
  MOCK_METHOD1(RenderNameInBox, void(const std::string&));
  MOCK_METHOD0(clearWin, void());
  MOCK_METHOD2(SetName, void(const std::string&, const std::string&));
  MOCK_METHOD0(HardBrake, void());
  MOCK_METHOD0(ResetIndentation, void());
  MOCK_METHOD0(MarkRubyBegin, void());
  MOCK_METHOD1(DisplayRubyText, void(const std::string&));

  // Calls to the parent class.
  void ConcreteSetFontColor(const std::vector<int>& colour_data) {
    TestTextWindow::SetFontColor(colour_data);
  }
  bool ConcreteDisplayChar(const std::string& current,
                           const std::string& next) {
    return TestTextWindow::DisplayCharacter(current, next);
  }

  std::shared_ptr<Surface> ConcreteTextSurface() {
    return TestTextWindow::GetTextSurface();
  }

  void ConcreteRenderNameInBox(const std::string& utf8str) {
    TestTextWindow::RenderNameInBox(utf8str);
  }

  void ConcreteClearWin() { TestTextWindow::ClearWin(); }

  void ConcreteSetName(const std::string& utf8name,
                       const std::string& next_char) {
    TestTextWindow::SetName(utf8name, next_char);
  }

  void ConcreteHardBrake() { TestTextWindow::HardBrake(); }

  void ConcreteResetIndentation() { TestTextWindow::ResetIndentation(); }

  void ConcreteMarkRubyBegin() { TestTextWindow::MarkRubyBegin(); }

  void ConcreteDisplayRubyText(const std::string& utf8str) {
    TestTextWindow::DisplayRubyText(utf8str);
  }
};

#endif  // TEST_TEST_SYSTEM_MOCK_TEXT_WINDOW_H_
