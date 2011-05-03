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

#include "TestSystem/TestEventSystem.hpp"

// -----------------------------------------------------------------------
// TestEventSystem
// -----------------------------------------------------------------------
TestEventSystem::TestEventSystem(Gameexe& gexe)
  : EventSystem(gexe),
    event_system_mock_(new EventSystemMockHandler) {
}

void TestEventSystem::setMockHandler(
  const boost::shared_ptr<EventSystemMockHandler>& handler) {
  event_system_mock_ = handler;
}

void TestEventSystem::executeEventSystem(RLMachine& machine) {
}

bool TestEventSystem::shiftPressed() const {
  return event_system_mock_->shiftPressed();
}

bool TestEventSystem::ctrlPressed() const {
  return event_system_mock_->ctrlPressed();
}

unsigned int TestEventSystem::getTicks() const {
  return event_system_mock_->getTicks();
}

void TestEventSystem::wait(unsigned int milliseconds) const {
  // waiting is a noop.
}
