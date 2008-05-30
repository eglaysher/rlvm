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

#include "libReallive/archive.h"
#include "MachineBase/RLMachine.hpp"
#include "Effects/Effect.hpp"
#include "NullSystem/NullSystem.hpp"
#include "NullSystem/NullEventSystem.hpp"
#include "NullSystem/NullSurface.hpp"

#include "Utilities.h"
#include "testUtils.hpp"
#include "tut/tut.hpp"

#include <memory>

using namespace std;
using boost::shared_ptr;

namespace tut {

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

struct Effect_data {
  // Use any old test case; it isn't getting executed
  libReallive::Archive arc;
  NullSystem system;
  NullEventSystem& event;
  RLMachine rlmachine;

  Effect_data()
    : arc(locateTestCase("Module_Str_SEEN/strcpy_0.TXT")),
      system(locateTestCase("Gameexe_data/Gameexe.ini")),
      event(dynamic_cast<NullEventSystem&>(system.event())),
      rlmachine(system, arc) {
  }
};

typedef test_group<Effect_data> tf;
typedef tf::object object;
tf Effect_data("Effect");

// -----------------------------------------------------------------------

/** 
 * Testing code used by test<1> to test the actual code in the base
 * Effect class.
 */
class EffectPreconditionTest : public Effect {
public:
  EffectPreconditionTest(RLMachine& machine, boost::shared_ptr<Surface> src,
                         boost::shared_ptr<Surface> dst,
                         int width, int height, int time,
                         bool blit_original_image)
    : Effect(machine, src, dst, width, height, time),
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

/** 
 * Tests the base
 */
template<>
template<>
void object::test<1>()
{
  shared_ptr<EffectEventSystemTest> event_system_impl(new EffectEventSystemTest);    
  event.setMockHandler(event_system_impl);

  shared_ptr<Surface> src(new NullSurface("src"));
  shared_ptr<Surface> dst(new NullSurface("dst"));

  auto_ptr<EffectPreconditionTest> effect(
    new EffectPreconditionTest(rlmachine, src, dst, 640, 480, 2, false));

  // First loop through (with 
  bool retVal = false;
  for (int i = 0; i < 2; ++i) {
    retVal = (*effect)(rlmachine);
    ensure_not("Didn't prematurely quit", retVal);

    ostringstream str;
    str << i;
    ensure("Callid", effect->log().called("performEffectForTime", str.str()));

    event_system_impl->setTicks(i + 1);
  }

  retVal = (*effect)(rlmachine);
  ensure("Quit at the right time", retVal);
}

// -----------------------------------------------------------------------

};
