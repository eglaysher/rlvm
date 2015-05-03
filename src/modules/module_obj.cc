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

#include "modules/module_obj.h"

#include "machine/properties.h"
#include "machine/rlmachine.h"
#include "systems/base/graphics_object.h"
#include "systems/base/graphics_system.h"
#include "systems/base/parent_graphics_object_data.h"
#include "systems/base/system.h"
#include "utilities/exception.h"
#include "libreallive/bytecode.h"

using libreallive::ExpressionPiece;

void EnsureIsParentObject(GraphicsObject& parent, int size) {
  if (parent.has_object_data()) {
    if (parent.GetObjectData().IsParentLayer()) {
      return;
    }
  }

  parent.SetObjectData(new ParentGraphicsObjectData(size));
}

GraphicsObject& GetGraphicsObject(RLMachine& machine,
                                  RLOperation* op,
                                  int obj) {
  GraphicsSystem& graphics = machine.system().graphics();

  int fgbg;
  if (!op->GetProperty(P_FGBG, fgbg))
    fgbg = OBJ_FG;

  int parentobj;
  if (op->GetProperty(P_PARENTOBJ, parentobj)) {
    GraphicsObject& parent = graphics.GetObject(fgbg, parentobj);
    EnsureIsParentObject(parent, graphics.GetObjectLayerSize());
    return static_cast<ParentGraphicsObjectData&>(parent.GetObjectData())
        .GetObject(obj);
  } else {
    return graphics.GetObject(fgbg, obj);
  }
}

LazyArray<GraphicsObject>& GetGraphicsObjects(RLMachine& machine,
                                              RLOperation* op) {
  GraphicsSystem& graphics = machine.system().graphics();

  int fgbg;
  if (!op->GetProperty(P_FGBG, fgbg))
    fgbg = OBJ_FG;

  int parentobj;
  if (op->GetProperty(P_PARENTOBJ, parentobj)) {
    GraphicsObject& parent = graphics.GetObject(fgbg, parentobj);
    EnsureIsParentObject(parent, graphics.GetObjectLayerSize());
    return static_cast<ParentGraphicsObjectData&>(parent.GetObjectData())
        .objects();
  } else if (fgbg == OBJ_FG) {
    return graphics.GetForegroundObjects();
  } else {
    return graphics.GetBackgroundObjects();
  }
}

void SetGraphicsObject(RLMachine& machine,
                       RLOperation* op,
                       int obj,
                       GraphicsObject& gobj) {
  GraphicsSystem& graphics = machine.system().graphics();

  int fgbg;
  if (!op->GetProperty(P_FGBG, fgbg))
    fgbg = OBJ_FG;

  int parentobj;
  if (op->GetProperty(P_PARENTOBJ, parentobj)) {
    GraphicsObject& parent = graphics.GetObject(fgbg, parentobj);
    EnsureIsParentObject(parent, graphics.GetObjectLayerSize());
    static_cast<ParentGraphicsObjectData&>(parent.GetObjectData())
        .SetObject(obj, gobj);
  } else {
    graphics.SetObject(fgbg, obj, gobj);
  }
}

// -----------------------------------------------------------------------

ObjRangeAdapter::ObjRangeAdapter(RLOperation* in) : handler(in) {}

ObjRangeAdapter::~ObjRangeAdapter() {}

void ObjRangeAdapter::operator()(RLMachine& machine,
                                 const libreallive::CommandElement& ff) {
  const libreallive::ExpressionPiecesVector& allParameters =
      ff.GetParsedParameters();

  // Range check the data
  if (allParameters.size() < 2)
    throw rlvm::Exception("Less then two arguments to an objRange function!");

  // BIG WARNING ABOUT THE FOLLOWING CODE: Note that we copy half of
  // what RLOperation.DispatchFunction() does; we manually call the
  // subclass's Dispatch() so that we can get around the automated
  // incrementing of the instruction pointer.
  int lowerRange = allParameters[0].GetIntegerValue(machine);
  int upperRange = allParameters[1].GetIntegerValue(machine);

  // Create a new list of expression pieces that contain a single integer at
  // the front. We will update this integer each time through the loop below.
  libreallive::ExpressionPiecesVector parameters;
  parameters.reserve(allParameters.size() - 1);
  parameters.emplace_back(
      libreallive::ExpressionPiece::IntConstant(lowerRange));

  // Copy everything after the first two items
  libreallive::ExpressionPiecesVector::const_iterator it =
      allParameters.begin();
  std::advance(it, 2);
  for (; it != allParameters.end(); ++it)
    parameters.emplace_back(*it);

  for (int i = lowerRange; i <= upperRange; ++i) {
    parameters[0] = libreallive::ExpressionPiece::IntConstant(i);
    handler->Dispatch(machine, parameters);
  }

  machine.AdvanceInstructionPointer();
}

RLOperation* RangeMappingFun(RLOperation* op) {
  return new ObjRangeAdapter(op);
}

// -----------------------------------------------------------------------
// ChildObjAdapter
// -----------------------------------------------------------------------

ChildObjAdapter::ChildObjAdapter(RLOperation* in) : handler(in) {}

ChildObjAdapter::~ChildObjAdapter() {}

