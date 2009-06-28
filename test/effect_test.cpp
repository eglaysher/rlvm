// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2008 Elliot Glaysher
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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// -----------------------------------------------------------------------

#include "gtest/gtest.h"

#include "libReallive/archive.h"
#include "MachineBase/RLMachine.hpp"
#include "Effects/Effect.hpp"
#include "Effects/BlindEffect.hpp"
#include "NullSystem/NullSystem.hpp"
#include "NullSystem/NullEventSystem.hpp"
#include "NullSystem/NullSurface.hpp"

#include "testUtils.hpp"

#include <boost/shared_ptr.hpp>
#include <boost/assign/list_of.hpp>
#include <memory>
#include <set>

using std::auto_ptr;
using boost::shared_ptr;

/// Helper to specify the return value of getTicks().
class EffectEventSystemTest : public EventSystemMockHandler {
public:
  EffectEventSystemTest() : ticks(0) { }
  void setTicks(unsigned int in) { ticks = in; }
  virtual unsigned int getTicks() const { return ticks; }
private:
  ///
  unsigned int ticks;
};

class EffectTest : public ::testing::Test {
 protected:
  EffectTest()
      : arc(locateTestCase("Module_Str_SEEN/strcpy_0.TXT")),
        system(locateTestCase("Gameexe_data/Gameexe.ini")),
        event(dynamic_cast<NullEventSystem&>(system.event())),
        event_system_impl(new EffectEventSystemTest),
        rlmachine(system, arc) {
    event.setMockHandler(event_system_impl);
  }

  // Use any old test case; it isn't getting executed
  libReallive::Archive arc;
  NullSystem system;
  NullEventSystem& event;
  RLMachine rlmachine;

  shared_ptr<EffectEventSystemTest> event_system_impl;
};

class EffectPreconditionTest : public Effect {
public:
  EffectPreconditionTest(RLMachine& machine, boost::shared_ptr<Surface> src,
                         boost::shared_ptr<Surface> dst,
                         Size size, int time,
                         bool blit_original_image)
    : Effect(machine, src, dst, size, time),
      blit_original_image_(blit_original_image),
      log_("EffectLog") {
  }
  virtual ~EffectPreconditionTest() {}
  MockLog& log() { return log_; }

protected:
  virtual void performEffectForTime(RLMachine& machine,
                                    int currentTime) {
    log_.recordFunction("performEffectForTime", currentTime);
  }

private:
  virtual bool blitOriginalImage() const {
    log_.recordFunction("blitOriginalImage");
    return blit_original_image_;
  }

  // What we should return
  bool blit_original_image_;

  mutable MockLog log_;
};


TEST_F(EffectTest, DISABLED_TestBase) {
  shared_ptr<Surface> src(new NullSurface("src"));
  shared_ptr<Surface> dst(new NullSurface("dst"));

  auto_ptr<EffectPreconditionTest> effect(
    new EffectPreconditionTest(rlmachine, src, dst, Size(640, 480), 2, false));

  // First loop through (with
  bool retVal = false;
  for (int i = 0; i < 2; ++i) {
    retVal = (*effect)(rlmachine);
    EXPECT_FALSE(retVal) << "Didn't prematurely quit";
    effect->log().ensure("performEffectForTime", i);

    event_system_impl->setTicks(i + 1);
  }

  retVal = (*effect)(rlmachine);
  EXPECT_TRUE(retVal) << "Quit at the right time";
}

// -----------------------------------------------------------------------

class BlindTopToBottomWithLog : public BlindTopToBottomEffect {
public:
  BlindTopToBottomWithLog(RLMachine& machine, boost::shared_ptr<Surface> src,
                          boost::shared_ptr<Surface> dst,
                          int width, int height, int time, int blindSize)
    : BlindTopToBottomEffect(machine, src, dst, Size(width, height), time, blindSize),
      log_("BlindTopToBottomEffect") {
  }

  virtual void performEffectForTime(RLMachine& machine, int currentTime) {
    log_.recordFunction("performEffectForTime", currentTime);
    BlindTopToBottomEffect::performEffectForTime(machine, currentTime);
  }

