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

#ifndef SRC_MODULES_MODULE_OBJ_HPP_
#define SRC_MODULES_MODULE_OBJ_HPP_

// Reusable function objects for the GraphicsObject system.

#include <boost/ptr_container/ptr_vector.hpp>

#include "MachineBase/RLOperation.hpp"

class GraphicsObject;

// -----------------------------------------------------------------------

void ensureIsParentObject(GraphicsObject& parent, int size);

GraphicsObject& getGraphicsObject(RLMachine& machine, RLOperation* op,
                                  int obj);

void setGraphicsObject(RLMachine& machine, RLOperation* op, int obj,
                       GraphicsObject& gobj);

// -----------------------------------------------------------------------

// An adapter that changes a normal object operation into one that operates on
// an object's child objects.
class ChildObjAdapter : public RLOp_SpecialCase {
 public:
  explicit ChildObjAdapter(RLOperation* in);

  virtual void operator()(RLMachine& machine,
                          const libReallive::CommandElement& ff);

 private:
  boost::scoped_ptr<RLOperation> handler;
};

RLOperation* childObjMappingFun(RLOperation* op);

// -----------------------------------------------------------------------

// Specialized form of Op_SetToIncomingInt to deal with looking up
// object from the Obj* helper templates; since a lot of Object
// related functions simply call a setter.
//
// This template magic saves having to write out 25 - 30 operation
// structs.
class Obj_SetOneIntOnObj : public RLOp_Void_2< IntConstant_T, IntConstant_T > {
 private:
  // The function signature for the setter function
  typedef void(GraphicsObject::*Setter)(const int);

  // The setter function to call on Op_SetToIncoming::reference when
  // called.
  Setter setter;

 public:
  explicit Obj_SetOneIntOnObj(Setter s);
  ~Obj_SetOneIntOnObj();

  void operator()(RLMachine& machine, int buf, int incoming);
};

// -----------------------------------------------------------------------

// Specialized form of Op_SetToIncomingInt to deal with looking up object from
// the Obj* helper templates; since a lot of Object related functions simply
// call a setter.
class Obj_SetTwoIntOnObj
    : public RLOp_Void_3< IntConstant_T, IntConstant_T, IntConstant_T > {
  // The function signature for the setter function
  typedef void(GraphicsObject::*Setter)(const int);

  // The setter functions to call on Op_SetToIncoming::reference when
  // called.
  Setter setterOne;
  Setter setterTwo;

 public:
  Obj_SetTwoIntOnObj(Setter one, Setter two);
  ~Obj_SetTwoIntOnObj();

  void operator()(RLMachine& machine, int buf, int incomingOne,
                  int incomingTwo);
};


#endif  // SRC_MODULES_MODULE_OBJ_HPP_
