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

#include "machine/rlmachine.h"
#include "Modules/Module_Msg.hpp"
#include "TestSystem/TestMachine.hpp"
#include "TestSystem/TestSystem.hpp"
#include "libreallive/archive.h"

#include "testUtils.hpp"

class MediumMsgLoopTest : public FullSystemTest {
 protected:
  MediumMsgLoopTest() { rlmachine.attachModule(new MsgModule); }
};

TEST_F(MediumMsgLoopTest, TestFastText) {
  system.text().setFastTextMode(0);

  rlmachine.exe("FastText", 0);
  EXPECT_TRUE(system.text().fastTextMode());

  rlmachine.exe("NormalText", 0);
  EXPECT_FALSE(system.text().fastTextMode());
}