void ChildObjAdapter::operator()(RLMachine& machine,
                                 const libreallive::CommandElement& ff) {
  const libreallive::ExpressionPiecesVector& allParameters =
      ff.GetParsedParameters();

  // Range check the data
  if (allParameters.size() < 1)
    throw rlvm::Exception("Less than one argument to an objLayered function!");

  int objset = allParameters[0].GetIntegerValue(machine);

  // Copy everything after the first item
  libreallive::ExpressionPiecesVector::const_iterator it =
      allParameters.begin();
  ++it;
  libreallive::ExpressionPiecesVector currentInstantiation(
      it, allParameters.end());

  handler->SetProperty(P_PARENTOBJ, objset);
  handler->Dispatch(machine, currentInstantiation);

  machine.AdvanceInstructionPointer();
}

RLOperation* ChildObjMappingFun(RLOperation* op) {
  return new ChildObjAdapter(op);
}

// -----------------------------------------------------------------------
// ChildObjRangeAdapter
// -----------------------------------------------------------------------

ChildObjRangeAdapter::ChildObjRangeAdapter(RLOperation* in) : handler(in) {}

ChildObjRangeAdapter::~ChildObjRangeAdapter() {}

void ChildObjRangeAdapter::operator()(RLMachine& machine,
                                      const libreallive::CommandElement& ff) {
  const libreallive::ExpressionPiecesVector& allParameters =
      ff.GetParsedParameters();

  // Range check the data
  if (allParameters.size() < 3) {
    throw rlvm::Exception(
        "Less then three arguments to an objChildRange function!");
  }

  // This part is like ChildObjAdapter; the first parameter is an integer
  // that represents the parent object.
  int objset = allParameters[0].GetIntegerValue(machine);

  // This part is like ObjRangeAdapter; the second and third parameters are
  // integers that represent a range of child objects.
  int lowerRange = allParameters[1].GetIntegerValue(machine);
  int upperRange = allParameters[2].GetIntegerValue(machine);

  // Create a new list of expression pieces that contain a single integer at
  // the front. We will update this integer each time through the loop below.
  libreallive::ExpressionPiecesVector parameters;
  parameters.reserve(allParameters.size() - 2);
  parameters.emplace_back(
      libreallive::ExpressionPiece::IntConstant(lowerRange));

  // Copy everything after the first three items
  libreallive::ExpressionPiecesVector::const_iterator it =
      allParameters.begin();
  std::advance(it, 3);
  for (; it != allParameters.end(); ++it)
    parameters.emplace_back(*it);

  for (int i = lowerRange; i <= upperRange; ++i) {
    parameters[0] = libreallive::ExpressionPiece::IntConstant(i);

    // Now Dispatch based on these parameters.
    handler->SetProperty(P_PARENTOBJ, objset);
    handler->Dispatch(machine, parameters);
  }

  machine.AdvanceInstructionPointer();
}

RLOperation* ChildRangeMappingFun(RLOperation* op) {
  return new ChildObjRangeAdapter(op);
}

// -----------------------------------------------------------------------
// Obj_CallFunction
// -----------------------------------------------------------------------

Obj_CallFunction::Obj_CallFunction(Function f) : function_(f) {}

Obj_CallFunction::~Obj_CallFunction() {}

void Obj_CallFunction::operator()(RLMachine& machine, int buf) {
  GraphicsObject& obj = GetGraphicsObject(machine, this, buf);
  ((obj).*(function_))();
}

// -----------------------------------------------------------------------
// Obj_SetOneIntOnObj
// -----------------------------------------------------------------------

Obj_SetOneIntOnObj::Obj_SetOneIntOnObj(Setter s) : setter(s) {}

Obj_SetOneIntOnObj::~Obj_SetOneIntOnObj() {}

void Obj_SetOneIntOnObj::operator()(RLMachine& machine, int buf, int incoming) {
  GraphicsObject& obj = GetGraphicsObject(machine, this, buf);
  ((obj).*(setter))(incoming);

  machine.system().graphics().mark_object_state_as_dirty();
}

// -----------------------------------------------------------------------
// Obj_SetTwoIntOnObj
// -----------------------------------------------------------------------

Obj_SetTwoIntOnObj::Obj_SetTwoIntOnObj(Setter one, Setter two)
    : setter_one_(one), setter_two_(two) {}

Obj_SetTwoIntOnObj::~Obj_SetTwoIntOnObj() {}

void Obj_SetTwoIntOnObj::operator()(RLMachine& machine,
                                    int buf,
                                    int incoming_one,
                                    int incoming_two) {
  GraphicsObject& obj = GetGraphicsObject(machine, this, buf);
  ((obj).*(setter_one_))(incoming_one);
  ((obj).*(setter_two_))(incoming_two);

  machine.system().graphics().mark_object_state_as_dirty();
}

// -----------------------------------------------------------------------
// Obj_SetRepnoIntOnObj
// -----------------------------------------------------------------------

Obj_SetRepnoIntOnObj::Obj_SetRepnoIntOnObj(Setter setter)
    : setter(setter) {}

Obj_SetRepnoIntOnObj::~Obj_SetRepnoIntOnObj() {}

void Obj_SetRepnoIntOnObj::operator()(RLMachine& machine,
                                      int buf,
                                      int idx,
                                      int val) {
  GraphicsObject& obj = GetGraphicsObject(machine, this, buf);
  ((obj).*(setter))(idx, val);
  machine.system().graphics().mark_object_state_as_dirty();
}
