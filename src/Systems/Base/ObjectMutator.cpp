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

#include "Systems/Base/ObjectMutator.hpp"

#include <iostream>
using namespace std;

#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/EventSystem.hpp"
#include "Systems/Base/GraphicsObject.hpp"
#include "Systems/Base/GraphicsObjectData.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/ParentGraphicsObjectData.hpp"
#include "Systems/Base/System.hpp"

ObjectMutator::ObjectMutator(int repr,
                             const char* name,
                             int creation_time,
                             int duration_time,
                             int delay,
                             int type)
    : repr_(repr),
      name_(name),
      creation_time_(creation_time),
      duration_time_(duration_time),
      delay_(delay),
      type_(type) {
}

ObjectMutator::~ObjectMutator() {}

bool ObjectMutator::operator()(RLMachine& machine, GraphicsObject& object) {
  unsigned int ticks = machine.system().event().getTicks();
  if (ticks > (creation_time_ + delay_)) {
    PerformSetting(machine, object);
    machine.system().graphics().markObjectStateAsDirty();
  }
  return ticks > (creation_time_ + delay_ + duration_time_);
}

bool ObjectMutator::OperationMatches(int repr, const char* name) {
  return repr_ == repr && (strcmp(name, name_) == 0);
}

int ObjectMutator::GetValueForTime(RLMachine& machine, int start, int end) {
  unsigned int ticks = machine.system().event().getTicks();
  if (ticks < (creation_time_ + delay_)) {
    return start;
  } else if (ticks < (creation_time_ + delay_ + duration_time_)) {
    // TODO(erg): This is the implementation for type_ == 0. Add nonlinear ones
    // for 1 and 2.
    unsigned int ticks_into_duration_ = ticks - creation_time_ - delay_;
    float percentage = float(ticks_into_duration_) / float(duration_time_);
    return start + ((end - start) * percentage);
  } else {
    return end;
  }
}

// -----------------------------------------------------------------------

OneIntObjectMutator::OneIntObjectMutator(
    const char* name,
    int creation_time, int duration_time, int delay,
    int type, int start_value, int target_value, Setter setter)
    : ObjectMutator(-1, name, creation_time, duration_time, delay, type),
      startval_(start_value),
      endval_(target_value),
      setter_(setter) {
}

OneIntObjectMutator::~OneIntObjectMutator() {
}

void OneIntObjectMutator::SetToEnd(RLMachine& machine,
                                   GraphicsObject& object) {
  (object.*setter_)(endval_);
}

void OneIntObjectMutator::PerformSetting(RLMachine& machine,
                                         GraphicsObject& object) {
  int value = GetValueForTime(machine, startval_, endval_);
  (object.*setter_)(value);
}

// -----------------------------------------------------------------------

TwoIntObjectMutator::TwoIntObjectMutator(
    const char* name,
    int creation_time, int duration_time, int delay, int type,
    int start_one, int target_one, Setter setter_one,
    int start_two, int target_two, Setter setter_two)
    : ObjectMutator(-1, name, creation_time, duration_time, delay, type),
      startval_one_(start_one),
      endval_one_(target_one),
      setter_one_(setter_one),
      startval_two_(start_two),
      endval_two_(target_two),
      setter_two_(setter_two) {
}

TwoIntObjectMutator::~TwoIntObjectMutator() {
}

void TwoIntObjectMutator::SetToEnd(RLMachine& machine,
                                   GraphicsObject& object) {
  (object.*setter_one_)(endval_one_);
  (object.*setter_two_)(endval_two_);
}

void TwoIntObjectMutator::PerformSetting(RLMachine& machine,
                                         GraphicsObject& object) {
  int value = GetValueForTime(machine, startval_one_, endval_one_);
  (object.*setter_one_)(value);

  value = GetValueForTime(machine, startval_two_, endval_two_);
  (object.*setter_two_)(value);
}
