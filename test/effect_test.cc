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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
//
// -----------------------------------------------------------------------

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "effects/blind_effect.h"
#include "effects/effect.h"
#include "machine/rlmachine.h"
#include "test_system/mock_surface.h"
#include "test_system/test_event_system.h"
#include "test_system/test_system.h"

#include "test_utils.h"

#include <memory>

using namespace testing;

// Helper to specify the return value of GetTicks().
class EffectEventSystemTest : public EventSystemMockHandler {
 public:
  EffectEventSystemTest() : ticks(0) {}
  void setTicks(unsigned int in) { ticks = in; }
  virtual unsigned int GetTicks() const { return ticks; }

 private:
  unsigned int ticks;
};

class EffectTest : public FullSystemTest {
 protected:
  EffectTest() : event_system_impl(new EffectEventSystemTest) {
    dynamic_cast<TestEventSystem&>(system.event())
        .SetMockHandler(event_system_impl);
  }

  std::shared_ptr<EffectEventSystemTest> event_system_impl;
};

class MockEffect : public Effect {
 public:
  MockEffect(RLMachine& machine,
             std::shared_ptr<Surface> src,
             std::shared_ptr<Surface> dst,
             Size size,
             int time)
      : Effect(machine, src, dst, size, time) {}

  MOCK_METHOD2(PerformEffectForTime, void(RLMachine& machine, int));
  MOCK_CONST_METHOD0(BlitOriginalImage, bool());
};

TEST_F(EffectTest, TestBase) {
  std::shared_ptr<Surface> src(MockSurface::Create("src"));
  std::shared_ptr<Surface> dst(MockSurface::Create("dst"));

  std::unique_ptr<MockEffect> effect(
      new MockEffect(rlmachine, src, dst, Size(640, 480), 2));

  bool retVal = false;
  for (int i = 0; i < 2; ++i) {
    EXPECT_CALL(*effect, BlitOriginalImage()).WillOnce(Return(false));
    EXPECT_CALL(*effect, PerformEffectForTime(_, i)).Times(1);
    EXPECT_FALSE((*effect)(rlmachine)) << "Didn't prematurely quit";
    ASSERT_TRUE(::testing::Mock::VerifyAndClearExpectations(effect.get()));

    event_system_impl->setTicks(i + 1);
  }

  EXPECT_TRUE((*effect)(rlmachine)) << "We didn't quit?";
}

// -----------------------------------------------------------------------

class MockBlitTopToBottom : public BlindTopToBottomEffect {
 public:
  MockBlitTopToBottom(RLMachine& machine,
                      std::shared_ptr<Surface> src,
                      std::shared_ptr<Surface> dst,
                      int width,
                      int height,
                      int time,
                      int blindSize)
      : BlindTopToBottomEffect(machine,
                               src,
                               dst,
                               Size(width, height),
                               time,
                               blindSize) {}

  MOCK_METHOD2(PerformEffectForTime, void(const RLMachine& machine, int));
  MOCK_METHOD2(RenderPolygon, void(int, int));
};

TEST_F(EffectTest, BlindTopToBottomEffect) {
  std::shared_ptr<MockSurface> src(MockSurface::Create("src"));
  std::shared_ptr<MockSurface> dst(MockSurface::Create("dst"));

  const int DURATION = 100;
  const int BLIND_SIZE = 50;
  const int HEIGHT = 480;
  std::unique_ptr<MockBlitTopToBottom> effect(new MockBlitTopToBottom(
      rlmachine, src, dst, 640, HEIGHT, DURATION, BLIND_SIZE));

  int numBlinds = (HEIGHT / BLIND_SIZE) + 1;

  bool retVal = false;

  // Test at 0
  EXPECT_CALL(*effect, RenderPolygon(0, 0)).Times(1);
  EXPECT_FALSE((*effect)(rlmachine)) << "Prematurely quit";
  ASSERT_TRUE(::testing::Mock::VerifyAndClearExpectations(effect.get()));

  // Test at 25
  event_system_impl->setTicks(25);
  EXPECT_CALL(*effect, RenderPolygon(0, 15)).Times(1);
  EXPECT_CALL(*effect, RenderPolygon(50, 64)).Times(1);
  EXPECT_CALL(*effect, RenderPolygon(100, 113)).Times(1);
  EXPECT_CALL(*effect, RenderPolygon(150, 162)).Times(1);
  EXPECT_CALL(*effect, RenderPolygon(200, 211)).Times(1);
  EXPECT_CALL(*effect, RenderPolygon(250, 260)).Times(1);
  EXPECT_CALL(*effect, RenderPolygon(300, 309)).Times(1);
  EXPECT_CALL(*effect, RenderPolygon(350, 358)).Times(1);
  EXPECT_CALL(*effect, RenderPolygon(400, 407)).Times(1);
  EXPECT_CALL(*effect, RenderPolygon(450, 456)).Times(1);
  EXPECT_FALSE((*effect)(rlmachine)) << "Prematurely quit";
  ASSERT_TRUE(::testing::Mock::VerifyAndClearExpectations(effect.get()));

  // Test at 50
  event_system_impl->setTicks(50);
  EXPECT_CALL(*effect, RenderPolygon(0, 30)).Times(1);
  EXPECT_CALL(*effect, RenderPolygon(50, 79)).Times(1);
  EXPECT_CALL(*effect, RenderPolygon(100, 128)).Times(1);
  EXPECT_CALL(*effect, RenderPolygon(150, 177)).Times(1);
  EXPECT_CALL(*effect, RenderPolygon(200, 226)).Times(1);
  EXPECT_CALL(*effect, RenderPolygon(250, 275)).Times(1);
  EXPECT_CALL(*effect, RenderPolygon(300, 324)).Times(1);
  EXPECT_CALL(*effect, RenderPolygon(350, 373)).Times(1);
  EXPECT_CALL(*effect, RenderPolygon(400, 422)).Times(1);
  EXPECT_CALL(*effect, RenderPolygon(450, 471)).Times(1);
  EXPECT_FALSE((*effect)(rlmachine)) << "Prematurely quit";
  ASSERT_TRUE(::testing::Mock::VerifyAndClearExpectations(effect.get()));

  // Test at 75
  event_system_impl->setTicks(75);
  EXPECT_CALL(*effect, RenderPolygon(0, 45)).Times(1);
  EXPECT_CALL(*effect, RenderPolygon(50, 94)).Times(1);
  EXPECT_CALL(*effect, RenderPolygon(100, 143)).Times(1);
  EXPECT_CALL(*effect, RenderPolygon(150, 192)).Times(1);
  EXPECT_CALL(*effect, RenderPolygon(200, 241)).Times(1);
  EXPECT_CALL(*effect, RenderPolygon(250, 290)).Times(1);
  EXPECT_CALL(*effect, RenderPolygon(300, 339)).Times(1);
  EXPECT_CALL(*effect, RenderPolygon(350, 388)).Times(1);
  EXPECT_CALL(*effect, RenderPolygon(400, 437)).Times(1);
  EXPECT_CALL(*effect, RenderPolygon(450, 486)).Times(1);
  EXPECT_FALSE((*effect)(rlmachine)) << "Prematurely quit";
  ASSERT_TRUE(::testing::Mock::VerifyAndClearExpectations(effect.get()));

  // Test at the end
  event_system_impl->setTicks(100);
  EXPECT_TRUE((*effect)(rlmachine)) << "We didn't quit?";
}
