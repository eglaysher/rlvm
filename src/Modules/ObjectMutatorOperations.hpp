// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2013 Elliot Glaysher
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
// -----------------------------------------------------------------------

#ifndef SRC_MACHINEBASE_OBJECTMUTATOROPERATIONS_HPP_
#define SRC_MACHINEBASE_OBJECTMUTATOROPERATIONS_HPP_

#include "MachineBase/RLOperation.hpp"

class GraphicsObject;

class Op_ObjectMutatorInt
    : public RLOp_Void_5< IntConstant_T, IntConstant_T, IntConstant_T,
                          IntConstant_T, IntConstant_T > {
 public:
  typedef int(GraphicsObject::*Getter)() const;
  typedef void(GraphicsObject::*Setter)(const int);

  Op_ObjectMutatorInt(Getter getter, Setter setter, const char* name);
  virtual ~Op_ObjectMutatorInt();

  virtual void operator()(RLMachine& machine,
                          int object,
                          int endval,
                          int duration_time,
                          int delay,
                          int type);
 private:
  Getter getter_;
  Setter setter_;
  const char* name_;
};

class Op_ObjectMutatorIntInt
    : public RLOp_Void_6< IntConstant_T, IntConstant_T, IntConstant_T,
                          IntConstant_T, IntConstant_T, IntConstant_T > {
 public:
  typedef int(GraphicsObject::*Getter)() const;
  typedef void(GraphicsObject::*Setter)(const int);

  Op_ObjectMutatorIntInt(Getter getter_one, Setter setter_one,
                         Getter getter_two, Setter setter_two,
                         const char* name);
  virtual ~Op_ObjectMutatorIntInt();

  virtual void operator()(RLMachine& machine,
                          int object,
                          int endval_one,
                          int endval_two,
                          int duration_time,
                          int delay,
                          int type);
 private:
  Getter getter_one_;
  Setter setter_one_;
  Getter getter_two_;
  Setter setter_two_;
  const char* name_;
};

// -----------------------------------------------------------------------

class Op_EndObjectMutation_Normal
    : public RLOp_Void_2< IntConstant_T, IntConstant_T > {
 public:
  Op_EndObjectMutation_Normal(const char* name);
  virtual ~Op_EndObjectMutation_Normal();

  virtual void operator()(RLMachine& machine, int object, int speedup);

 private:
  const char* name_;
};

// -----------------------------------------------------------------------

class Op_EndObjectMutation_RepNo
    : public RLOp_Void_3< IntConstant_T, IntConstant_T, IntConstant_T > {
 public:
  Op_EndObjectMutation_RepNo(const char* name);
  virtual ~Op_EndObjectMutation_RepNo();

  virtual void operator()(RLMachine& machine, int object, int repno,
                          int speedup);

 private:
  const char* name_;
};

#endif  // SRC_MACHINEBASE_OBJECTMUTATOROPERATIONS_HPP_
