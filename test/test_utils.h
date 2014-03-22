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
// the Free Software Foundation; either version  of the License, or
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

#ifndef TEST_TESTUTILS_HPP_
#define TEST_TESTUTILS_HPP_

#include <string>

#include "gtest/gtest.h"
#include "libreallive/archive.h"
#include "test_system/test_machine.h"
#include "test_system/test_system.h"

// Locates a test file in the test/ directory.
std::string locateTestCase(const std::string& baseName);

// A base class for all tests that instantiate an archive, a System and a
// Machine.
class FullSystemTest : public ::testing::Test {
 protected:
  // Grab a random SEEN.TXT file and the Gameexe.ini file and load them.
  FullSystemTest();
  ~FullSystemTest();

  libreallive::Archive arc;
  TestSystem system;
  TestMachine rlmachine;
};

#endif  // TEST_TESTUTILS_HPP_
