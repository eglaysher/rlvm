// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006, 2007 Elliot Glaysher
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

#include "Module_Obj.hpp"
#include "Module_ObjPosDims.hpp"

#include "MachineBase/RLOperation.hpp"
#include "MachineBase/RLOperation/DefaultValue.hpp"
#include "MachineBase/RLOperation/References.hpp"

#include "Systems/Base/GraphicsObject.hpp"

#include <iostream>
using namespace std;

// -----------------------------------------------------------------------

/**
 * Theoretically implements objGetPos. People don't actually
 * understand what's going on in this module (more so then anything
 * else in rldev/rlvm.)
 *
 * This is probably wrong or overlooks all sorts of weird corner cases
 * that aren't immediatly obvious.
 */
struct Obj_objGetPos
  : public RLOp_Void_3< IntConstant_T, IntReference_T, IntReference_T >
{
  int layer_;
  Obj_objGetPos(int layer) : layer_(layer) {}

  void operator()(RLMachine& machine, int objNum, IntReferenceIterator xIt,
                  IntReferenceIterator yIt)
  {
    GraphicsObject& obj = getGraphicsObject(machine, layer_, objNum);
    *xIt = obj.x();
    *yIt = obj.y();
  }
};

// -----------------------------------------------------------------------

/**
 * @note objGetDims takes an integer as its fourth argument, but we
 * have no idea what this is or how it affects things. Usually appears
 * to be 4. ????
 */
struct Obj_objGetDims
  : public RLOp_Void_4< IntConstant_T, IntReference_T, IntReference_T,
                        DefaultIntValue_T<4> >
{
  int layer_;
  Obj_objGetDims(int layer) : layer_(layer) {}

  void operator()(RLMachine& machine, int objNum, IntReferenceIterator widthIt,
                  IntReferenceIterator heightIt, int unknown)
  {
    GraphicsObject& obj = getGraphicsObject(machine, layer_, objNum);
    *widthIt = obj.pixelWidth();
    *heightIt = obj.pixelHeight();
  }
};

// -----------------------------------------------------------------------

ObjFgPosDimsModule::ObjFgPosDimsModule()
  : RLModule("ObjFgPosDims", 1, 84)
{
  addOpcode(1000, 0, new Obj_objGetPos(OBJ_FG_LAYER));
  addOpcode(1100, 0, new Obj_objGetDims(OBJ_FG_LAYER));
  addOpcode(1100, 1, new Obj_objGetDims(OBJ_FG_LAYER));
}

// -----------------------------------------------------------------------


ObjBgPosDimsModule::ObjBgPosDimsModule()
  : RLModule("ObjBgPosDims", 1, 85)
{
  addOpcode(1000, 0, new Obj_objGetPos(OBJ_BG_LAYER));
  addOpcode(1100, 0, new Obj_objGetDims(OBJ_BG_LAYER));
  addOpcode(1100, 1, new Obj_objGetDims(OBJ_BG_LAYER));
}
