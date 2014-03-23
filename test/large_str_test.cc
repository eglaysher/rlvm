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

#include "modules/module_str.h"
#include "libreallive/archive.h"
#include "libreallive/intmemref.h"

#include "machine/rlmachine.h"

#include "test_system/test_system.h"

#include "test_utils.h"

#include <string>
#include <iostream>

using namespace std;
using namespace libreallive;

// Tests strcpy_0, which should copy the string valid int strS[0].
//
// Corresponding kepago listing:
//
//   strS[0] = "valid"
//
TEST(LargeModuleStrTest, strcpy0) {
  libreallive::Archive arc(locateTestCase("Module_Str_SEEN/strcpy_0.TXT"));
  TestSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new StrModule);
  rlmachine.ExecuteUntilHalted();

  string one = rlmachine.GetStringValue(STRS_LOCATION, 0);
  EXPECT_EQ("valid", one) << "strcpy_0 script failed to set value";
}

// Tests strcpy_1, which should copy the first 2 chracters into strS[0].
//
// Corresponding kepago listing:
//
//   strcpy(strS[0], "valid", 2)
//
TEST(LargeModuleStrTest, strcpy1) {
  libreallive::Archive arc(locateTestCase("Module_Str_SEEN/strcpy_1.TXT"));
  TestSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new StrModule);
  rlmachine.ExecuteUntilHalted();

  string one = rlmachine.GetStringValue(STRS_LOCATION, 0);
  EXPECT_EQ("va", one) << "strcpy_1 script failed to set value";
}

// Tests strclear_0, which should clear the value of a string.
//
// Corresponding kepago listing:
//
//   strS[0] = "valid"
//   strS[1] = "valid"
//   strclear(strS[0])
//
TEST(LargeModuleStrTest, strclear0) {
  libreallive::Archive arc(locateTestCase("Module_Str_SEEN/strclear_0.TXT"));
  TestSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new StrModule);
  rlmachine.ExecuteUntilHalted();

  string one = rlmachine.GetStringValue(STRS_LOCATION, 0);
  EXPECT_EQ("", one) << "strclear_0 script failed to unset value";

  // We include this check to make sure the machine is sane and that
  // the first assignment works, so strclear doesn't appear to work
  // because assignment failed.
  string two = rlmachine.GetStringValue(STRS_LOCATION, 1);
  EXPECT_EQ("valid", two) << "strclear_0 script failed to set value";
}

// Tests strclear_1, which should clear out a whole range of values.
//
//   strS[0] = "valid"
//   strS[1] = "valid"
//   strS[2] = "valid"
//   strclear(strS[0], strS[1])
TEST(LargeModuleStrTest, strclear1) {
  libreallive::Archive arc(locateTestCase("Module_Str_SEEN/strclear_1.TXT"));
  TestSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new StrModule);
  rlmachine.ExecuteUntilHalted();

  string one = rlmachine.GetStringValue(STRS_LOCATION, 0);
  EXPECT_EQ("", one) << "strclear_1 script failed to unset value";
  string two = rlmachine.GetStringValue(STRS_LOCATION, 1);
  EXPECT_EQ("", two) << "strclear_1 script failed to unset value";

  // We include this check to make sure the machine is sane and that
  // the first assignment works, so strclear doesn't appear to work
  // because assignment failed.
  string three = rlmachine.GetStringValue(STRS_LOCATION, 2);
  EXPECT_EQ("valid", three) << "strclear_1 script failed to set value";
}

// Tests strcat, which should end up with the string "valid" in strS[0].
//
// Corresponding kepago listing:
//
//   strS[0] = "va"
//   strS[0] += "lid"
//
TEST(LargeModuleStrTest, strcat) {  // NOLINT
  libreallive::Archive arc(locateTestCase("Module_Str_SEEN/strcat_0.TXT"));
  TestSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new StrModule);
  rlmachine.ExecuteUntilHalted();

  string one = rlmachine.GetStringValue(STRS_LOCATION, 0);
  EXPECT_EQ("valid", one) << "strcat script failed to set value";
}

