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
#include "modules/module_msg.h"
#include "modules/module_str.h"
#include "test_system/test_system.h"

#include "test_utils.h"

#include <iostream>
using namespace std;
using namespace libreallive;

// Tests goto.
//
// Corresponding kepago listing:
//
//   intA[0] = 1;
//   goto @aa
//   intA[1] = 1;
//   @aa
//   intA[2] = 1;
TEST(LargeJmpTest, goto) {
  libreallive::Archive arc(locateTestCase("Module_Jmp_SEEN/goto_0.TXT"));
  TestSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new JmpModule);
  rlmachine.ExecuteUntilHalted();

  EXPECT_EQ(1, rlmachine.GetIntValue(IntMemRef('A', 0)))
      << "Didn't set precondition (!?!?)";
  EXPECT_EQ(0, rlmachine.GetIntValue(IntMemRef('A', 1)))
      << "Set intA[1]; this means the goto was ignored!";
  EXPECT_EQ(1, rlmachine.GetIntValue(IntMemRef('A', 0)))
      << "Didn't set postcondition (!?!?)";
}

// -----------------------------------------------------------------------

// Tests goto_if (if false)
//
// Corresponding kepago listing:
//
//   // intB[0] may or maynot be set by the calling program.
//   intA[0] = 1
//   goto_if (intB[0]) @aa
//   intA[1] = 1
//   @aa
//   intA[2] = 1
TEST(LargeJmpTest, goto_if_false) {
  libreallive::Archive arc(locateTestCase("Module_Jmp_SEEN/goto_if_0.TXT"));
  TestSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new JmpModule);
  rlmachine.ExecuteUntilHalted();

  EXPECT_EQ(1, rlmachine.GetIntValue(IntMemRef('A', 0)))
      << "Didn't set precondition (!?!?)";
  EXPECT_EQ(1, rlmachine.GetIntValue(IntMemRef('A', 1)))
      << "Didn't set intA[1]; goto-ed when we had a false value!";
  EXPECT_EQ(1, rlmachine.GetIntValue(IntMemRef('A', 0)))
      << "Didn't set postcondition (!?!?)";
}

// -----------------------------------------------------------------------

// Tests goto_if (if true)
//
// Corresponding kepago listing:
//
//   // intB[0] may or maynot be set by the calling program.
//   intA[0] = 1
//   goto_if (intB[0]) @aa
//   intA[1] = 1
//   @aa
//   intA[2] = 1
TEST(LargeJmpTest, goto_if_true) {
  libreallive::Archive arc(locateTestCase("Module_Jmp_SEEN/goto_if_0.TXT"));
  TestSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new JmpModule);
  rlmachine.SetIntValue(IntMemRef('B', 0), 1);

  rlmachine.ExecuteUntilHalted();

  EXPECT_EQ(1, rlmachine.GetIntValue(IntMemRef('A', 0)))
      << "Didn't set precondition (!?!?)";
  EXPECT_EQ(0, rlmachine.GetIntValue(IntMemRef('A', 1)))
      << "Set intA[1]; failed to goto when we had a true value!";
  EXPECT_EQ(1, rlmachine.GetIntValue(IntMemRef('A', 0)))
      << "Didn't set postcondition (!?!?)";
}

// -----------------------------------------------------------------------

// Tests goto_unless (if false)
//
// Corresponding kepago listing:
//
//   // intB[0] may or maynot be set by the calling program.
//   intA[0] = 1
//   goto_unless(intB[0]) @aa
//   intA[1] = 1
//   @aa
//   intA[2] = 1
TEST(LargeJmpTest, goto_unless_false) {
  libreallive::Archive arc(locateTestCase("Module_Jmp_SEEN/goto_unless_0.TXT"));
  TestSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new JmpModule);

  rlmachine.ExecuteUntilHalted();

  EXPECT_EQ(1, rlmachine.GetIntValue(IntMemRef('A', 0)))
      << "Didn't set precondition (!?!?)";
  EXPECT_EQ(0, rlmachine.GetIntValue(IntMemRef('A', 1)))
      << "Set intA[1]; this means that we didn't goto but we had a false val";
  EXPECT_EQ(1, rlmachine.GetIntValue(IntMemRef('A', 0)))
      << "Didn't set postcondition (!?!?)";
}

