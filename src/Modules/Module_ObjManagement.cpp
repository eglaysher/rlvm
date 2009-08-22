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
 * @file   Module_ObjManagement.cpp
 * @author Elliot Glaysher
 * @date   Wed Jan 31 18:33:08 2007
 *
 * @brief  Modules 60, 61, and 62. Misc. object management.
 */

#include "Module_Obj.hpp"
#include "Module_ObjManagement.hpp"

#include "MachineBase/Properties.hpp"
#include "MachineBase/RLOperation.hpp"
#include "MachineBase/RLMachine.hpp"
#include "MachineBase/RLModule.hpp"
#include "Systems/Base/GraphicsObject.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/System.hpp"

// -----------------------------------------------------------------------

struct Obj_objCopyFgToBg : public RLOp_Void_1<IntConstant_T>
{
  void operator()(RLMachine& machine, int buf) {
    GraphicsSystem& sys = machine.system().graphics();
    GraphicsObject& go = sys.getObject(OBJ_FG, buf);
    sys.setObject(OBJ_BG, buf, go);
  }
};

// -----------------------------------------------------------------------

struct Obj_objCopy : public RLOp_Void_2<IntConstant_T, IntConstant_T> {
  int from_fgbg_, to_fgbg_;
  Obj_objCopy(int from, int to) : from_fgbg_(from), to_fgbg_(to) {}

  void operator()(RLMachine& machine, int sbuf, int dbuf) {
    GraphicsSystem& sys = machine.system().graphics();
    GraphicsObject& go = sys.getObject(from_fgbg_, sbuf);
    sys.setObject(to_fgbg_, dbuf, go);
  }
};

// -----------------------------------------------------------------------

struct Obj_objClear_0 : public RLOp_Void_1<IntConstant_T> {
  void operator()(RLMachine& machine, int buf) {
    GraphicsObject& obj = getGraphicsObject(machine, this, buf);
    obj.clearObject();
  }
};

// -----------------------------------------------------------------------

struct Obj_objClear_1 : public RLOp_Void_2<IntConstant_T, IntConstant_T> {
  // I highly suspect that this has range semantics like
  // Obj_setWipeCopyTo_1, but none of the games I own use this
  // function.
  void operator()(RLMachine& machine, int min, int max) {
    // Inclusive ranges make baby Kerrigan and Ritchie cry.
    max++;

    for (int i = min; i < max; ++i) {
      getGraphicsObject(machine, this, i).clearObject();
    }
  }
};

// -----------------------------------------------------------------------

struct Obj_objDelete_0 : public RLOp_Void_1<IntConstant_T> {
  void operator()(RLMachine& machine, int buf) {
    GraphicsObject& obj = getGraphicsObject(machine, this, buf);
    obj.deleteObject();
  }
};

// -----------------------------------------------------------------------

struct Obj_objDelete_1 : public RLOp_Void_2<IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, int min, int max) {
    // Inclusive ranges make baby Kerrigan and Ritchie cry.
    max++;

    for (int i = min; i < max; ++i) {
      getGraphicsObject(machine, this, i).deleteObject();
    }
  }
};

// -----------------------------------------------------------------------

struct Obj_setWipeCopyTo_0 : public RLOp_Void_1< IntConstant_T > {
  int val_;
  Obj_setWipeCopyTo_0(int value) : val_(value) {}

  void operator()(RLMachine& machine, int buf) {
    getGraphicsObject(machine, this, buf).setWipeCopy(val_);
  }
};

// -----------------------------------------------------------------------

struct Obj_setWipeCopyTo_1 : public RLOp_Void_2< IntConstant_T, IntConstant_T >
{
  int val_;
  Obj_setWipeCopyTo_1(int value) : val_(value) {}

  void operator()(RLMachine& machine, int min, int numObjsToSet) {
    int maxObj = min + numObjsToSet;
    for (int i = min; i < maxObj; ++i) {
      getGraphicsObject(machine, this, i).setWipeCopy(val_);
    }
  }
};

// -----------------------------------------------------------------------

ObjCopyFgToBg::ObjCopyFgToBg()
  : RLModule("ObjCopyFgToBg", 1, 60)
{
  // This may be wrong; the function is undocumented, but this appears
  // to fix the display problem in Kanon OP.
  addOpcode(2, 0, "objCopyFgToBg", new Obj_objCopyFgToBg);
}

// -----------------------------------------------------------------------

void addObjManagementFunctions(RLModule& m) {
  m.addOpcode(4, 0, "objWipeCopyOn", new Obj_setWipeCopyTo_0(1));
  m.addOpcode(4, 1, "objWipeCopyOn", new Obj_setWipeCopyTo_1(1));
  m.addOpcode(5, 0, "objWipeCopyOff", new Obj_setWipeCopyTo_0(0));
  m.addOpcode(5, 1, "objWipeCopyOff", new Obj_setWipeCopyTo_1(0));

  m.addOpcode(10, 0, "objClear", new Obj_objClear_0);
  m.addOpcode(10, 1, "objClear", new Obj_objClear_1);
  m.addOpcode(11, 0, "objDelete", new Obj_objDelete_0);
  m.addOpcode(11, 1, "objDelete", new Obj_objDelete_1);
}

// -----------------------------------------------------------------------

ObjFgManagement::ObjFgManagement()
  : RLModule("ObjFgManagement", 1, 61)
{
  addOpcode(2, 0, "objCopy", new Obj_objCopy(OBJ_FG, OBJ_FG));
  addOpcode(3, 0, "objCopyToBg", new Obj_objCopy(OBJ_FG, OBJ_BG));

  addObjManagementFunctions(*this);
  setProperty(P_FGBG, OBJ_FG);
}

// -----------------------------------------------------------------------

ObjBgManagement::ObjBgManagement()
  : RLModule("ObjBgManagement", 1, 62)
{
  addOpcode(2, 0, "objBgCopyToFg", new Obj_objCopy(OBJ_BG, OBJ_FG));
  addOpcode(3, 0, "objBgCopy", new Obj_objCopy(OBJ_BG, OBJ_BG));

  addObjManagementFunctions(*this);
  setProperty(P_FGBG, OBJ_BG);
}

// -----------------------------------------------------------------------

ChildObjFgManagement::ChildObjFgManagement()
    : MappedRLModule(childObjMappingFun, "ChildObjFgManagement", 2, 61) {
  addOpcode(2, 0, "objSetCopy", new Obj_objCopy(OBJ_FG, OBJ_FG));
  addOpcode(3, 0, "objSetCopyToBg", new Obj_objCopy(OBJ_FG, OBJ_BG));

  addObjManagementFunctions(*this);
  setProperty(P_FGBG, OBJ_FG);
}

// -----------------------------------------------------------------------

ChildObjBgManagement::ChildObjBgManagement()
    : MappedRLModule(childObjMappingFun, "ChildObjFgManagement", 2, 62) {
  addOpcode(2, 0, "objSetBgCopyToFg", new Obj_objCopy(OBJ_BG, OBJ_FG));
  addOpcode(3, 0, "objSetBgCopy", new Obj_objCopy(OBJ_BG, OBJ_BG));

  addObjManagementFunctions(*this);
  setProperty(P_FGBG, OBJ_BG);
}

// -----------------------------------------------------------------------