// Tests strlen. Should be 5, for "vaild"
//
// Corresponding kepago listing:
//
//   strS[0] = "valid"
//   intA[0] = strlen(strS[0])
//
TEST(LargeModuleStrTest, strlen) {
  libreallive::Archive arc(locateTestCase("Module_Str_SEEN/strlen_0.TXT"));
  TestSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new StrModule);
  rlmachine.ExecuteUntilHalted();

  int one = rlmachine.GetIntValue(IntMemRef('A', 0));
  EXPECT_EQ(5, one) << "strlen script failed to set the strlen(\"valid\") to 5";
}

// Tests strcmp; make sure that it gives the same values as normal strcmp.
//
// Corresponding kepago listing:
//
//   strS[0] = "a"
//   strS[1] = "b"
//   strS[2] = "b"
//   strS[3] = "c"
//   intA[0] = strcmp(strS[0], strS[1])
//   intA[1] = strcmp(strS[1], strS[2])
//   intA[2] = strcmp(strS[2], strS[3])
//
TEST(LargeModuleStrTest, strcmp) {
  libreallive::Archive arc(locateTestCase("Module_Str_SEEN/strcmp_0.TXT"));
  TestSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new StrModule);
  rlmachine.ExecuteUntilHalted();

  EXPECT_EQ(strcmp("a", "b"), rlmachine.GetIntValue(IntMemRef('A', 0)))
      << "Different values for strcmp(\"a\", \"b\")";
  EXPECT_EQ(strcmp("b", "b"), rlmachine.GetIntValue(IntMemRef('A', 1)))
      << "Different values for strcmp(\"b\", \"b\")";
  EXPECT_EQ(strcmp("b", "c"), rlmachine.GetIntValue(IntMemRef('A', 2)))
      << "Different values for strcmp(\"b\", \"c\")";
}

// First strsub_0 test. Make sure it works on ASCII strings.
TEST(LargeModuleStrTest, strsub0_ascii) {
  libreallive::Archive arc(locateTestCase("Module_Str_SEEN/strsub_0.TXT"));
  TestSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new StrModule);
  rlmachine.ExecuteUntilHalted();

  EXPECT_EQ("lid", rlmachine.GetStringValue(STRS_LOCATION, 1))
      << "strsub returned wrong value";
}

// Second strsub_0 test. Make sure it works on Shift_JIS strings.
//
//   strS[0] = "わたしの名前、まだ覚えてる？"
//   strS[1] = strsub(strS[0], 7)
//
TEST(LargeModuleStrTest, strsub0_shiftjis) {
  libreallive::Archive arc(locateTestCase("Module_Str_SEEN/strsub_1.TXT"));
  TestSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new StrModule);
  rlmachine.ExecuteUntilHalted();

  EXPECT_EQ("\x82\xDC\x82\xBE\x8A\x6F\x82\xA6\x82\xC4\x82\xE9\x81\x48",
            rlmachine.GetStringValue(STRS_LOCATION, 1))
      << "strsub returned wrong value";
}

// First strsub_1 test. Make sure it deals with ASCII.
//
//   strS[0] = "valid"
//   strS[1] = strsub(strS[0], 1, 2)
//
TEST(LargeModuleStrTest, strsub1_ascii) {
  libreallive::Archive arc(locateTestCase("Module_Str_SEEN/strsub_2.TXT"));
  TestSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new StrModule);
  rlmachine.ExecuteUntilHalted();

  EXPECT_EQ("al", rlmachine.GetStringValue(STRS_LOCATION, 1))
      << "strsub returned wrong value";
}

// Second strsub_1 test. Make sure it works on Shift_JIS strings.
//
//  strS[0] = "わたしの名前、まだ覚えてる？"
//  strS[1] = strsub(strS[0], 7)
//
TEST(LargeModuleStrTest, strsub1_shiftjis) {
  libreallive::Archive arc(locateTestCase("Module_Str_SEEN/strsub_3.TXT"));
  TestSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new StrModule);
  rlmachine.ExecuteUntilHalted();

  EXPECT_EQ("\x96\xBC\x91\x4F", rlmachine.GetStringValue(STRS_LOCATION, 1))
      << "strsub returned wrong value";
}

// Strrsub test. (We don't need to test this on Shift_JIS since this
// calls the same code path as strsub, with just a different offset
// method.)
//
//   strS[0] = "valid"
//   strS[1] = strrsub(strS[0], 2)
//
TEST(LargeModuleStrTest, strrsub_0) {
  libreallive::Archive arc(locateTestCase("Module_Str_SEEN/strrsub_0.TXT"));
  TestSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new StrModule);
  rlmachine.ExecuteUntilHalted();

  EXPECT_EQ("id", rlmachine.GetStringValue(STRS_LOCATION, 1))
      << "strrsub returned wrong value";
}

