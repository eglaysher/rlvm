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

#include "modules/object_mutator_operations.h"

#include "long_operations/wait_long_operation.h"
#include "machine/properties.h"
#include "machine/rlmachine.h"
#include "modules/module_obj.h"
#include "systems/base/event_system.h"
#include "systems/base/graphics_object.h"
#include "systems/base/graphics_system.h"
#include "systems/base/object_mutator.h"
#include "systems/base/system.h"

namespace {

bool MutatorIsDone(RLMachine& machine,
                   RLOperation* op,
                   int obj,
                   int repno,
                   const std::string& name) {
  return GetGraphicsObject(machine, op, obj)
             .IsMutatorRunningMatching(repno, name) == false;
}

bool ObjectMutatorIsWorking(RLMachine& machine,
                            RLOperation* op,
                            int obj,
                            int repno,
                            const std::string& name) {
  return GetGraphicsObject(machine, op, obj)
             .IsMutatorRunningMatching(repno, name) == false;
}

}  // namespace

// -----------------------------------------------------------------------

Op_ObjectMutatorInt::Op_ObjectMutatorInt(Getter getter,
                                         Setter setter,
                                         const std::string& name)
    : getter_(getter), setter_(setter), name_(name) {}

Op_ObjectMutatorInt::~Op_ObjectMutatorInt() {}

void Op_ObjectMutatorInt::operator()(RLMachine& machine,
                                     int object,
                                     int endval,
                                     int duration_time,
                                     int delay,
                                     int type) {
  unsigned int creation_time = machine.system().event().GetTicks();
  GraphicsObject& obj = GetGraphicsObject(machine, this, object);

  int startval = (obj.*getter_)();
  obj.AddObjectMutator(std::unique_ptr<ObjectMutator>(
      new OneIntObjectMutator(name_,
                              creation_time,
                              duration_time,
                              delay,
                              type,
                              startval,
                              endval,
                              setter_)));
}

// -----------------------------------------------------------------------

Op_ObjectMutatorRepnoInt::Op_ObjectMutatorRepnoInt(Getter getter,
                                                   Setter setter,
                                                   const std::string& name)
    : getter_(getter), setter_(setter), name_(name) {}

Op_ObjectMutatorRepnoInt::~Op_ObjectMutatorRepnoInt() {}

void Op_ObjectMutatorRepnoInt::operator()(RLMachine& machine,
                                          int object,
                                          int repno,
                                          int endval,
                                          int duration_time,
                                          int delay,
                                          int type) {
  unsigned int creation_time = machine.system().event().GetTicks();
  GraphicsObject& obj = GetGraphicsObject(machine, this, object);

  int startval = (obj.*getter_)(repno);
  obj.AddObjectMutator(std::unique_ptr<ObjectMutator>(
      new RepnoIntObjectMutator(name_,
                                creation_time,
                                duration_time,
                                delay,
                                type,
                                repno,
                                startval,
                                endval,
                                setter_)));
}

// -----------------------------------------------------------------------

Op_ObjectMutatorIntInt::Op_ObjectMutatorIntInt(Getter getter_one,
                                               Setter setter_one,
                                               Getter getter_two,
                                               Setter setter_two,
                                               const std::string& name)
    : getter_one_(getter_one),
      setter_one_(setter_one),
      getter_two_(getter_two),
      setter_two_(setter_two),
      name_(name) {}

Op_ObjectMutatorIntInt::~Op_ObjectMutatorIntInt() {}

void Op_ObjectMutatorIntInt::operator()(RLMachine& machine,
                                        int object,
                                        int endval_one,
                                        int endval_two,
                                        int duration_time,
                                        int delay,
                                        int type) {
  unsigned int creation_time = machine.system().event().GetTicks();
  GraphicsObject& obj = GetGraphicsObject(machine, this, object);
  int startval_one = (obj.*getter_one_)();
  int startval_two = (obj.*getter_two_)();

  obj.AddObjectMutator(std::unique_ptr<ObjectMutator>(
      new TwoIntObjectMutator(name_,
                              creation_time,
                              duration_time,
                              delay,
                              type,
                              startval_one,
                              endval_one,
                              setter_one_,
                              startval_two,
                              endval_two,
                              setter_two_)));
}

