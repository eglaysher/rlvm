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

#ifndef SRC_MODULES_MODULE_OBJ_H_
#define SRC_MODULES_MODULE_OBJ_H_

// Reusable function objects for the GraphicsObject system.

#include "machine/rloperation.h"
#include "utilities/lazy_array.h"

class GraphicsObject;

// -----------------------------------------------------------------------

void EnsureIsParentObject(GraphicsObject& parent, int size);

GraphicsObject& GetGraphicsObject(RLMachine& machine, RLOperation* op, int obj);

LazyArray<GraphicsObject>& GetGraphicsObjects(RLMachine& machine,
                                              RLOperation* op);

void SetGraphicsObject(RLMachine& machine,
                       RLOperation* op,
                       int obj,
                       GraphicsObject& gobj);

// -----------------------------------------------------------------------

// Special adapter to make any of obj* and objBg* operation structs
// into an objRange* or objRangeBg* struct.
//
// We extract the first two expression pieces from the incoming
// command and assume that they are integers and are the bounds on the
// object number. We then construct a set of parameters to pass to the
// real implementation.
//
// This is certainly not the most efficient way to do it, but it cuts
// down on a duplicated operation struct for each obj* and objBg*
// function, alowing us to just use this adapter with the already
// defined operations.
class ObjRangeAdapter : public RLOp_SpecialCase {
 public:
  explicit ObjRangeAdapter(RLOperation* in);
  virtual ~ObjRangeAdapter();

  virtual void operator()(RLMachine& machine,
                          const libreallive::CommandElement& ff) final;

 private:
  std::unique_ptr<RLOperation> handler;
};

// Mapping function for a MappedRLModule which turns operation op into
// a ranged operation.
//
// The wrapper takes ownership of the incoming op pointer, and the
// caller takes ownership of the resultant RLOperation.
RLOperation* RangeMappingFun(RLOperation* op);

// -----------------------------------------------------------------------

// An adapter that changes a normal object operation into one that operates on
// an object's child objects.
class ChildObjAdapter : public RLOp_SpecialCase {
 public:
  explicit ChildObjAdapter(RLOperation* in);
  virtual ~ChildObjAdapter();

  virtual void operator()(RLMachine& machine,
                          const libreallive::CommandElement& ff) final;

 private:
  std::unique_ptr<RLOperation> handler;
};

RLOperation* ChildObjMappingFun(RLOperation* op);

// -----------------------------------------------------------------------

// An adapter that ranges over children of a certain parent object.
class ChildObjRangeAdapter : public RLOp_SpecialCase {
 public:
  explicit ChildObjRangeAdapter(RLOperation* in);
  virtual ~ChildObjRangeAdapter();

  virtual void operator()(RLMachine& machine,
                          const libreallive::CommandElement& ff) final;

 private:
  std::unique_ptr<RLOperation> handler;
};

// The combo form of rangeMappingFun and childObjMappingFun. Used for
// operations that start with (parent object num, first child num, last child
// num).
RLOperation* ChildRangeMappingFun(RLOperation* op);

// -----------------------------------------------------------------------

// Calls a function on an object.
//
// NOTE: This does *not* call mark_object_state_as_dirty(), like the rest of
// these adapters.
class Obj_CallFunction : public RLOpcode<IntConstant_T> {
 public:
  typedef void (GraphicsObject::*Function)();

  explicit Obj_CallFunction(Function f);
  virtual ~Obj_CallFunction();

  virtual void operator()(RLMachine& machine, int buf) final;

 private:
  Function function_;
};

// -----------------------------------------------------------------------

// Specialized form of Op_SetToIncomingInt to deal with looking up
// object from the Obj* helper templates; since a lot of Object
// related functions simply call a setter.
//
// This template magic saves having to write out 25 - 30 operation
// structs.
class Obj_SetOneIntOnObj : public RLOpcode<IntConstant_T, IntConstant_T> {
 public:
  // The function signature for the setter function
  typedef void (GraphicsObject::*Setter)(const int);

  explicit Obj_SetOneIntOnObj(Setter s);
  virtual ~Obj_SetOneIntOnObj();

  virtual void operator()(RLMachine& machine, int buf, int incoming) final;

 private:
  // The setter function to call on Op_SetToIncoming::reference when
  // called.
  Setter setter;
};

// -----------------------------------------------------------------------

// Specialized form of Op_SetToIncomingInt to deal with looking up object from
// the Obj* helper templates; since a lot of Object related functions simply
// call a setter.
class Obj_SetTwoIntOnObj
    : public RLOpcode<IntConstant_T, IntConstant_T, IntConstant_T> {
 public:
  typedef void (GraphicsObject::*Setter)(const int);

  Obj_SetTwoIntOnObj(Setter one, Setter two);
  virtual ~Obj_SetTwoIntOnObj();

  virtual void operator()(RLMachine& machine,
                          int buf,
                          int incoming_one,
                          int incoming_two) final;

 private:
  Setter setter_one_;
  Setter setter_two_;
};

// -----------------------------------------------------------------------

class Obj_SetRepnoIntOnObj
    : public RLOpcode<IntConstant_T, IntConstant_T, IntConstant_T> {
 public:
  typedef void (GraphicsObject::*Setter)(const int, const int);

  Obj_SetRepnoIntOnObj(Setter setter);
  virtual ~Obj_SetRepnoIntOnObj();

  virtual void operator()(RLMachine& machine,
                          int buf,
                          int idx,
                          int val) final;

 private:
  Setter setter;
};

#endif  // SRC_MODULES_MODULE_OBJ_H_
