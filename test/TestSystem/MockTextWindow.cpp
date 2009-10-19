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

#include "TestSystem/MockTextWindow.hpp"

#include "gmock/gmock.h"

using ::testing::_;
using ::testing::Invoke;

MockTextWindow::MockTextWindow(System& system, int win)
    : TestTextWindow(system, win) {
  ON_CALL(*this, execute())
      .WillByDefault(Invoke(this, &MockTextWindow::ConcreteExecute));
  ON_CALL(*this, setFontColor(_))
      .WillByDefault(Invoke(this, &MockTextWindow::ConcreteSetFontColor));
  ON_CALL(*this, displayChar(_, _))
      .WillByDefault(Invoke(this, &MockTextWindow::ConcreteDisplayChar));
  ON_CALL(*this, charWidth(_))
      .WillByDefault(Invoke(this, &MockTextWindow::ConcreteCharWidth));
  ON_CALL(*this, textSurface())
      .WillByDefault(Invoke(this, &MockTextWindow::ConcreteTextSurface));
  ON_CALL(*this, renderNameInBox(_))
      .WillByDefault(Invoke(this, &MockTextWindow::ConcreteRenderNameInBox));
  ON_CALL(*this, clearWin())
      .WillByDefault(Invoke(this, &MockTextWindow::ConcreteClearWin));
  ON_CALL(*this, setName(_, _))
      .WillByDefault(Invoke(this, &MockTextWindow::ConcreteSetName));
  ON_CALL(*this, hardBrake())
      .WillByDefault(Invoke(this, &MockTextWindow::ConcreteHardBrake));
  ON_CALL(*this, resetIndentation())
      .WillByDefault(Invoke(this, &MockTextWindow::ConcreteResetIndentation));
  ON_CALL(*this, markRubyBegin())
      .WillByDefault(Invoke(this, &MockTextWindow::ConcreteMarkRubyBegin));
  ON_CALL(*this, displayRubyText(_))
      .WillByDefault(Invoke(this, &MockTextWindow::ConcreteDisplayRubyText));
}

MockTextWindow::~MockTextWindow() {
}