// -----------------------------------------------------------------------

// Tests goto_on (table jumps)
//
// Corresponding kepago listing:
//
//   goto_on(intB[0]) { @a , @b , @c }
//   intA[0] = -1
//   goto @end
//
//   @a
//   intA[0] = 0
//   goto @end
//
//   @b
//   intA[0] = 1
//   goto @end
//
//   @c
//   intA[0] = 2
//   goto @end
//
//   @end
TEST(LargeJmpTest, goto_on) {
  // Repeat this test to hit each label
  for (int i = 0; i < 3; ++i) {
    libreallive::Archive arc(locateTestCase("Module_Jmp_SEEN/goto_on_0.TXT"));
    TestSystem system;
    RLMachine rlmachine(system, arc);
    rlmachine.AttachModule(new JmpModule);
    rlmachine.SetIntValue(IntMemRef('B', 0), i);
    rlmachine.ExecuteUntilHalted();

    EXPECT_EQ(i, rlmachine.GetIntValue(IntMemRef('A', 0)))
        << "Wrong destination on goto_on";
  }
}

// -----------------------------------------------------------------------

// Tests goto_on (with an illegal value)
//
// Corresponding kepago listing:
//
//   goto_on(intB[0]) { @a , @b , @c }
//   intA[0] = -1
//   goto @end
//
//   @a
//   intA[0] = 0
//   goto @end
//
//   @b
//   intA[0] = 1
//   goto @end
//
//   @c
//   intA[0] = 2
//   goto @end
//
//   @end
TEST(LargeJmpTest, goto_on_illegal) {
  libreallive::Archive arc(locateTestCase("Module_Jmp_SEEN/goto_on_0.TXT"));
  TestSystem system(locateTestCase("Gameexe_data/Gameexe.ini"));
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new JmpModule);
  rlmachine.SetIntValue(IntMemRef('B', 0), 7);
  rlmachine.ExecuteUntilHalted();

  EXPECT_EQ(-1, rlmachine.GetIntValue(IntMemRef('A', 0)))
      << "We jumped somewhere unexpected on a bad value!";
}

// -----------------------------------------------------------------------

// Tests goto_case
//
// Corresponding kepago listing:
//
//   goto_case(intB[0]) { 0: @a; 1: @b; 2: @c; _: @default }
//
//   @a
//   intA[0] = 0
//   goto @end
//
//   @b
//   intA[0] = 1
//   goto @end
//
//   @c
//   intA[0] = 2
//   goto @end
//
//   @default
//   intA[0] = 3
//
//   @end
TEST(LargeJmpTest, goto_case) {
  for (int i = 0; i < 3; ++i) {
    libreallive::Archive arc(locateTestCase("Module_Jmp_SEEN/goto_case_0.TXT"));
    TestSystem system;
    RLMachine rlmachine(system, arc);
    rlmachine.AttachModule(new JmpModule);
    rlmachine.SetIntValue(IntMemRef('B', 0), i);
    rlmachine.ExecuteUntilHalted();

    EXPECT_EQ(i, rlmachine.GetIntValue(IntMemRef('A', 0)))
        << "We jumped somewhere unexpected on a bad value!";
  }

  // Now test the default value
  libreallive::Archive arc(locateTestCase("Module_Jmp_SEEN/goto_case_0.TXT"));
  TestSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new JmpModule);
  rlmachine.SetIntValue(IntMemRef('B', 0), 29);
  rlmachine.ExecuteUntilHalted();

  EXPECT_EQ(3, rlmachine.GetIntValue(IntMemRef(0, 0)))
      << "We jumped somewhere unexpected on a bad value!";
}

// -----------------------------------------------------------------------

// Tests gosub
//
// Corresponding kepago listing:
//
//   intA[0] = 1
//   gosub @sub
//   intA[2] = 1
//   goto @end
//
//   @sub
//   intA[1] = 1
//   ret()
//
//   // If we fall through, reset the flag
//   intA[1] = 0
//   @end
TEST(LargeJmpTest, gosub_0) {
  libreallive::Archive arc(locateTestCase("Module_Jmp_SEEN/gosub_0.TXT"));
  TestSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new JmpModule);
  rlmachine.ExecuteUntilHalted();

  EXPECT_EQ(1, rlmachine.GetIntValue(IntMemRef('A', 0)))
      << "Didn't set precondition (!?!?)";
  EXPECT_EQ(1, rlmachine.GetIntValue(IntMemRef('A', 1)))
      << "intA[1] is unset; this means the gosub or ret was ignored!";
  EXPECT_EQ(1, rlmachine.GetIntValue(IntMemRef('A', 0)))
      << "Didn't set postcondition (!?!?)";
}