// Strrsub test.
//
//  strS[0] = "valid"
//  strS[1] = strrsub(strS[0], 2, 1)
//
TEST(LargeModuleStrTest, strrsub_1) {
  libreallive::Archive arc(locateTestCase("Module_Str_SEEN/strrsub_1.TXT"));
  TestSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new StrModule);
  rlmachine.ExecuteUntilHalted();

  EXPECT_EQ("i", rlmachine.GetStringValue(STRS_LOCATION, 1))
      << "strrsub returned wrong value";
}

// strcharlen test on ASCII strings
//
//  strS[0] = "valid"
//  intA[0] = strcharlen(strS[0])
//
TEST(LargeModuleStrTest, strcharlen_ascii) {
  libreallive::Archive arc(locateTestCase("Module_Str_SEEN/strcharlen_0.TXT"));
  TestSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new StrModule);
  rlmachine.ExecuteUntilHalted();

  EXPECT_EQ(5, rlmachine.GetIntValue(IntMemRef('A', 0)))
      << "strcharlen returned wrong value";
}

// strcharlen test on Shift_JIS strings
//
//   strS[0] = "わたしの名前、まだ覚えてる？"
//   intA[0] = strcharlen(strS[0])
//
TEST(LargeModuleStrTest, srcharlen_shiftjis) {
  libreallive::Archive arc(locateTestCase("Module_Str_SEEN/strcharlen_1.TXT"));
  TestSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new StrModule);
  rlmachine.ExecuteUntilHalted();

  EXPECT_EQ(14, rlmachine.GetIntValue(IntMemRef('A', 0)))
      << "strcharlen returned wrong value";
}

// strtrunc test on ASCII characters
//
//   strS[0] = "valid"
//   strtrunc(strS[0], 2)
//
TEST(LargeModuleStrTest, strtrunc_ascii) {
  libreallive::Archive arc(locateTestCase("Module_Str_SEEN/strtrunc_0.TXT"));
  TestSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new StrModule);
  rlmachine.ExecuteUntilHalted();

  EXPECT_EQ("va", rlmachine.GetStringValue(STRS_LOCATION, 0))
      << "strtrunc returned wrong value";
}

// strtrunc test on Shift-JIS characters
TEST(LargeModuleStrTest, strtrunc_shiftjis) {
  libreallive::Archive arc(locateTestCase("Module_Str_SEEN/strtrunc_1.TXT"));
  TestSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new StrModule);
  rlmachine.ExecuteUntilHalted();

  EXPECT_EQ("\x82\xED\x82\xBD\x82\xB5",
            rlmachine.GetStringValue(STRS_LOCATION, 0))
      << "strtrunc returned wrong value";
}

// Test hantozen() on ASCII numbers
//
//   strS[0] = "12345"
//   strS[0] = hantozen()
//
TEST(LargeModuleStrTest, hantozen_ascii) {
  libreallive::Archive arc(locateTestCase("Module_Str_SEEN/hantozen_0.TXT"));
  TestSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new StrModule);
  rlmachine.ExecuteUntilHalted();

  EXPECT_EQ("\x82\x50\x82\x51\x82\x52\x82\x53\x82\x54",
            rlmachine.GetStringValue(STRS_LOCATION, 0))
      << "hantozen returned wrong value";
}

// Test hantozen() on half width katakana; should convert them to full
// width katakana.
//
//   strS[0] = "ﾜﾀｼﾉﾅﾏｴ"
//   strS[0] = hantozen()
//
TEST(LargeModuleStrTest, hantozen_half_width_katakana) {
  libreallive::Archive arc(locateTestCase("Module_Str_SEEN/hantozen_1.TXT"));
  TestSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new StrModule);
  rlmachine.ExecuteUntilHalted();

  EXPECT_EQ("\x83\x8F\x83\x5E\x83\x56\x83\x6D\x83\x69\x83\x7D\x83\x47",
            rlmachine.GetStringValue(STRS_LOCATION, 0))
      << "hantozen returned wrong value";
}

