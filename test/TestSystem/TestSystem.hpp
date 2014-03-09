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

#ifndef TEST_TESTSYSTEM_TESTSYSTEM_HPP_
#define TEST_TESTSYSTEM_TESTSYSTEM_HPP_

#include <string>

#include "Systems/Base/System.hpp"
#include "TestSystem/TestGraphicsSystem.hpp"
#include "TestSystem/TestEventSystem.hpp"
#include "TestSystem/TestTextSystem.hpp"
#include "TestSystem/TestSoundSystem.hpp"
#include "libreallive/gameexe.h"

// System subclass used for testing.
class TestSystem : public System {
 public:
  explicit TestSystem(const std::string& path_to_gameexe);
  TestSystem();

  // Implementation of System:
  virtual void run(RLMachine& machine);
  virtual TestGraphicsSystem& graphics();
  virtual EventSystem& event();
  virtual Gameexe& gameexe();
  virtual TextSystem& text();
  virtual SoundSystem& sound();

 private:
  Gameexe gameexe_;

  TestGraphicsSystem null_graphics_system;
  TestEventSystem null_event_system;
  TestTextSystem null_text_system;
  TestSoundSystem null_sound_system;
};

#endif  // TEST_TESTSYSTEM_TESTSYSTEM_HPP_