// -----------------------------------------------------------------------

// Tests gosub_if (if false)
TEST(LargeJmpTest, gosub_if_false) {
  libreallive::Archive arc(locateTestCase("Module_Jmp_SEEN/gosub_if_0.TXT"));
  TestSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new JmpModule);
  rlmachine.ExecuteUntilHalted();

  EXPECT_EQ(1, rlmachine.GetIntValue(IntMemRef('A', 0)))
      << "Didn't set precondition (!?!?)";
  EXPECT_EQ(0, rlmachine.GetIntValue(IntMemRef('A', 1)))
      << "Set intA[1]; this means we gosubed on a false value (or ret problem)";
  EXPECT_EQ(1, rlmachine.GetIntValue(IntMemRef('A', 0)))
      << "Didn't set postcondition (!?!?)";
}

// -----------------------------------------------------------------------

// Tests gosub_if (if true)
TEST(LargeJmpTest, gosub_if_true) {
  libreallive::Archive arc(locateTestCase("Module_Jmp_SEEN/gosub_if_0.TXT"));
  TestSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new JmpModule);
  rlmachine.SetIntValue(IntMemRef('B', 0), 1);

  rlmachine.ExecuteUntilHalted();

  EXPECT_EQ(1, rlmachine.GetIntValue(IntMemRef('A', 0)))
      << "Didn't set precondition (!?!?)";
  EXPECT_EQ(1, rlmachine.GetIntValue(IntMemRef('A', 1)))
      << "Didn't set intA[1]; didn't gosub on true!";
  EXPECT_EQ(1, rlmachine.GetIntValue(IntMemRef('A', 0)))
      << "Didn't set postcondition (!?!?)";
}

// -----------------------------------------------------------------------

// Tests gosub_unless (if false)
//
// Corresponding kepago listing:
//
//   intA[0] = 1
//   gosub_unless(intB[0]) @sub
//   intA[2] = 1
//   goto @end
//
//   @sub
//   intA[1] = 1
//   ret()
//
//   // If we fall through, reset the flag
//   intA[1] = 0
//   @end
TEST(LargeJmpTest, gosub_unless_false) {
  libreallive::Archive arc(
      locateTestCase("Module_Jmp_SEEN/gosub_unless_0.TXT"));
  TestSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new JmpModule);
  rlmachine.ExecuteUntilHalted();

  EXPECT_EQ(1, rlmachine.GetIntValue(IntMemRef('A', 0)))
      << "Didn't set precondition (!?!?)";
  EXPECT_EQ(1, rlmachine.GetIntValue(IntMemRef('A', 1)))
      << "Didn't set intA[1]; didn't gosub on false!";
  EXPECT_EQ(1, rlmachine.GetIntValue(IntMemRef('A', 0)))
      << "Didn't set postcondition (!?!?)";
}

// -----------------------------------------------------------------------

// Tests gosub_unless (if true)
//
// Corresponding kepago listing:
//
//   intA[0] = 1
//   gosub_unless(intB[0]) @sub
//   intA[2] = 1
//   goto @end
//
//   @sub
//   intA[1] = 1
//   ret()
//
//   // If we fall through, reset the flag
//   intA[1] = 0
//   @end
TEST(LargeJmpTest, gosub_unless_true) {
  libreallive::Archive arc(
      locateTestCase("Module_Jmp_SEEN/gosub_unless_0.TXT"));
  TestSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new JmpModule);
  rlmachine.SetIntValue(IntMemRef('B', 0), 1);

  rlmachine.ExecuteUntilHalted();

  EXPECT_EQ(1, rlmachine.GetIntValue(IntMemRef('A', 0)))
      << "Didn't set precondition (!?!?)";
  EXPECT_EQ(0, rlmachine.GetIntValue(IntMemRef('A', 1)))
      << "Set intA[1]; this means we gosubed on a true value (or ret problem)";
  EXPECT_EQ(1, rlmachine.GetIntValue(IntMemRef('A', 0)))
      << "Didn't set postcondition (!?!?)";
}