// Tests zentohan on fullwidth ASCII characters.
//
//   strS[0] = "１２３４５"
//   strS[0] = zentohan()
//
TEST(LargeModuleStrTest, hantozen_fullwidth_ascii) {
  libreallive::Archive arc(locateTestCase("Module_Str_SEEN/zentohan_0.TXT"));
  TestSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new StrModule);
  rlmachine.ExecuteUntilHalted();

  EXPECT_EQ("12345", rlmachine.GetStringValue(STRS_LOCATION, 0))
      << "zentohan returned wrong value";
}

// Tests zentohan on fullwidth katakana characters.
//
//   strS[0] = "ワタシノナマエ"
//   strS[0] = zentohan()
//
TEST(LargeModuleStrTest, zentohan_fullwidth_katakana) {
  libreallive::Archive arc(locateTestCase("Module_Str_SEEN/zentohan_1.TXT"));
  TestSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new StrModule);
  rlmachine.ExecuteUntilHalted();

  EXPECT_EQ("\xDC\xC0\xBC\xC9\xC5\xCF\xB4",
            rlmachine.GetStringValue(STRS_LOCATION, 0))
      << "zentohan returned wrong value";
}

// Test Uppercase_0
//
//   strS[0] = "Valid"
//   // This syntax hurts my brain
//   strS[0] = Uppercase()
//
TEST(LargeModuleStrTest, Uppercase_0) {
  libreallive::Archive arc(locateTestCase("Module_Str_SEEN/uppercase_0.TXT"));
  TestSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new StrModule);
  rlmachine.ExecuteUntilHalted();

  EXPECT_EQ("VALID", rlmachine.GetStringValue(STRS_LOCATION, 0))
      << "Uppercase returned wrong value";
}

// Test Uppercase_1
//
//   strS[0] = "Valid"
//   // This syntax hurts my brain
//   strS[0] = Uppercase()
//
TEST(LargeModuleStrTest, Uppercase_1) {
  libreallive::Archive arc(locateTestCase("Module_Str_SEEN/uppercase_1.TXT"));
  TestSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new StrModule);
  rlmachine.ExecuteUntilHalted();

  EXPECT_EQ("Valid", rlmachine.GetStringValue(STRS_LOCATION, 0))
      << "Uppercase touched strS[0]";
  EXPECT_EQ("VALID", rlmachine.GetStringValue(STRS_LOCATION, 1))
      << "Uppercase returned wrong value";
}

// Test Lowercase_0
//
//   strS[0] = "Valid"
//   // This syntax hurts my brain
//   strS[0] = Lowercase()
//
TEST(LargeModuleStrTest, Lowercase_0) {
  libreallive::Archive arc(locateTestCase("Module_Str_SEEN/lowercase_0.TXT"));
  TestSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new StrModule);
  rlmachine.ExecuteUntilHalted();

  EXPECT_EQ("valid", rlmachine.GetStringValue(STRS_LOCATION, 0))
      << "Lowercase returned wrong value";
}

// Test Lowercase_1.
//
//   strS[0] = "Valid"
//   // This syntax hurts my brain
//   strS[1] = Lowercase(strS[0])
//
TEST(LargeModuleStrTest, Lowercase_1) {
  libreallive::Archive arc(locateTestCase("Module_Str_SEEN/lowercase_1.TXT"));
  TestSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new StrModule);
  rlmachine.ExecuteUntilHalted();

  EXPECT_EQ("Valid", rlmachine.GetStringValue(STRS_LOCATION, 0))
      << "Lowercase touched strS[0]";
  EXPECT_EQ("valid", rlmachine.GetStringValue(STRS_LOCATION, 1))
      << "Lowercase returned wrong value";
}

// Test itoa_ws's various stuff
//
//   strS[0] = itoa_ws(-1)
//   strS[1] = itoa_ws(-1, 3)
//   strS[2] = itoa_ws(3)
//   strS[3] = itoa_ws(1, 3)
//
TEST(LargeModuleStrTest, itoa_ws) {
  libreallive::Archive arc(locateTestCase("Module_Str_SEEN/itoa_ws_0.TXT"));
  TestSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new StrModule);
  rlmachine.ExecuteUntilHalted();

  EXPECT_EQ("\x81\x7C\x82\x50", rlmachine.GetStringValue(STRS_LOCATION, 0))
      << "itoa_ws set wrong value for strS[0]";
  EXPECT_EQ("\x81\x7C\x81\x40\x81\x40\x82\x50",
            rlmachine.GetStringValue(STRS_LOCATION, 1))
      << "itoa_ws set wrong value for strS[1]";
  EXPECT_EQ("\x82\x52", rlmachine.GetStringValue(STRS_LOCATION, 2))
      << "itoa_ws set wrong value for strS[2]";
  EXPECT_EQ("\x81\x40\x81\x40\x82\x50",
            rlmachine.GetStringValue(STRS_LOCATION, 3))
      << "itoa_ws set wrong value for strS[3]";
}

