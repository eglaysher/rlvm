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
#include "modules/module_mem.h"

#include "test_system/test_system.h"

#include "test_utils.h"

#include <iostream>
using namespace std;
using libreallive::IntMemRef;

// Tests setarray_0.
//
// Corresponding kepago listing:
//
//   intA[3] = -1
//   setarray(intA[0], 1, 2, 3)
//
TEST(LargeMemTest, setarray_0) {
  libreallive::Archive arc(locateTestCase("Module_Mem_SEEN/setarray_0.TXT"));
  TestSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new MemModule);
  rlmachine.ExecuteUntilHalted();

  EXPECT_EQ(1, rlmachine.GetIntValue(IntMemRef('A', 0)))
      << "setarray returned wrong value for intA[0]";
  EXPECT_EQ(2, rlmachine.GetIntValue(IntMemRef('A', 1)))
      << "setarray returned wrong value for intA[1]";
  EXPECT_EQ(3, rlmachine.GetIntValue(IntMemRef('A', 2)))
      << "setarray returned wrong value for intA[2]";
  EXPECT_EQ(-1, rlmachine.GetIntValue(IntMemRef('A', 3)))
      << "setarray touched the value in intA[3]!!";
}

// Tests setrng_0.
//
// Corresponding kepago listing:
//
//   intA[4] = -1
//   intA[0] = -1
//   setrng(intA[0], intA[3])
TEST(LargeMemTest, setrng_0) {
  libreallive::Archive arc(locateTestCase("Module_Mem_SEEN/setrng_0.TXT"));
  TestSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new MemModule);
  rlmachine.ExecuteUntilHalted();

  EXPECT_EQ(0, rlmachine.GetIntValue(IntMemRef('A', 0)))
      << "setrng returned wrong value for intA[0]";
  EXPECT_EQ(0, rlmachine.GetIntValue(IntMemRef('A', 1)))
      << "setrng returned wrong value for intA[1]";
  EXPECT_EQ(0, rlmachine.GetIntValue(IntMemRef('A', 2)))
      << "setrng returned wrong value for intA[2]";
  EXPECT_EQ(0, rlmachine.GetIntValue(IntMemRef('A', 3)))
      << "setrng returned wrong value for intA[3]";
  EXPECT_EQ(-1, rlmachine.GetIntValue(IntMemRef('A', 4)))
      << "setrng touched the value in intA[4]!!!";
}

// Tests setrng_1.
//
//   intA[4] = -1
//   intA[0] = -1
//   setrng(intA[0], intA[3], 4)
TEST(LargeMemTest, setrng_1) {
  libreallive::Archive arc(locateTestCase("Module_Mem_SEEN/setrng_1.TXT"));
  TestSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new MemModule);
  rlmachine.ExecuteUntilHalted();

  EXPECT_EQ(4, rlmachine.GetIntValue(IntMemRef('A', 0)))
      << "setrng returned wrong value for intA[0]";
  EXPECT_EQ(4, rlmachine.GetIntValue(IntMemRef('A', 1)))
      << "setrng returned wrong value for intA[1]";
  EXPECT_EQ(4, rlmachine.GetIntValue(IntMemRef('A', 2)))
      << "setrng returned wrong value for intA[2]";
  EXPECT_EQ(4, rlmachine.GetIntValue(IntMemRef('A', 3)))
      << "setrng returned wrong value for intA[3]";
  EXPECT_EQ(-1, rlmachine.GetIntValue(IntMemRef('A', 4)))
      << "setrng touched the value in intA[4]!!!";
}

// Tests cpyrng_0.
TEST(LargeMemTest, cpyrng_0) {
  libreallive::Archive arc(locateTestCase("Module_Mem_SEEN/cpyrng_0.TXT"));
  TestSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new MemModule);
  rlmachine.ExecuteUntilHalted();

  // First make sure setarray did what we expected it to...
  EXPECT_EQ(1, rlmachine.GetIntValue(IntMemRef('A', 0)))
      << "setarray returned wrong value for intA[0]";
  EXPECT_EQ(2, rlmachine.GetIntValue(IntMemRef('A', 1)))
      << "setarray returned wrong value for intA[1]";
  EXPECT_EQ(3, rlmachine.GetIntValue(IntMemRef('A', 2)))
      << "setarray returned wrong value for intA[2]";

  // Now make sure cpyrng did its job.
  EXPECT_EQ(1, rlmachine.GetIntValue(IntMemRef('B', 0)))
      << "cpyrng returned wrong value for intB[0]";
  EXPECT_EQ(2, rlmachine.GetIntValue(IntMemRef('B', 1)))
      << "cpyrng returned wrong value for intB[1]";
  EXPECT_EQ(3, rlmachine.GetIntValue(IntMemRef('B', 2)))
      << "cpyrng returned wrong value for intB[2]";
}

