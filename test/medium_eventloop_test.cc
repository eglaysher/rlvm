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
#include "modules/module_event_loop.h"

#include "test_utils.h"

class MediumEventLoopTest : public FullSystemTest {
 protected:
  MediumEventLoopTest() { rlmachine.AttachModule(new EventLoopModule); }
};

TEST_F(MediumEventLoopTest, TestSkipMode) {
  rlmachine.set_store_register(20);

  rlmachine.Exe("SetSkipMode", 0);
  EXPECT_TRUE(system.text().skip_mode());
  rlmachine.Exe("SkipMode", 0);
  EXPECT_EQ(1, rlmachine.store_register());

  rlmachine.Exe("ClearSkipMode", 0);
  EXPECT_FALSE(system.text().skip_mode());
  rlmachine.Exe("SkipMode", 0);
  EXPECT_EQ(0, rlmachine.store_register());
}