// Test itoa_s, which shouldn't be as hard to write as itoa_ws. -_-
//
//   strS[0] = itoa_s(-1)
//   strS[1] = itoa_s(-1, 3)
//   strS[2] = itoa_s(3)
//   strS[3] = itoa_s(1, 3)
//
TEST(LargeModuleStrTest, itoa_s) {
  libreallive::Archive arc(locateTestCase("Module_Str_SEEN/itoa_s_0.TXT"));
  TestSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new StrModule);
  rlmachine.ExecuteUntilHalted();

  EXPECT_EQ("-1", rlmachine.GetStringValue(STRS_LOCATION, 0))
      << "itoa_s set wrong value for strS[0]";
  EXPECT_EQ("-  1", rlmachine.GetStringValue(STRS_LOCATION, 1))
      << "itoa_s set wrong value for strS[1]";
  EXPECT_EQ("3", rlmachine.GetStringValue(STRS_LOCATION, 2))
      << "itoa_s set wrong value for strS[2]";
  EXPECT_EQ("  1", rlmachine.GetStringValue(STRS_LOCATION, 3))
      << "itoa_s set wrong value for strS[3]";
}

// Test itoa_w, which shouldn't be as hard to write as itoa_ws. -_-
//
//   strS[0] = itoa_w(-1)
//   strS[1] = itoa_w(-1, 3)
//   strS[2] = itoa_w(3)
//   strS[3] = itoa_w(1, 3)
//
TEST(LargeModuleStrTest, itoa_w) {
  libreallive::Archive arc(locateTestCase("Module_Str_SEEN/itoa_w_0.TXT"));
  TestSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new StrModule);
  rlmachine.ExecuteUntilHalted();

  EXPECT_EQ("\x81\x7C\x82\x50", rlmachine.GetStringValue(STRS_LOCATION, 0))
      << "itoa_w set wrong value for strS[0]";
  EXPECT_EQ("\x81\x7C\x82\x4F\x82\x4F\x82\x50",
            rlmachine.GetStringValue(STRS_LOCATION, 1))
      << "itoa_w set wrong value for strS[1]";
  EXPECT_EQ("\x82\x52", rlmachine.GetStringValue(STRS_LOCATION, 2))
      << "itoa_w set wrong value for strS[2]";
  EXPECT_EQ("\x82\x4F\x82\x4F\x82\x50",
            rlmachine.GetStringValue(STRS_LOCATION, 3))
      << "itoa_w set wrong value for strS[3]";
}

// Test itoa
//
//   strS[0] = itoa(-1)
//   strS[1] = itoa(-1, 3)
//   strS[2] = itoa(3)
//   strS[3] = itoa(1, 3)
//
TEST(LargeModuleStrTest, itoa) {
  libreallive::Archive arc(locateTestCase("Module_Str_SEEN/itoa_0.TXT"));
  TestSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new StrModule);
  rlmachine.ExecuteUntilHalted();

  EXPECT_EQ("-1", rlmachine.GetStringValue(STRS_LOCATION, 0))
      << "itoa set wrong value for strS[0]";
  EXPECT_EQ("-001", rlmachine.GetStringValue(STRS_LOCATION, 1))
      << "itoa set wrong value for strS[1]";
  EXPECT_EQ("3", rlmachine.GetStringValue(STRS_LOCATION, 2))
      << "itoa set wrong value for strS[2]";
  EXPECT_EQ("001", rlmachine.GetStringValue(STRS_LOCATION, 3))
      << "itoa set wrong value for strS[3]";
}