// -----------------------------------------------------------------------

// Tests gosub_case
//
// Corresponding kepago listing:
//
//   goto_case(intB[0]) { 0: @a; 1: @b; 2: @c; _: @default }
//
//   @a
//   intA[0] = 0
//   goto @end
//
//   @b
//   intA[0] = 1
//   goto @end
//
//   @c
//   intA[0] = 2
//   goto @end
//
//   @default
//   intA[0] = 3
//
//   @end
TEST(LargeJmpTest, gosub_case) {
  for (int i = 0; i < 3; ++i) {
    libreallive::Archive arc(
        locateTestCase("Module_Jmp_SEEN/gosub_case_0.TXT"));
    TestSystem system(locateTestCase("Gameexe_data/Gameexe.ini"));
    RLMachine rlmachine(system, arc);
    rlmachine.AttachModule(new JmpModule);
    rlmachine.SetIntValue(IntMemRef('B', 0), i);
    rlmachine.ExecuteUntilHalted();

    EXPECT_EQ(i, rlmachine.GetIntValue(IntMemRef('A', 0)))
        << "We jumped somewhere unexpected on a bad value!";
  }

  // Now test the default value
  libreallive::Archive arc(locateTestCase("Module_Jmp_SEEN/gosub_case_0.TXT"));
  TestSystem system(locateTestCase("Gameexe_data/Gameexe.ini"));
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new JmpModule);
  rlmachine.SetIntValue(IntMemRef('B', 0), 29);
  rlmachine.ExecuteUntilHalted();

  EXPECT_EQ(3, rlmachine.GetIntValue(IntMemRef('A', 0)))
      << "We jumped somewhere unexpected on a bad value!";
}

// -----------------------------------------------------------------------

// Tests jump (within current file). In this test, we use entrypoints
// like goto labels.
//
// Corresponding kepago listing:
//
//   jump(1, intB[0]);
//
//   #ENTRYPOINT 1
//   intA[0] = 1
//   goto @end
//
//   #ENTRYPOINT 2
//   intA[0] = 2
//   goto @end
//
//   #ENTRYPOINT 3
//   intA[0] = 3
//   goto @end
//
//   @end
TEST(LargeJmpTest, jump) {
  for (int i = 1; i < 4; ++i) {
    libreallive::Archive arc(locateTestCase("Module_Jmp_SEEN/jump_0.TXT"));
    TestSystem system;
    RLMachine rlmachine(system, arc);
    rlmachine.AttachModule(new JmpModule);
    rlmachine.SetIntValue(IntMemRef('B', 0), i);
    rlmachine.ExecuteUntilHalted();

    EXPECT_EQ(i, rlmachine.GetIntValue(IntMemRef('A', 0)))
        << "We jumped somewhere unexpected on a bad value!";
  }
}

// -----------------------------------------------------------------------

// Tests jumping to different entrypoints.
//
// Corresponding kepago listing for SEEN00001:
//
//   jump(2, intB[0])
//
// Corresponding kepago listing for SEEN00002:
//
//   #ENTRYPOINT 1
//   intA[0] = 1
//   goto @end
//
//   #ENTRYPOINT 2
//   intA[0] = 2
//   goto @end
//
//   #ENTRYPOINT 3
//   intA[0] = 3
//   goto @end
//
//   @end
TEST(LargeJmpTest, jumpTest) {
  for (int i = 1; i < 4; ++i) {
    libreallive::Archive arc(locateTestCase("Module_Jmp_SEEN/jumpTest.TXT"));
    TestSystem system;
    RLMachine rlmachine(system, arc);
    rlmachine.AttachModule(new JmpModule);
    rlmachine.SetIntValue(IntMemRef('B', 0), i);
    rlmachine.ExecuteUntilHalted();

    EXPECT_EQ(i, rlmachine.GetIntValue(IntMemRef('A', 0)))
        << "We jumped somewhere unexpected on a bad value!";
  }
}

// -----------------------------------------------------------------------