// -----------------------------------------------------------------------

Op_EndObjectMutation_Normal::Op_EndObjectMutation_Normal(
    const std::string& name)
    : name_(name) {}

Op_EndObjectMutation_Normal::~Op_EndObjectMutation_Normal() {}

void Op_EndObjectMutation_Normal::operator()(RLMachine& machine,
                                             int object,
                                             int speedup) {
  GetGraphicsObject(machine, this, object)
      .EndObjectMutatorMatching(machine, -1, name_, speedup);
}

// -----------------------------------------------------------------------

Op_EndObjectMutation_RepNo::Op_EndObjectMutation_RepNo(const std::string& name)
    : name_(name) {}

Op_EndObjectMutation_RepNo::~Op_EndObjectMutation_RepNo() {}

void Op_EndObjectMutation_RepNo::operator()(RLMachine& machine,
                                            int object,
                                            int repno,
                                            int speedup) {
  GetGraphicsObject(machine, this, object)
      .EndObjectMutatorMatching(machine, repno, name_, speedup);
}

// -----------------------------------------------------------------------

Op_MutatorCheck::Op_MutatorCheck(const std::string& name)
    : name_(name) {}

Op_MutatorCheck::~Op_MutatorCheck() {}

int Op_MutatorCheck::operator()(RLMachine& machine, int object) {
  GraphicsObject& obj = GetGraphicsObject(machine, this, object);
  return obj.IsMutatorRunningMatching(object, name_) ? 1 : 0;
}

// -----------------------------------------------------------------------

Op_MutatorWaitNormal::Op_MutatorWaitNormal(const std::string& name)
    : name_(name) {}

Op_MutatorWaitNormal::~Op_MutatorWaitNormal() {}

void Op_MutatorWaitNormal::operator()(RLMachine& machine, int obj) {
  WaitLongOperation* wait_op = new WaitLongOperation(machine);
  wait_op->BreakOnEvent(
      std::bind(MutatorIsDone, std::ref(machine), this, obj, -1, name_));
  machine.PushLongOperation(wait_op);
}

// -----------------------------------------------------------------------

Op_MutatorWaitRepNo::Op_MutatorWaitRepNo(const std::string& name)
    : name_(name) {}

Op_MutatorWaitRepNo::~Op_MutatorWaitRepNo() {}

void Op_MutatorWaitRepNo::operator()(RLMachine& machine, int obj, int repno) {
  WaitLongOperation* wait_op = new WaitLongOperation(machine);
  wait_op->BreakOnEvent(
      std::bind(MutatorIsDone, std::ref(machine), this, obj, repno, name_));
  machine.PushLongOperation(wait_op);
}

// -----------------------------------------------------------------------

Op_MutatorWaitCNormal::Op_MutatorWaitCNormal(const std::string& name)
    : name_(name) {}

Op_MutatorWaitCNormal::~Op_MutatorWaitCNormal() {}

void Op_MutatorWaitCNormal::operator()(RLMachine& machine, int obj) {
  WaitLongOperation* wait_op = new WaitLongOperation(machine);
  wait_op->BreakOnClicks();
  wait_op->BreakOnEvent(std::bind(
      ObjectMutatorIsWorking, std::ref(machine), this, obj, -1, name_));
  machine.PushLongOperation(wait_op);
}

// -----------------------------------------------------------------------

Op_MutatorWaitCRepNo::Op_MutatorWaitCRepNo(const std::string& name)
    : name_(name) {}

Op_MutatorWaitCRepNo::~Op_MutatorWaitCRepNo() {}

void Op_MutatorWaitCRepNo::operator()(RLMachine& machine, int obj, int repno) {
  WaitLongOperation* wait_op = new WaitLongOperation(machine);
  wait_op->BreakOnClicks();
  wait_op->BreakOnEvent(std::bind(
      ObjectMutatorIsWorking, std::ref(machine), this, obj, repno, name_));
  machine.PushLongOperation(wait_op);
}
