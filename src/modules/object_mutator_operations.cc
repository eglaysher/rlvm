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

#include "machine/properties.h"
#include "machine/rlmachine.h"
#include "modules/module_obj.h"
#include "Systems/Base/EventSystem.hpp"
#include "Systems/Base/GraphicsObject.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/ObjectMutator.hpp"
#include "Systems/Base/System.hpp"

// -----------------------------------------------------------------------

Op_ObjectMutatorInt::Op_ObjectMutatorInt(Getter getter,
                                         Setter setter,
                                         const char* name)
    : getter_(getter), setter_(setter), name_(name) {}

Op_ObjectMutatorInt::~Op_ObjectMutatorInt() {}

void Op_ObjectMutatorInt::operator()(RLMachine& machine,
                                     int object,
                                     int endval,
                                     int duration_time,
                                     int delay,
                                     int type) {
  unsigned int creation_time = machine.system().event().getTicks();
  GraphicsObject& obj = getGraphicsObject(machine, this, object);

  int startval = (obj.*getter_)();
  obj.AddObjectMutator(new OneIntObjectMutator(name_,
                                               creation_time,
                                               duration_time,
                                               delay,
                                               type,
                                               startval,
                                               endval,
                                               setter_));
}

// -----------------------------------------------------------------------

Op_ObjectMutatorRepnoInt::Op_ObjectMutatorRepnoInt(Getter getter,
                                                   Setter setter,
                                                   const char* name)
    : getter_(getter), setter_(setter), name_(name) {}

Op_ObjectMutatorRepnoInt::~Op_ObjectMutatorRepnoInt() {}

void Op_ObjectMutatorRepnoInt::operator()(RLMachine& machine,
                                          int object,
                                          int repno,
                                          int endval,
                                          int duration_time,
                                          int delay,
                                          int type) {
  unsigned int creation_time = machine.system().event().getTicks();
  GraphicsObject& obj = getGraphicsObject(machine, this, object);

  int startval = (obj.*getter_)(repno);
  obj.AddObjectMutator(new RepnoIntObjectMutator(name_,
                                                 creation_time,
                                                 duration_time,
                                                 delay,
                                                 type,
                                                 repno,
                                                 startval,
                                                 endval,
                                                 setter_));
}

// -----------------------------------------------------------------------

Op_ObjectMutatorIntInt::Op_ObjectMutatorIntInt(Getter getter_one,
                                               Setter setter_one,
                                               Getter getter_two,
                                               Setter setter_two,
                                               const char* name)
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
  unsigned int creation_time = machine.system().event().getTicks();
  GraphicsObject& obj = getGraphicsObject(machine, this, object);
  int startval_one = (obj.*getter_one_)();
  int startval_two = (obj.*getter_two_)();

  obj.AddObjectMutator(new TwoIntObjectMutator(name_,
                                               creation_time,
                                               duration_time,
                                               delay,
                                               type,
                                               startval_one,
                                               endval_one,
                                               setter_one_,
                                               startval_two,
                                               endval_two,
                                               setter_two_));
}

// -----------------------------------------------------------------------

Op_EndObjectMutation_Normal::Op_EndObjectMutation_Normal(const char* name)
    : name_(name) {}

Op_EndObjectMutation_Normal::~Op_EndObjectMutation_Normal() {}

void Op_EndObjectMutation_Normal::operator()(RLMachine& machine,
                                             int object,
                                             int speedup) {
  getGraphicsObject(machine, this, object)
      .EndObjectMutatorMatching(machine, -1, name_, speedup);
}

// -----------------------------------------------------------------------

Op_EndObjectMutation_RepNo::Op_EndObjectMutation_RepNo(const char* name)
    : name_(name) {}

Op_EndObjectMutation_RepNo::~Op_EndObjectMutation_RepNo() {}

void Op_EndObjectMutation_RepNo::operator()(RLMachine& machine,
                                            int object,
                                            int repno,
                                            int speedup) {
  getGraphicsObject(machine, this, object)
      .EndObjectMutatorMatching(machine, repno, name_, speedup);
}