// Tests farcall:
//
// Corresponding kepago listing for SEEN00001:
//
//   intA[0] = 1
//   farcall(2, intB[0])
//   intA[2] = 1
//
// Corresponding kepago listing for SEEN00002:
//
//   #ENTRYPOINT 1
//   intA[1] = 1
//   ret
//
//   #ENTRYPOINT 2
//   intA[1] = 2
//   ret
//
//   #ENTRYPOINT 3
//   intA[1] = 3
//   ret
TEST(LargeJmpTest, farcall) {
  for (int i = 1; i < 4; ++i) {
    libreallive::Archive arc(
        locateTestCase("Module_Jmp_SEEN/farcallTest_0.TXT"));
    TestSystem system;
    RLMachine rlmachine(system, arc);
    rlmachine.AttachModule(new JmpModule);
    rlmachine.SetIntValue(IntMemRef('B', 0), i);
    rlmachine.ExecuteUntilHalted();

    EXPECT_EQ(1, rlmachine.GetIntValue(IntMemRef('A', 0)))
        << "Precondition not set! (!?!?!?!)";
    EXPECT_EQ(i, rlmachine.GetIntValue(IntMemRef('A', 1)))
        << "We jumped somewhere unexpected on a bad value!";
    EXPECT_EQ(1, rlmachine.GetIntValue(IntMemRef('A', 2)))
        << "Postcondition not set! (We didn't return correctly!)";
  }
}

// -----------------------------------------------------------------------

// Tests gosub_with
//
// Corresponding kepago listing:
//
//   #version 1.3
//
//   intB[0] = 1
//   intB[1] = 2
//   strS[0] = "one"
//   strS[1] = "two"
//
//   intA[0] = gosub_with(intB[0], intB[1]) @intTest
//   intA[1] = gosub_with(strS[0], strS[1]) @stringTest
//   goto @end
//
//   @stringTest
//   strS[3] = strK[0] + strK[1]
//   ret_with(strlen(strS[3]))
//   goto @end
//
//   @intTest
//   intD[0] = intL[0] + intL[1]
//   ret_with(intD[0])
//   goto @end
//
//   @end
TEST(LargeJmpTest, gosub_with) {
  libreallive::Archive arc(locateTestCase("Module_Jmp_SEEN/gosub_with_0.TXT"));
  TestSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new JmpModule);
  rlmachine.AttachModule(new StrModule);
  rlmachine.ExecuteUntilHalted();

  // Original states that shouldn't be modified
  EXPECT_EQ(1, rlmachine.GetIntValue(IntMemRef('B', 0)))
      << "Precondition not set! (!?!?!?!) (intB[0])";
  EXPECT_EQ(2, rlmachine.GetIntValue(IntMemRef('B', 1)))
      << "Precondition not set! (!?!?!?!) (intB[1])";
  EXPECT_EQ("one", rlmachine.GetStringValue(STRS_LOCATION, 0))
      << "Precondition not set! (!?!?!?!) (strS[0])";
  EXPECT_EQ("two", rlmachine.GetStringValue(STRS_LOCATION, 1))
      << "Precondition not set! (!?!?!?!) (strS[1])";

  // Check the intermediate values that were calculated in the
  // functions
  EXPECT_EQ("onetwo", rlmachine.GetStringValue(STRS_LOCATION, 3))
      << "Wrong intermediary value for strS[3] in @stringTest!";
  EXPECT_EQ(3, rlmachine.GetIntValue(IntMemRef('D', 0)))
      << "Wrong intermediary value for intD[0] in @intTest!";

  // Make sure that intA[0] and intA[1] are set correctly when the
  // gosub_with returns
  EXPECT_EQ(3, rlmachine.GetIntValue(IntMemRef('A', 0)))
      << "Wrong final value for intA[0]!";
  EXPECT_EQ(6, rlmachine.GetIntValue(IntMemRef('A', 1)))
      << "Wrong final value for intA[1]!";
}

// -----------------------------------------------------------------------

// Recursive C implementation of fibonaci. Doesn't have all the weird
// stack maitanance of the Reallive one I wrote, so this should help
// test it.
static int recFib(int input) {
  if (input == 0)
    return 0;
  if (input == 1)
    return 1;
  return recFib(input - 1) + recFib(input - 2);
}

