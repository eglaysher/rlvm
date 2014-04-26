// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2008 Elliot Glaysher
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

#include "test_system/test_event_system.h"

// -----------------------------------------------------------------------
// TestEventSystem
// -----------------------------------------------------------------------
TestEventSystem::TestEventSystem(Gameexe& gexe)
    : EventSystem(gexe), event_system_mock_(new EventSystemMockHandler) {}

void TestEventSystem::SetMockHandler(
    const std::shared_ptr<EventSystemMockHandler>& handler) {
  event_system_mock_ = handler;
}

void TestEventSystem::ExecuteEventSystem(RLMachine& machine) {}

bool TestEventSystem::ShiftPressed() const {
  return event_system_mock_->shiftPressed();
}

bool TestEventSystem::CtrlPressed() const {
  return event_system_mock_->ctrlPressed();
}

unsigned int TestEventSystem::GetTicks() const {
  return event_system_mock_->GetTicks();
}

void TestEventSystem::Wait(unsigned int milliseconds) const {
  // waiting is a noop.
}

Point TestEventSystem::GetCursorPos() {
  return Point(0, 0);
}

void TestEventSystem::GetCursorPos(Point& position,
                                   int& button1,
                                   int& button2) {}

void TestEventSystem::FlushMouseClicks() {}

unsigned int TestEventSystem::TimeOfLastMouseMove() {
  return 0;
}

void TestEventSystem::InjectMouseMovement(RLMachine& machine,
                                          const Point& loc) {}

void TestEventSystem::InjectMouseDown(RLMachine& machine) {}

void TestEventSystem::InjectMouseUp(RLMachine& machine) {}
