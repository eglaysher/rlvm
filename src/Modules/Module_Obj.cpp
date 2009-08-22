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
 * @file   Module_Obj.cpp
 * @author Elliot Glaysher
 * @date   Sat Feb 10 10:34:12 2007
 *
 * @brief  Reusable function objects for the GraphicsObject system.
 */

#include "Modules/Module_Obj.hpp"

#include "MachineBase/Properties.hpp"
#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/GraphicsObject.hpp"
#include "Systems/Base/ParentGraphicsObjectData.hpp"
#include "Utilities/Exception.hpp"
#include "libReallive/bytecode.h"

#include <iostream>
using namespace std;

using libReallive::ExpressionPiece;
using boost::ptr_vector;

void ensureIsParentObject(GraphicsObject& parent) {
  if (parent.hasObjectData()) {
    if (parent.objectData().isParentLayer()) {
      return;
    }
  }

  parent.setObjectData(new ParentGraphicsObjectData);
}

// -----------------------------------------------------------------------

GraphicsObject& getGraphicsObject(RLMachine& machine, RLOperation* op,
                                  int obj) {
  GraphicsSystem& graphics = machine.system().graphics();

  int fgbg;
  if (!op->getProperty(P_FGBG, fgbg))
    fgbg = OBJ_FG;

  int parentobj;
  if (op->getProperty(P_PARENTOBJ, parentobj)) {
    GraphicsObject& parent = graphics.getObject(fgbg, parentobj);
    ensureIsParentObject(parent);
    return static_cast<ParentGraphicsObjectData&>(parent.objectData()).
        getObject(obj);
  } else {
    return graphics.getObject(fgbg, obj);
  }
}

// -----------------------------------------------------------------------

void setGraphicsObject(RLMachine& machine, RLOperation* op, int obj,
                       GraphicsObject& gobj) {
  GraphicsSystem& graphics = machine.system().graphics();

  int fgbg;
  if (!op->getProperty(P_FGBG, fgbg))
    fgbg = OBJ_FG;

  int parentobj;
  if (op->getProperty(P_PARENTOBJ, parentobj)) {
    GraphicsObject& parent = graphics.getObject(fgbg, parentobj);
    ensureIsParentObject(parent);
    static_cast<ParentGraphicsObjectData&>(parent.objectData()).
        setObject(obj, gobj);
  } else {
    graphics.setObject(fgbg, obj, gobj);
  }
}

// -----------------------------------------------------------------------
// ChildObjAdapter
// -----------------------------------------------------------------------

ChildObjAdapter::ChildObjAdapter(RLOperation* in) : handler(in) {
}

// -----------------------------------------------------------------------

void ChildObjAdapter::operator()(RLMachine& machine,
                                 const libReallive::CommandElement& ff) {
  const ptr_vector<ExpressionPiece>& allParameters = ff.getParameters();

  // Range check the data
  if (allParameters.size() < 1)
    throw rlvm::Exception("Less than one argument to an objLayered function!");

  int objset = allParameters[0].integerValue(machine);

  // Copy everything after the first item
  ptr_vector<ExpressionPiece> currentInstantiation;
  ptr_vector<ExpressionPiece>::const_iterator it = allParameters.begin();
  ++it;
  for (; it != allParameters.end(); ++it) {
    currentInstantiation.push_back(it->clone());
  }

  handler->setProperty(P_PARENTOBJ, objset);
  handler->dispatch(machine, currentInstantiation);

  machine.advanceInstructionPointer();
}

// -----------------------------------------------------------------------

RLOperation* childObjMappingFun(RLOperation* op) {
  return new ChildObjAdapter(op);
}

// -----------------------------------------------------------------------
// Obj_SetOneIntOnObj
// -----------------------------------------------------------------------

Obj_SetOneIntOnObj::Obj_SetOneIntOnObj(Setter s)
    : setter(s) {
}

// -----------------------------------------------------------------------

Obj_SetOneIntOnObj::~Obj_SetOneIntOnObj()
{}

// -----------------------------------------------------------------------

void Obj_SetOneIntOnObj::operator()(RLMachine& machine, int buf, int incoming) {
  GraphicsObject& obj = getGraphicsObject(machine, this, buf);
  ((obj).*(setter))(incoming);
}


// -----------------------------------------------------------------------
// Obj_SetTwoIntOnObj
// -----------------------------------------------------------------------

Obj_SetTwoIntOnObj::Obj_SetTwoIntOnObj(Setter one, Setter two)
  : setterOne(one), setterTwo(two) {
}

// -----------------------------------------------------------------------

Obj_SetTwoIntOnObj::~Obj_SetTwoIntOnObj() {
}

// -----------------------------------------------------------------------

void Obj_SetTwoIntOnObj::operator()(RLMachine& machine, int buf,
                                    int incomingOne, int incomingTwo) {
  GraphicsObject& obj = getGraphicsObject(machine, this, buf);
  ((obj).*(setterOne))(incomingOne);
  ((obj).*(setterTwo))(incomingTwo);
}