// Tests atoi.
//
//   intA[0] = atoi("15")
//   intA[1] = atoi("   15")
//   intA[2] = atoi("-12")
//   intA[3] = atoi("5  27")
//   intA[4] = atoi("asdf")
//
TEST(LargeModuleStrTest, atoi) {
  libreallive::Archive arc(locateTestCase("Module_Str_SEEN/atoi_0.TXT"));
  TestSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new StrModule);
  rlmachine.ExecuteUntilHalted();

  EXPECT_EQ(15, rlmachine.GetIntValue(IntMemRef('A', 0)))
      << "atoi returned wrong value for intA[0]";
  EXPECT_EQ(15, rlmachine.GetIntValue(IntMemRef('A', 1)))
      << "atoi returned wrong value for intA[1]";
  EXPECT_EQ(-12, rlmachine.GetIntValue(IntMemRef('A', 2)))
      << "atoi returned wrong value for intA[2]";
  EXPECT_EQ(5, rlmachine.GetIntValue(IntMemRef('A', 3)))
      << "atoi returned wrong value for intA[3]";
  EXPECT_EQ(0, rlmachine.GetIntValue(IntMemRef('A', 4)))
      << "atoi returned wrong value for intA[4]";
}

// Tests digits.
//
//   intA[0] = digits(1)
//   intA[1] = digits(20)
//   intA[2] = digits(-20)
//
TEST(LargeModuleStrTest, digits) {
  libreallive::Archive arc(locateTestCase("Module_Str_SEEN/digits_0.TXT"));
  TestSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new StrModule);
  rlmachine.ExecuteUntilHalted();

  EXPECT_EQ(1, rlmachine.GetIntValue(IntMemRef('A', 0)))
      << "digits returned wrong value for intA[0]";
  EXPECT_EQ(2, rlmachine.GetIntValue(IntMemRef('A', 1)))
      << "digits returned wrong value for intA[1]";
  EXPECT_EQ(2, rlmachine.GetIntValue(IntMemRef('A', 2)))
      << "digits returned wrong value for intA[2]";
}

// Tests strpos.
//
//   intA[0] = strpos("equal", "equal")
//   intA[1] = strpos("This is the the repitition test.", "the")
//   intA[2] = strpos("Itaiyo", "Uguu~")
//
TEST(LargeModuleStrTest, strpos) {
  libreallive::Archive arc(locateTestCase("Module_Str_SEEN/strpos_0.TXT"));
  TestSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new StrModule);
  rlmachine.ExecuteUntilHalted();

  EXPECT_EQ(0, rlmachine.GetIntValue(IntMemRef('A', 0)))
      << "strpos returned wrong value for intA[0]";
  EXPECT_EQ(8, rlmachine.GetIntValue(IntMemRef('A', 1)))
      << "strpos returned wrong value for intA[1]";
  EXPECT_EQ(-1, rlmachine.GetIntValue(IntMemRef('A', 2)))
      << "strpos returned wrong value for intA[2]";
}

// Tests strlpos.
//
//   intA[0] = strlpos("equal", "equal")
//   intA[1] = strlpos("This is the the repitition test.", "the")
//   intA[2] = strlpos("Itaiyo", "Uguu~")
//
TEST(LargeModuleStrTest, strlpos) {
  libreallive::Archive arc(locateTestCase("Module_Str_SEEN/strlpos_0.TXT"));
  TestSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new StrModule);
  rlmachine.ExecuteUntilHalted();

  EXPECT_EQ(0, rlmachine.GetIntValue(IntMemRef('A', 0)))
      << "strlpos returned wrong value for intA[0]";
  EXPECT_EQ(12, rlmachine.GetIntValue(IntMemRef('A', 1)))
      << "strlpos returned wrong value for intA[1]";
  EXPECT_EQ(-1, rlmachine.GetIntValue(IntMemRef('A', 2)))
      << "strlpos returned wrong value for intA[2]";
}

// Tests strused.
//
//   strS[1] = "Used"
//   intA[0] = strused(strS[0])
//   intA[1] = strused(strS[1])
//
TEST(LargeModuleStrTest, strused) {
  libreallive::Archive arc(locateTestCase("Module_Str_SEEN/strused_0.TXT"));
  TestSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.AttachModule(new StrModule);
  rlmachine.ExecuteUntilHalted();

  EXPECT_EQ(0, rlmachine.GetIntValue(IntMemRef('A', 0)))
      << "strused returned wrong value for intA[0]";
  EXPECT_EQ(1, rlmachine.GetIntValue(IntMemRef('A', 1)))
      << "strused returned wrong value for intA[1]";
}
