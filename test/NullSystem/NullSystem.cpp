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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// -----------------------------------------------------------------------

#include "NullSystem.hpp"
#include "testUtils.hpp"
#include <iostream>

#include "Systems/Base/SystemError.hpp"

using namespace std;

// -----------------------------------------------------------------------
// NullSystem
// -----------------------------------------------------------------------
NullSystem::NullSystem(const std::string& path_to_gameexe)
  : gameexe_(path_to_gameexe),
    null_graphics_system(*this, gameexe_),
    null_event_system(gameexe_),
    null_text_system(*this, gameexe_),
    null_sound_system(*this
)
{}

// -----------------------------------------------------------------------

NullSystem::NullSystem()
  : gameexe_(),
    null_graphics_system(*this, gameexe_),
    null_event_system(gameexe_),
    null_text_system(*this, gameexe_),
    null_sound_system(*this)
{
  gameexe_("__GAMEPATH") = locateTestCase("Gameroot") + "/";
  gameexe_("FOLDNAME.G00") = "G00";
}

// -----------------------------------------------------------------------

void NullSystem::run(RLMachine& machine) { /* do nothing */ }

GraphicsSystem& NullSystem::graphics() { return null_graphics_system; }
EventSystem& NullSystem::event() { return null_event_system; }
Gameexe& NullSystem::gameexe() { return gameexe_; }
TextSystem& NullSystem::text() { return null_text_system; }
SoundSystem& NullSystem::sound() { return null_sound_system; }
