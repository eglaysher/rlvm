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

#include "test_system/mock_text_window.h"

#include "gmock/gmock.h"

using ::testing::_;
using ::testing::Invoke;

MockTextWindow::MockTextWindow(System& system, int win)
    : TestTextWindow(system, win) {
  ON_CALL(*this, SetFontColor(_))
      .WillByDefault(Invoke(this, &MockTextWindow::ConcreteSetFontColor));
  ON_CALL(*this, DisplayCharacter(_, _))
      .WillByDefault(Invoke(this, &MockTextWindow::ConcreteDisplayChar));
  ON_CALL(*this, GetTextSurface())
      .WillByDefault(Invoke(this, &MockTextWindow::ConcreteTextSurface));
  ON_CALL(*this, RenderNameInBox(_))
      .WillByDefault(Invoke(this, &MockTextWindow::ConcreteRenderNameInBox));
  ON_CALL(*this, clearWin())
      .WillByDefault(Invoke(this, &MockTextWindow::ConcreteClearWin));
  ON_CALL(*this, SetName(_, _))
      .WillByDefault(Invoke(this, &MockTextWindow::ConcreteSetName));
  ON_CALL(*this, HardBrake())
      .WillByDefault(Invoke(this, &MockTextWindow::ConcreteHardBrake));
  ON_CALL(*this, ResetIndentation())
      .WillByDefault(Invoke(this, &MockTextWindow::ConcreteResetIndentation));
  ON_CALL(*this, MarkRubyBegin())
      .WillByDefault(Invoke(this, &MockTextWindow::ConcreteMarkRubyBegin));
  ON_CALL(*this, DisplayRubyText(_))
      .WillByDefault(Invoke(this, &MockTextWindow::ConcreteDisplayRubyText));
}

MockTextWindow::~MockTextWindow() {}
