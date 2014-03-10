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

struct stackNop : public RLOp_Void_1<IntConstant_T> {
  void operator()(RLMachine& machine, int numberOfNops) {
    GraphicsSystem& sys = machine.system().graphics();

    for (int i = 0; i < numberOfNops; ++i) {
      sys.addGraphicsStackCommand("");
    }
  }
};

struct stackTrunc : public RLOp_Void_1<IntConstant_T> {
  void operator()(RLMachine& machine, int count) {
    GraphicsSystem& sys = machine.system().graphics();
    sys.stackPop(sys.stackSize() - count);
  }
};

struct GetDCPixel : public RLOp_Void_6<IntConstant_T,
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
    machine.system().graphics().getDC(dc)->getDCPixel(
        Point(x, y), rval, gval, bval);
    *r = rval;
    *g = gval;
    *b = bval;
  }
};

}  // namespace

// -----------------------------------------------------------------------

ScrModule::ScrModule() : RLModule("Scr", 1, 30) {
  addOpcode(0, 0, "stackClear", callFunction(&GraphicsSystem::clearStack));
  addOpcode(1, 0, "stackNop", new stackNop);
  addOpcode(2, 0, "stackPop", callFunction(&GraphicsSystem::stackPop));
  addOpcode(3, 0, "stackSize", returnIntValue(&GraphicsSystem::stackSize));
  addOpcode(4, 0, "stackTrunc", new stackTrunc);

  addOpcode(20,
            0,
            "DrawAuto",
            callFunctionWith(&GraphicsSystem::setScreenUpdateMode,
                             GraphicsSystem::SCREENUPDATEMODE_AUTOMATIC));
  addOpcode(21,
            0,
            "DrawSemiAuto",
            callFunctionWith(&GraphicsSystem::setScreenUpdateMode,
                             GraphicsSystem::SCREENUPDATEMODE_SEMIAUTOMATIC));
  addOpcode(22,
            0,
            "DrawManual",
            callFunctionWith(&GraphicsSystem::setScreenUpdateMode,
                             GraphicsSystem::SCREENUPDATEMODE_MANUAL));

  addOpcode(31, 0, "GetDCPixel", new GetDCPixel);
}
