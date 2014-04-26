// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006, 2007 Elliot Glaysher
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

#include "gtest/gtest.h"

#include "libreallive/archive.h"
#include "libreallive/intmemref.h"
#include "machine/rlmachine.h"
#include "modules/module_jmp.h"
#include "modules/module_sys.h"
#include "test_system/test_system.h"

#include "test_utils.h"

#include <iostream>
using namespace std;

using libreallive::IntMemRef;

// Tests SceneNum, which should return the current scene number. Calls
// a SEEN.TXT made out of three SEENS, which just call SceneNum() and
// jump to the next scenario.
TEST(LargeModuleSysTest, SceneNum) {
  libreallive::Archive arc(locateTestCase("Module_Sys_SEEN/SceneNum.TXT"));
  TestSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new SysModule);
  rlmachine.AttachModule(new JmpModule);
  rlmachine.ExecuteUntilHalted();

  int values[3];
  for (int i = 0; i < 3; ++i)
    values[i] = rlmachine.GetIntValue(IntMemRef(0, i));

  EXPECT_EQ(1, values[0]) << "SceneNum::SEEN0001 didn't set value";
  EXPECT_EQ(248, values[1]) << "SceneNum::SEEN0248 didn't set value";
  EXPECT_EQ(639, values[2]) << "SceneNum::SEEN0639 didn't set value";
}

// Tests integer built-in functions.
TEST(LargeModuleSysTest, BuiltIns) {
  libreallive::Archive arc(locateTestCase("Module_Sys_SEEN/builtins.TXT"));
  TestSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new SysModule);
  rlmachine.ExecuteUntilHalted();

  int values[6];
  for (int i = 0; i < sizeof(values) / sizeof(values[0]); ++i)
    values[i] = rlmachine.GetIntValue(IntMemRef('A', i));

  // 5.6.1  Integer built-ins
  EXPECT_EQ(0, values[0]) << "Incorrect value for intA[0]";
  EXPECT_TRUE(values[1] == 0 || values[1] == -1)
      << "Incorrect value for intA[1]";
  EXPECT_EQ(-1, values[2]) << "Incorrect value for intA[2]";
  EXPECT_TRUE(-10 <= values[3] && values[3] < 0)
      << "Incorrect value for intA[3]";
  EXPECT_TRUE(-10 <= values[4] && values[4] <= 10)
      << "Incorrect value for intA[4]";
  EXPECT_TRUE(-10 <= values[5] && values[5] <= 10)
      << "Incorrect value for intA[5]";
}
