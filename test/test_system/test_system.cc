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

#include "test_system/test_system.h"

#include <string>

#include "systems/base/system_error.h"
#include "test_utils.h"

// -----------------------------------------------------------------------
// TestSystem
// -----------------------------------------------------------------------
TestSystem::TestSystem(const std::string& path_to_gameexe)
    : gameexe_(path_to_gameexe),
      null_graphics_system(*this, gameexe_),
      null_event_system(gameexe_),
      null_text_system(*this, gameexe_),
      null_sound_system(*this) {}

TestSystem::TestSystem()
    : gameexe_(),
      null_graphics_system(*this, gameexe_),
      null_event_system(gameexe_),
      null_text_system(*this, gameexe_),
      null_sound_system(*this) {
  gameexe_("__GAMEPATH") = locateTestCase("Gameroot") + "/";
  gameexe_("FOLDNAME.G00") = "G00";
}

TestSystem::~TestSystem() {}

void TestSystem::Run(RLMachine& machine) {}

TestGraphicsSystem& TestSystem::graphics() { return null_graphics_system; }
EventSystem& TestSystem::event() { return null_event_system; }
Gameexe& TestSystem::gameexe() { return gameexe_; }
TextSystem& TestSystem::text() { return null_text_system; }
SoundSystem& TestSystem::sound() { return null_sound_system; }
