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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
//
// -----------------------------------------------------------------------

#include "modules/module_obj_getters.h"

#include "machine/properties.h"
#include "machine/rloperation.h"
#include "machine/rloperation/default_value.h"
#include "machine/rloperation/references.h"
#include "machine/rloperation/rlop_store.h"
#include "modules/module_obj.h"
#include "systems/base/graphics_object.h"

// -----------------------------------------------------------------------

namespace {

class Obj_GetInt : public RLStoreOpcode<IntConstant_T> {
 public:
  typedef int (GraphicsObject::*Getter)() const;

  explicit Obj_GetInt(Getter getter) : getter_(getter) {}
  virtual ~Obj_GetInt() {}

  virtual int operator()(RLMachine& machine, int buf) {
    GraphicsObject& obj = GetGraphicsObject(machine, this, buf);
    return ((obj).*(getter_))();
  }

 private:
  Getter getter_;
};

// Theoretically implements objGetPos. People don't actually
// understand what's going on in this module (more so then anything
// else in rldev/rlvm.)
//
// This is probably wrong or overlooks all sorts of weird corner cases
// that aren't immediatly obvious.
struct objGetPos
    : public RLOpcode<IntConstant_T, IntReference_T, IntReference_T> {
  void operator()(RLMachine& machine,
                  int objNum,
                  IntReferenceIterator xIt,
                  IntReferenceIterator yIt) {
    GraphicsObject& obj = GetGraphicsObject(machine, this, objNum);
    *xIt = obj.x();
    *yIt = obj.y();
  }
};

struct objGetAdjust : public RLOpcode<IntConstant_T, IntConstant_T,
                                         IntReference_T, IntReference_T> {
  void operator()(RLMachine& machine, int objNum, int repno,
                  IntReferenceIterator xIt,
                  IntReferenceIterator yIt) {
    GraphicsObject& obj = GetGraphicsObject(machine, this, objNum);
    *xIt = obj.x_adjustment(repno);
    *yIt = obj.y_adjustment(repno);
  }
};

struct objGetAdjustX : public RLStoreOpcode<IntConstant_T, IntConstant_T> {
  int operator()(RLMachine& machine, int objNum, int repno) {
    GraphicsObject& obj = GetGraphicsObject(machine, this, objNum);
    return obj.x_adjustment(repno);
  }
};

struct objGetAdjustY : public RLStoreOpcode<IntConstant_T, IntConstant_T> {
  int operator()(RLMachine& machine, int objNum, int repno) {
    GraphicsObject& obj = GetGraphicsObject(machine, this, objNum);
    return obj.y_adjustment(repno);
  }
};

// @note objGetDims takes an integer as its fourth argument, but we
// have no idea what this is or how it affects things. Usually appears
// to be 4. ????
struct objGetDims : public RLOpcode<IntConstant_T,
                                       IntReference_T,
                                       IntReference_T,
                                       DefaultIntValue_T<4>> {
  void operator()(RLMachine& machine,
                  int objNum,
                  IntReferenceIterator widthIt,
                  IntReferenceIterator heightIt,
                  int unknown) {
    GraphicsObject& obj = GetGraphicsObject(machine, this, objNum);
    *widthIt = obj.PixelWidth();
    *heightIt = obj.PixelHeight();
  }
};

void addFunctions(RLModule& m) {
  m.AddOpcode(1000, 0, "objGetPos", new objGetPos);
  m.AddOpcode(1001, 0, "objGetPosX", new Obj_GetInt(&GraphicsObject::x));
  m.AddOpcode(1002, 0, "objGetPosY", new Obj_GetInt(&GraphicsObject::y));
  m.AddOpcode(
      1003, 0, "objGetAlpha", new Obj_GetInt(&GraphicsObject::raw_alpha));
  m.AddOpcode(1004, 0, "objGetShow", new Obj_GetInt(&GraphicsObject::visible));

  m.AddOpcode(1006, 0, "objGetAdjust", new objGetAdjust);
  m.AddOpcode(1007, 0, "objGetAdjustX", new objGetAdjustX);
  m.AddOpcode(1008, 0, "objGetAdjustY", new objGetAdjustY);
  m.AddOpcode(1009, 0, "objGetMono", new Obj_GetInt(&GraphicsObject::mono));
  m.AddOpcode(1010, 0, "objGetInvert", new Obj_GetInt(&GraphicsObject::invert));
  m.AddOpcode(1011, 0, "objGetLight", new Obj_GetInt(&GraphicsObject::light));

  m.AddOpcode(1039, 0, "objGetPattNo",
              new Obj_GetInt(&GraphicsObject::GetPattNo));
  m.AddOpcode(1100, 0, "objGetDims", new objGetDims);
  m.AddOpcode(1100, 1, "objGetDims", new objGetDims);
}

}  // namespace

// -----------------------------------------------------------------------

ObjFgGettersModule::ObjFgGettersModule() : RLModule("ObjFgGetters", 1, 84) {
  addFunctions(*this);
  SetProperty(P_FGBG, OBJ_FG);
}

// -----------------------------------------------------------------------

ObjBgGettersModule::ObjBgGettersModule() : RLModule("ObjBgGetters", 1, 85) {
  addFunctions(*this);
  SetProperty(P_FGBG, OBJ_BG);
}

// -----------------------------------------------------------------------

ChildObjFgGettersModule::ChildObjFgGettersModule()
    : MappedRLModule(ChildObjMappingFun, "ChildObjFgGetters", 2, 84) {
  addFunctions(*this);
  SetProperty(P_FGBG, OBJ_FG);
}

// -----------------------------------------------------------------------

ChildObjBgGettersModule::ChildObjBgGettersModule()
    : MappedRLModule(ChildObjMappingFun, "ChildObjBgGetters", 2, 85) {
  addFunctions(*this);
  SetProperty(P_FGBG, OBJ_BG);
}
