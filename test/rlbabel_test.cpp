// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2009 Elliot Glaysher
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
// -----------------------------------------------------------------------

#include "gtest/gtest.h"

#include "MachineBase/RLMachine.hpp"
#include "TestSystem/TestSystem.hpp"
#include "Systems/Base/RlBabelDLL.hpp"
#include "Utilities/Exception.hpp"
#include "libreallive/archive.h"
#include "libreallive/intmemref.h"
#include "testUtils.hpp"

#include <string>

class RLBabelTest : public FullSystemTest {};

const std::string rlBabel = "rlBabel";

TEST_F(RLBabelTest, Loading) {
  EXPECT_FALSE(rlmachine.dllLoaded(rlBabel));
  rlmachine.loadDLL(0, rlBabel);
  EXPECT_TRUE(rlmachine.dllLoaded(rlBabel));

  // Shouldn't throw.
  rlmachine.callDLL(0, dllInitialise, 0, 0, 0, 0);

  rlmachine.unloadDLL(0);
  EXPECT_FALSE(rlmachine.dllLoaded(rlBabel));

  EXPECT_THROW({ rlmachine.callDLL(0, dllInitialise, 0, 0, 0, 0); },
               rlvm::Exception);
}

TEST_F(RLBabelTest, Initialization) {
  rlmachine.loadDLL(0, rlBabel);

  // Initialize DLL
  rlmachine.callDLL(0, dllInitialise, 0, 0, 0, 0);

  // Prime the buffer
  rlmachine.setStringValue(
      libreallive::STRS_LOCATION, 0, "A world, covered in white");
  rlmachine.callDLL(
      0, dllTextoutStart, libreallive::STRS_LOCATION << 16, 0, 0, 0);

  // TODO: Doing anything real with RLBabel requires that we have working
  // font metrics in TestSystem...
}
