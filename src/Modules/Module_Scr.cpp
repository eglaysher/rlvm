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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//  
// -----------------------------------------------------------------------

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

/**
 * @file   Module_Scr.cpp
 * @author Elliot Glaysher
 * @date   Sat Feb  3 09:32:05 2007
 * 
 * @brief A module that contains a few graphics related functions.
 *
 * A quarter of what's in Sys should really be here instead. This
 * probably has something to do with the implementation details of the
 * official RealLive interpreter.
 */
#include "Module_Scr.hpp"


#include "MachineBase/RLOperation.hpp"
#include "MachineBase/GeneralOperations.hpp"
#include "MachineBase/RLMachine.hpp"

#include "Systems/Base/System.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/Surface.hpp"
#include "Systems/Base/GraphicsStackFrame.hpp"

// -----------------------------------------------------------------------

struct Scr_stackNop : public RLOp_Void_1< IntConstant_T > {
  void operator()(RLMachine& machine, int numberOfNops) {
    GraphicsSystem& sys = machine.system().graphics();

    for(int i = 0; i < numberOfNops; ++i) {
      sys.addGraphicsStackFrame("Nop");
    }
  }
};

// -----------------------------------------------------------------------

struct Scr_stackTrunc : public RLOp_Void_1< IntConstant_T > {
  void operator()(RLMachine& machine, int count) {
    GraphicsSystem& sys = machine.system().graphics();
    sys.stackPop(sys.stackSize() - count);
  }
};

// -----------------------------------------------------------------------

struct Scr_GetDCPixel : public RLOp_Void_6<
  IntConstant_T, IntConstant_T, IntConstant_T,
  IntReference_T, IntReference_T, IntReference_T>
{
  void operator()(RLMachine& machine, int x, int y, int dc,
                  IntReferenceIterator r, IntReferenceIterator g,
                  IntReferenceIterator b)
  {
    int rval, gval, bval;
    machine.system().graphics().getDC(dc)->getDCPixel(x, y, rval, gval, bval);
    *r = rval;
    *g = gval;
    *b = bval;
  }
};

// -----------------------------------------------------------------------

ScrModule::ScrModule()
  : RLModule("Scr", 1, 30)
{
  addOpcode(0, 0, "stackClear", callFunction(&GraphicsSystem::clearStack));
  addOpcode(1, 0, "stackNop", new Scr_stackNop);
  addOpcode(2, 0, "stackPop", callFunction(&GraphicsSystem::stackPop));
  addOpcode(3, 0, "stackSize", returnIntValue(&GraphicsSystem::stackSize));
  addOpcode(4, 0, "stackTrunc", new Scr_stackTrunc);

  addOpcode(20, 0, "DrawAuto",
            setToConstant(&GraphicsSystem::setScreenUpdateMode,
                          GraphicsSystem::SCREENUPDATEMODE_AUTOMATIC));
  addOpcode(21, 0, "DrawSemiAuto",
            setToConstant(&GraphicsSystem::setScreenUpdateMode,
                          GraphicsSystem::SCREENUPDATEMODE_SEMIAUTOMATIC));
  addOpcode(22, 0, "DrawManual",
            setToConstant(&GraphicsSystem::setScreenUpdateMode,
                          GraphicsSystem::SCREENUPDATEMODE_MANUAL));

  addOpcode(31, 0, "GetDCPixel", new Scr_GetDCPixel);
}

