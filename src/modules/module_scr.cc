// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006 Elliot Glaysher
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

#include "modules/module_scr.h"

#include "machine/general_operations.h"
#include "machine/rlmachine.h"
#include "machine/rloperation.h"
#include "systems/base/graphics_stack_frame.h"
#include "systems/base/graphics_system.h"
#include "systems/base/surface.h"
#include "systems/base/system.h"

namespace {

struct stackNop : public RLOpcode<IntConstant_T> {
  void operator()(RLMachine& machine, int numberOfNops) {
    GraphicsSystem& sys = machine.system().graphics();

    for (int i = 0; i < numberOfNops; ++i) {
      sys.AddGraphicsStackCommand("");
    }
  }
};

struct stackTrunc : public RLOpcode<IntConstant_T> {
  void operator()(RLMachine& machine, int count) {
    GraphicsSystem& sys = machine.system().graphics();
    sys.StackPop(sys.StackSize() - count);
  }
};

struct GetDCPixel : public RLOpcode<IntConstant_T,
                                       IntConstant_T,
                                       IntConstant_T,
                                       IntReference_T,
                                       IntReference_T,
                                       IntReference_T> {
  void operator()(RLMachine& machine,
                  int x,
                  int y,
                  int dc,
                  IntReferenceIterator r,
                  IntReferenceIterator g,
                  IntReferenceIterator b) {
    int rval, gval, bval;
    machine.system().graphics().GetDC(dc)->GetDCPixel(
        Point(x, y), rval, gval, bval);
    *r = rval;
    *g = gval;
    *b = bval;
  }
};

}  // namespace

// -----------------------------------------------------------------------

ScrModule::ScrModule() : RLModule("Scr", 1, 30) {
  AddOpcode(0, 0, "stackClear", CallFunction(&GraphicsSystem::ClearStack));
  AddOpcode(1, 0, "stackNop", new stackNop);
  AddOpcode(2, 0, "StackPop", CallFunction(&GraphicsSystem::StackPop));
  AddOpcode(3, 0, "StackSize", ReturnIntValue(&GraphicsSystem::StackSize));
  AddOpcode(4, 0, "stackTrunc", new stackTrunc);

  AddOpcode(20,
            0,
            "DrawAuto",
            CallFunctionWith(&GraphicsSystem::SetScreenUpdateMode,
                             GraphicsSystem::SCREENUPDATEMODE_AUTOMATIC));
  AddOpcode(21,
            0,
            "DrawSemiAuto",
            CallFunctionWith(&GraphicsSystem::SetScreenUpdateMode,
                             GraphicsSystem::SCREENUPDATEMODE_SEMIAUTOMATIC));
  AddOpcode(22,
            0,
            "DrawManual",
            CallFunctionWith(&GraphicsSystem::SetScreenUpdateMode,
                             GraphicsSystem::SCREENUPDATEMODE_MANUAL));

  AddOpcode(31, 0, "GetDCPixel", new GetDCPixel);
}
