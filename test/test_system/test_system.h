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

#ifndef TEST_TEST_SYSTEM_TEST_SYSTEM_H_
#define TEST_TEST_SYSTEM_TEST_SYSTEM_H_

#include <string>

#include "libreallive/gameexe.h"
#include "systems/base/system.h"
#include "test_system/test_event_system.h"
#include "test_system/test_graphics_system.h"
#include "test_system/test_sound_system.h"
#include "test_system/test_text_system.h"

// System subclass used for testing.
class TestSystem : public System {
 public:
  explicit TestSystem(const std::string& path_to_gameexe);
  TestSystem();
  virtual ~TestSystem();

  // Implementation of System:
  virtual void Run(RLMachine& machine) override;
  virtual TestGraphicsSystem& graphics() override;
  virtual EventSystem& event() override;
  virtual Gameexe& gameexe() override;
  virtual TextSystem& text() override;
  virtual SoundSystem& sound() override;

 private:
  Gameexe gameexe_;

  TestGraphicsSystem null_graphics_system;
  TestEventSystem null_event_system;
  TestTextSystem null_text_system;
  TestSoundSystem null_sound_system;
};

#endif  // TEST_TEST_SYSTEM_TEST_SYSTEM_H_
