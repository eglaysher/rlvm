// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2007 Elliot Glaysher
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

#include "test_system/test_text_window.h"

#include <memory>
#include <sstream>
#include <string>

#include "systems/base/rect.h"
#include "test_system/mock_surface.h"

using std::ostringstream;

TestTextWindow::TestTextWindow(System& system, int x) : TextWindow(system, x) {}

TestTextWindow::~TestTextWindow() {}

std::shared_ptr<Surface> TestTextWindow::GetTextSurface() {
  // TODO(erg): May need to use a real size?
  return std::shared_ptr<Surface>(
      MockSurface::Create("Text Surface", Size(640, 480)));
}

std::shared_ptr<Surface> TestTextWindow::GetNameSurface() {
  return name_surface_;
}

void TestTextWindow::RenderNameInBox(const std::string& utf8str) {
  ostringstream oss;
  oss << "Name Surface [" << utf8str << "]";
  name_surface_.reset(MockSurface::Create(oss.str(), Size(640, 480)));
}

void TestTextWindow::ClearWin() {
  TextWindow::ClearWin();
  current_contents_ = "";
  name_surface_.reset();
}

void TestTextWindow::SetFontColor(const std::vector<int>& colour_data) {
  TextWindow::SetFontColor(colour_data);
}

bool TestTextWindow::DisplayCharacter(const std::string& current,
                                      const std::string& next) {
  bool ret = TextWindow::DisplayCharacter(current, next);
  // Must record after we've called superclass because DisplayCharacter() can
  // linebreak.
  current_contents_ += current;
  return ret;
}

void TestTextWindow::SetName(const std::string& utf8name,
                             const std::string& next_char) {
  TextWindow::SetName(utf8name, next_char);
}

void TestTextWindow::HardBrake() {
  TextWindow::HardBrake();
  current_contents_ += "\n";
}

void TestTextWindow::ResetIndentation() { TextWindow::ResetIndentation(); }

void TestTextWindow::MarkRubyBegin() { TextWindow::MarkRubyBegin(); }

void TestTextWindow::DisplayRubyText(const std::string& utf8str) {}
