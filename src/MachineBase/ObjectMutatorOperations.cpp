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

#include "MachineBase/ObjectMutatorOperations.hpp"

#include "MachineBase/Properties.hpp"
#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/EventSystem.hpp"
#include "Systems/Base/GraphicsObject.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/ObjectMutator.hpp"
#include "Systems/Base/System.hpp"

void GetMutatorObjectParams(RLOperation* op,
                            int in_object,
                            int* out_layer,
                            int* out_parent,
                            int* out_child) {
  // Fetch object  properties that need to be passed to the mutator.
  if (!op->getProperty(P_FGBG, *out_layer))
    *out_layer = OBJ_FG;

  if (op->getProperty(P_PARENTOBJ, *out_parent)) {
    *out_child = in_object;
  } else {
    *out_parent = in_object;
    *out_child = -1;
  }
}

// -----------------------------------------------------------------------

Op_ObjectMutatorInt::Op_ObjectMutatorInt(Getter getter,
                                         Setter setter,
                                         const char* name)
    : getter_(getter),
      setter_(setter),
      name_(name) {
}

Op_ObjectMutatorInt::~Op_ObjectMutatorInt() {}

void Op_ObjectMutatorInt::operator()(RLMachine& machine,
                                     int object,
                                     int endval,
                                     int duration_time,
                                     int delay,
                                     int type) {
  int fgbg, parentobject, childobject;
  GetMutatorObjectParams(this, object, &fgbg, &parentobject, &childobject);

  unsigned int creation_time = machine.system().event().getTicks();
  machine.system().graphics().AddObjectMutator(
      new OneIntObjectMutator(machine,
                              fgbg, parentobject, childobject, name_,
                              creation_time, delay, duration_time,
                              type, endval, getter_, setter_));
}

// -----------------------------------------------------------------------


Op_ObjectMutatorIntInt::Op_ObjectMutatorIntInt(
    Getter getter_one, Setter setter_one,
    Getter getter_two, Setter setter_two,
    const char* name)
    : getter_one_(getter_one),
      setter_one_(setter_one),
      getter_two_(getter_two),
      setter_two_(setter_two),
      name_(name) {
}

Op_ObjectMutatorIntInt::~Op_ObjectMutatorIntInt() {}

void Op_ObjectMutatorIntInt::operator()(RLMachine& machine,
                                        int object,
                                        int endval_one,
                                        int endval_two,
                                        int duration_time,
                                        int delay,
                                        int type) {
  int fgbg, parentobject, childobject;
  GetMutatorObjectParams(this, object, &fgbg, &parentobject, &childobject);

  unsigned int creation_time = machine.system().event().getTicks();
  machine.system().graphics().AddObjectMutator(
      new TwoIntObjectMutator(machine,
                              fgbg, parentobject, childobject, name_,
                              creation_time, delay, duration_time, type,
                              endval_one, getter_one_, setter_one_,
                              endval_two, getter_two_, setter_two_));
}
