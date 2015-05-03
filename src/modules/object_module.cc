// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2014 Elliot Glaysher
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

#include "modules/object_module.h"

#include "machine/rlmodule.h"
#include "modules/module_obj.h"
#include "modules/object_mutator_operations.h"

ObjectModule::ObjectModule(const std::string& prefix,
                           RLModule* module)
    : prefix_(prefix),
      module_(module) {
}

ObjectModule::~ObjectModule() {}

void ObjectModule::AddSingleObjectCommands(int base_id,
                                           const std::string& name,
                                           NormalGetter getter,
                                           NormalSetter setter) {
  string base_name = prefix_ + name;
  module_->AddOpcode(1000 + base_id, 0, base_name,
                     new Obj_SetOneIntOnObj(setter));

  string eve_name = prefix_ + "Eve" + name;
  string base_eve_name = "objEve" + name;
  module_->AddOpcode(2000 + base_id, 0, eve_name,
                     new Obj_SetOneIntOnObj(setter));
  module_->AddOpcode(2000 + base_id, 1, eve_name,
                     new Op_ObjectMutatorInt(getter, setter, base_eve_name));

  AddCheck(eve_name, base_eve_name, base_id);
  AddNormalFinale(eve_name, base_eve_name, base_id);
}

void ObjectModule::AddDoubleObjectCommands(int base_id,
                                           const std::string& name,
                                           NormalGetter getter_one,
                                           NormalSetter setter_one,
                                           NormalGetter getter_two,
                                           NormalSetter setter_two) {
  string base_name = prefix_ + name;
  module_->AddOpcode(1000 + base_id, 0, base_name,
                     new Obj_SetTwoIntOnObj(setter_one, setter_two));

  string eve_name = prefix_ + "Eve" + name;
  string base_eve_name = "objEve" + name;
  module_->AddOpcode(2000 + base_id, 0, eve_name,
                     new Obj_SetTwoIntOnObj(setter_one, setter_two));
  module_->AddOpcode(2000 + base_id, 1, eve_name,
                     new Op_ObjectMutatorIntInt(
                         getter_one,
                         setter_one,
                         getter_two,
                         setter_two,
                         base_eve_name));

  AddCheck(eve_name, base_eve_name, base_id);
  AddNormalFinale(eve_name, base_eve_name, base_id);
}

void ObjectModule::AddRepnoObjectCommands(int base_id,
                                          const std::string& name,
                                          RepnoGetter getter,
                                          RepnoSetter setter) {
  string base_name = prefix_ + name;
  module_->AddOpcode(1000 + base_id, 0, base_name,
                     new Obj_SetRepnoIntOnObj(setter));

  string eve_name = prefix_ + "Eve" + name;
  string base_eve_name = "objEve" + name;
  module_->AddOpcode(2000 + base_id, 0, eve_name,
                     new Obj_SetRepnoIntOnObj(setter));
  module_->AddOpcode(2000 + base_id, 1, eve_name,
                     new Op_ObjectMutatorRepnoInt(
                         getter,
                         setter,
                         base_eve_name));

  AddCheck(eve_name, base_eve_name, base_id);
  AddRepnoFinale(eve_name, base_eve_name, base_id);
}

void ObjectModule::AddCheck(const std::string& eve_name,
                            const std::string& base_eve_name,
                            int base_id) {
  string check_name = eve_name + "Check";
  module_->AddOpcode(3000 + base_id, 0, check_name,
                     new Op_MutatorCheck(base_eve_name));
}

void ObjectModule::AddNormalFinale(const std::string& eve_name,
                                   const std::string& base_eve_name,
                                   int base_id) {
  string wait_name = eve_name + "Wait";
  module_->AddOpcode(4000 + base_id, 0, wait_name,
                     new Op_MutatorWaitNormal(base_eve_name));

  string waitc_name = eve_name + "WaitC";
  module_->AddOpcode(5000 + base_id, 0, waitc_name,
                     new Op_MutatorWaitCNormal(base_eve_name));

  string end_name = eve_name + "End";
  module_->AddOpcode(6000 + base_id, 0, end_name,
                     new Op_EndObjectMutation_Normal(base_eve_name));
}

void ObjectModule::AddRepnoFinale(const std::string& eve_name,
                                  const std::string& base_eve_name,
                                  int base_id) {
  string wait_name = eve_name + "Wait";
  module_->AddOpcode(4000 + base_id, 0, wait_name,
                     new Op_MutatorWaitRepNo(base_eve_name));

  string waitc_name = eve_name + "WaitC";
  module_->AddOpcode(5000 + base_id, 0, waitc_name,
                     new Op_MutatorWaitCRepNo(base_eve_name));

  string end_name = eve_name + "End";
  module_->AddOpcode(6000 + base_id, 0, end_name,
                     new Op_EndObjectMutation_RepNo(base_eve_name));
}