// Tests setarray_stepped_0.
//
//   setrng(intA[0], intA[5], -1)
//   setarray_stepped(intA[0], 2, 1, 2, 3)
//
TEST(LargeMemTest, setarray_stepped_0) {
  libreallive::Archive arc(
      locateTestCase("Module_Mem_SEEN/setarray_stepped_0.TXT"));
  TestSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new MemModule);
  rlmachine.ExecuteUntilHalted();

  // First make sure setarray_stepped did what we expected it to, and that it
  // didn't overwrite the work of setrng
  EXPECT_EQ(1, rlmachine.GetIntValue(IntMemRef('A', 0)))
      << "setarray_stepped returned wrong value for intA[0]";
  EXPECT_EQ(-1, rlmachine.GetIntValue(IntMemRef('A', 1)))
      << "setarray_stepped touched the value of intA[1]";
  EXPECT_EQ(2, rlmachine.GetIntValue(IntMemRef('A', 2)))
      << "setarray_stepped returned wrong value for intA[2]";
  EXPECT_EQ(-1, rlmachine.GetIntValue(IntMemRef('A', 3)))
      << "setarray_stepped touched the value for intA[3]";
  EXPECT_EQ(3, rlmachine.GetIntValue(IntMemRef('A', 4)))
      << "setarray_stepped returned wrong value for intA[4]";
  EXPECT_EQ(-1, rlmachine.GetIntValue(IntMemRef('A', 5)))
      << "setarray_stepped touched the value for intA[5]";
}

// Tests setrng_stepped_0.
//
//   setrng(intA[0], intA[5], -1)
//   setrng_stepped(intA[0], 2, 3)
//
TEST(LargeMemTest, setrng_stepped_0) {
  libreallive::Archive arc(
      locateTestCase("Module_Mem_SEEN/setrng_stepped_0.TXT"));
  TestSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new MemModule);
  rlmachine.ExecuteUntilHalted();

  // First make sure setrng_stepped did what we expected it to, and that it
  // didn't overwrite the work of setrng
  EXPECT_EQ(0, rlmachine.GetIntValue(IntMemRef('A', 0)))
      << "setrng_stepped returned wrong value for intA[0]";
  EXPECT_EQ(-1, rlmachine.GetIntValue(IntMemRef('A', 1)))
      << "setrng_stepped touched the value of intA[1]";
  EXPECT_EQ(0, rlmachine.GetIntValue(IntMemRef('A', 2)))
      << "setrng_stepped returned wrong value for intA[2]";
  EXPECT_EQ(-1, rlmachine.GetIntValue(IntMemRef('A', 3)))
      << "setrng_stepped touched the value for intA[3]";
  EXPECT_EQ(0, rlmachine.GetIntValue(IntMemRef('A', 4)))
      << "setrng_stepped returned wrong value for intA[4]";
  EXPECT_EQ(-1, rlmachine.GetIntValue(IntMemRef('A', 5)))
      << "setrng_stepped touched the value for intA[5]";
}

// Tests setrng_stepped_1.
//
//   setrng(intA[0], intA[5], -1)
//   setrng_stepped(intA[0], 2, 3, 5)
//
TEST(LargeMemTest, setrng_stepped_1) {
  libreallive::Archive arc(
      locateTestCase("Module_Mem_SEEN/setrng_stepped_1.TXT"));
  TestSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new MemModule);
  rlmachine.ExecuteUntilHalted();

  // First make sure setrng_stepped did what we expected it to, and that it
  // didn't overwrite the work of setrng
  EXPECT_EQ(5, rlmachine.GetIntValue(IntMemRef('A', 0)))
      << "setrng_stepped returned wrong value for intA[0]";
  EXPECT_EQ(-1, rlmachine.GetIntValue(IntMemRef('A', 1)))
      << "setrng_stepped touched the value of intA[1]";
  EXPECT_EQ(5, rlmachine.GetIntValue(IntMemRef('A', 2)))
      << "setrng_stepped returned wrong value for intA[2]";
  EXPECT_EQ(-1, rlmachine.GetIntValue(IntMemRef('A', 3)))
      << "setrng_stepped touched the value for intA[3]";
  EXPECT_EQ(5, rlmachine.GetIntValue(IntMemRef('A', 4)))
      << "setrng_stepped returned wrong value for intA[4]";
  EXPECT_EQ(-1, rlmachine.GetIntValue(IntMemRef('A', 5)))
      << "setrng_stepped touched the value for intA[5]";
}

// Tests cpyvars.
//
//   intB[3] = 5
//   intB[5] = 1
//   intB[8] = 2
//   cpyvars(intA[0], 2, intB[1], intB[3], intB[6])
TEST(LargeMemTest, cpyvars) {
  libreallive::Archive arc(locateTestCase("Module_Mem_SEEN/cpyvars_0.TXT"));
  TestSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new MemModule);
  rlmachine.ExecuteUntilHalted();

  // First make sure cpyvars did what we expected it to...
  EXPECT_EQ(5, rlmachine.GetIntValue(IntMemRef('A', 0)))
      << "cpyvars set wrong value for intA[0]";
  EXPECT_EQ(1, rlmachine.GetIntValue(IntMemRef('A', 1)))
      << "cpyvars set wrong value for intA[1]";
  EXPECT_EQ(2, rlmachine.GetIntValue(IntMemRef('A', 2)))
      << "cpyvars set wrong value for intA[2]";
}

// Tests sum.
//
//   intA[0] = 0
//   intA[1] = 1
//   intA[2] = 2
//   intA[3] = 3
//   intA[10] = sum(intA[0], intA[3])
//
TEST(LargeMemTest, sum_0) {
  libreallive::Archive arc(locateTestCase("Module_Mem_SEEN/sum_0.TXT"));
  TestSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new MemModule);
  rlmachine.ExecuteUntilHalted();

  // First make sure sum did what we expected it to...
  EXPECT_EQ(6, rlmachine.GetIntValue(IntMemRef('A', 10)))
      << "sum returned the wrong value for intA[10]";
}