  virtual void renderPolygon(int polyStart, int polyEnd) {
    log_.recordFunction("renderPolygon", polyStart, polyEnd);
    BlindTopToBottomEffect::renderPolygon(polyStart, polyEnd);
  }

  MockLog& log() { return log_; }

private:
  mutable MockLog log_;
};

TEST_F(EffectTest, DISABLED_BlindTopToBottomEffect) {
  shared_ptr<NullSurface> src(new NullSurface("src"));
  shared_ptr<NullSurface> dst(new NullSurface("dst"));

  const int DURATION = 100;
  const int BLIND_SIZE = 50;
  const int HEIGHT = 480;
  auto_ptr<BlindTopToBottomWithLog> effect(
    new BlindTopToBottomWithLog(
      rlmachine, src, dst, 640, HEIGHT, DURATION, BLIND_SIZE));

  int numBlinds = (HEIGHT / BLIND_SIZE) + 1;

  bool retVal = false;

  // Test at 0
  retVal = (*effect)(rlmachine);
  EXPECT_FALSE(retVal) << "Prematurely quit";
  effect->log().ensure("renderPolygon", 0, 0);
  effect->log().clear();

  // Test at 25
  event_system_impl->setTicks(25);
  retVal = (*effect)(rlmachine);
  EXPECT_FALSE(retVal) << "Prematurely quit";
  effect->log().ensure("renderPolygon", 0, 15);
  effect->log().ensure("renderPolygon", 50, 64);
  effect->log().ensure("renderPolygon", 100, 113);
  effect->log().ensure("renderPolygon", 150, 162);
  effect->log().ensure("renderPolygon", 200, 211);
  effect->log().ensure("renderPolygon", 250, 260);
  effect->log().ensure("renderPolygon", 300, 309);
  effect->log().ensure("renderPolygon", 350, 358);
  effect->log().ensure("renderPolygon", 400, 407);
  effect->log().ensure("renderPolygon", 450, 456);
  effect->log().clear();

  // Test at 50
  event_system_impl->setTicks(50);
  retVal = (*effect)(rlmachine);
  EXPECT_FALSE(retVal) << "Prematurely quit";
  effect->log().ensure("renderPolygon", 0, 30);
  effect->log().ensure("renderPolygon", 50, 79);
  effect->log().ensure("renderPolygon", 100, 128);
  effect->log().ensure("renderPolygon", 150, 177);
  effect->log().ensure("renderPolygon", 200, 226);
  effect->log().ensure("renderPolygon", 250, 275);
  effect->log().ensure("renderPolygon", 300, 324);
  effect->log().ensure("renderPolygon", 350, 373);
  effect->log().ensure("renderPolygon", 400, 422);
  effect->log().ensure("renderPolygon", 450, 471);
  effect->log().clear();

  // Test at 75
  event_system_impl->setTicks(75);
  retVal = (*effect)(rlmachine);
  EXPECT_FALSE(retVal) << "Prematurely quit";
  effect->log().ensure("renderPolygon", 0, 45);
  effect->log().ensure("renderPolygon", 50, 94);
  effect->log().ensure("renderPolygon", 100, 143);
  effect->log().ensure("renderPolygon", 150, 192);
  effect->log().ensure("renderPolygon", 200, 241);
  effect->log().ensure("renderPolygon", 250, 290);
  effect->log().ensure("renderPolygon", 300, 339);
  effect->log().ensure("renderPolygon", 350, 388);
  effect->log().ensure("renderPolygon", 400, 437);
  effect->log().ensure("renderPolygon", 450, 486);
  effect->log().clear();

  // TODO: test at the end?
//  (98, pair_list_of(0, 50)(50, 100)(100, 150)(150, 200)(200, 250)(250, 300)
//   (300, 350)(350, 400)(400, 450)(450, 499));

  // Test at the end
  event_system_impl->setTicks(100);
  retVal = (*effect)(rlmachine);
  EXPECT_TRUE(retVal) << "We quit";
}
