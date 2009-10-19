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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
// -----------------------------------------------------------------------

#ifndef TEST_TESTSYSTEM_MOCKTEXTWINDOW_HPP_
#define TEST_TESTSYSTEM_MOCKTEXTWINDOW_HPP_

#include "gmock/gmock.h"

#include "TestSystem/TestTextWindow.hpp"

#include <string>
#include <vector>

// A TextWindow that acts as a mock, but delegates to a TestTextWindow.
class MockTextWindow : public TestTextWindow {
 public:
  MockTextWindow(System& system, int win);
  virtual ~MockTextWindow();

  MOCK_METHOD0(execute, void());
  MOCK_METHOD1(setFontColor, void(const std::vector<int>&));
  MOCK_METHOD2(displayChar, bool(const std::string&, const std::string&));
  MOCK_METHOD1(charWidth, int(uint16_t));
  MOCK_METHOD0(textSurface, boost::shared_ptr<Surface>());
  MOCK_METHOD1(renderNameInBox, void(const std::string&));
  MOCK_METHOD0(clearWin, void());
  MOCK_METHOD2(setName, void(const std::string&, const std::string&));
  MOCK_METHOD0(hardBrake, void());
  MOCK_METHOD0(resetIndentation, void());
  MOCK_METHOD0(markRubyBegin, void());
  MOCK_METHOD1(displayRubyText, void(const std::string&));

  // Calls to the parent class.
  void ConcreteExecute() { TestTextWindow::execute(); }
  void ConcreteSetFontColor(const std::vector<int>& colour_data) {
    TestTextWindow::setFontColor(colour_data);
  }
  bool ConcreteDisplayChar(const std::string& current,
                           const std::string& next) {
    return TestTextWindow::displayChar(current, next);
  }

  int ConcreteCharWidth(uint16_t codepoint) const {
    return TestTextWindow::charWidth(codepoint);
  }

  boost::shared_ptr<Surface> ConcreteTextSurface() {
    return TestTextWindow::textSurface();
  }

  void ConcreteRenderNameInBox(const std::string& utf8str) {
    TestTextWindow::renderNameInBox(utf8str);
  }

  void ConcreteClearWin() {
    TestTextWindow::clearWin();
  }

  void ConcreteSetName(const std::string& utf8name,
                       const std::string& next_char) {
    TestTextWindow::setName(utf8name, next_char);
  }

  void ConcreteHardBrake() {
    TestTextWindow::hardBrake();
  }

  void ConcreteResetIndentation() {
    TestTextWindow::resetIndentation();
  }

  void ConcreteMarkRubyBegin() {
    TestTextWindow::markRubyBegin();
  }

  void ConcreteDisplayRubyText(const std::string& utf8str) {
    TestTextWindow::displayRubyText(utf8str);
  }
};

#endif  // TEST_TESTSYSTEM_MOCKTEXTWINDOW_HPP_