// -----------------------------------------------------------------------

// Tests the most complex test regarding goto_with and the stack:
// calculate fibonacci numbers.
//
// Corresponding kepago listing:
//
//   @code
//   #version 1.3
//
//   // Stack marker. Since Reallive provides no variable
//   // stack, we need to manually keep track of this.
//   intA[0] = 0
//
//   // intB[0] is the top of our stack.
//   intB[0] = 0
//
//   // Call fibonacci
//   intE[0] = gosub_with(intD[0]) @fib
//   goto @end
//
//   // -----------------------------------------------------
//
//   @fib
//   // Check the termination conditions
//   if intL[0] == 0 ret_with(0)
//   if intL[0] == 1 ret_with(1)
//
//   // Copy the incoming data onto the stack, and
//   // increment the pointer.
//   intB[intA[0]] = intL[0] - 1
//   intA[0] += 1
//   intB[intA[0]] = intL[0] - 2
//   intA[0] += 1
//
//   intB[intA[0] - 2] = gosub_with(intB[intA[0] - 2]) @fib
//   intB[intA[0] - 1] = gosub_with(intB[intA[0] - 1]) @fib
//
//   // Decrement the stack point
//   intA[0] -= 2
//   ret_with(intB[intA[0]] + intB[intA[0] + 1])
//
//   @end
TEST(LargeJmpTest, fibonacci) {
  for (int i = 0; i < 10; ++i) {
    libreallive::Archive arc(locateTestCase("Module_Jmp_SEEN/fibonacci.TXT"));
    TestSystem system;
    RLMachine rlmachine(system, arc);
    rlmachine.AttachModule(new JmpModule);
    rlmachine.AttachModule(new StrModule);
    rlmachine.SetIntValue(IntMemRef('D', 0), i);
    rlmachine.ExecuteUntilHalted();

    stringstream ss;
    ss << "Wrong output value for fib(" << i << ")";
    EXPECT_EQ(recFib(i), rlmachine.GetIntValue(IntMemRef('E', 0)))
        << ss.str().c_str();
  }
}

// -----------------------------------------------------------------------

// Tests farcall_with()/rtl_with().
//
// Corresponding kepago listing for SEEN00001:
//
//   #version 1.3
//
//   intA[0] = 1
//   intA[1] = farcall_with(2, intB[0], intB[1])
//   intA[2] = 1
//
// Corresponding kepago listing for SEEN00002:
//
//   #version 1.3
//
//   #ENTRYPOINT 1
//   rtl_with(1 + intL[0])
//
//   #ENTRYPOINT 2
//   rtl_with(2 + intL[0])
//
//   #ENTRYPOINT 3
//   rtl_with(3 + intL[0])
TEST(LargeJmpTest, farcall_with) {
  for (int offset = 0; offset < 2; ++offset) {
    for (int entrypoint = 1; entrypoint < 4; ++entrypoint) {
      libreallive::Archive arc(
          locateTestCase("Module_Jmp_SEEN/farcall_withTest.TXT"));
      TestSystem system;
      RLMachine rlmachine(system, arc);
      rlmachine.AttachModule(new JmpModule);
      rlmachine.AttachModule(new StrModule);
      rlmachine.SetIntValue(IntMemRef('B', 0), entrypoint);
      rlmachine.SetIntValue(IntMemRef('B', 1), offset);
      rlmachine.ExecuteUntilHalted();

      stringstream ss;
      ss << "Wrong output value for pair (" << offset << ", " << entrypoint
         << ") in farcall_with test!";
      EXPECT_EQ(entrypoint + offset, rlmachine.GetIntValue(IntMemRef('A', 1)))
          << ss.str().c_str();
    }
  }
}

// Tests the new pushStringValue that Little Busters depends on.
TEST(LargeJmpTest, pushStringValueUp) {
  libreallive::Archive arc(
      locateTestCase("Module_Jmp_SEEN/pushStringValueUp.TXT"));
  TestSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new JmpModule);
  rlmachine.AttachModule(new StrModule);
  rlmachine.ExecuteUntilHalted();

  EXPECT_EQ("GOOD", rlmachine.GetStringValue(STRM_LOCATION, 0));
}
