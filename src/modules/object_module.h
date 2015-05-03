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

#ifndef SRC_MODULES_OBJECT_MODULE_H_
#define SRC_MODULES_OBJECT_MODULE_H_

#include <string>

#include "machine/rlmodule.h"
#include "systems/base/graphics_object.h"

class RLModule;
class RLOperation;

class ObjectModule {
 public:
  typedef int (GraphicsObject::*NormalGetter)() const;
  typedef void (GraphicsObject::*NormalSetter)(const int);

  typedef int (GraphicsObject::*RepnoGetter)(const int) const;
  typedef void (GraphicsObject::*RepnoSetter)(const int, const int);

  ObjectModule(const std::string& prefix, RLModule* module);
  virtual ~ObjectModule();

  void AddSingleObjectCommands(int base_id, const std::string& name,
                               NormalGetter getter, NormalSetter setter);
  void AddDoubleObjectCommands(int base_id,
                               const std::string& name,
                               NormalGetter getter_one,
                               NormalSetter setter_one,
                               NormalGetter getter_two,
                               NormalSetter setter_two);
  void AddRepnoObjectCommands(int base_id,
                              const std::string& name,
                              RepnoGetter getter,
                              RepnoSetter setter);

  template <typename first, typename second>
  void AddCustomRepno(int base_id,
                      const std::string& name);

 private:
  void AddCheck(const std::string& eve_name,
                const std::string& base_eve_name,
                int base_id);

  void AddNormalFinale(const std::string& eve_name,
                       const std::string& base_eve_name,
                       int base_id);

  // The Repno commands need to have their own versions of Wait and End because
  // they the repno is part of the object identity and is passed in, changing
  // the arity.
  void AddRepnoFinale(const std::string& eve_name,
                      const std::string& base_eve_name,
                      int base_id);

  std::string prefix_;
  RLModule* module_;
};

template <typename first, typename second>
void ObjectModule::AddCustomRepno(int base_id,
                                  const std::string& name) {
  std::string base_name = prefix_ + name;
  module_->AddOpcode(1000 + base_id, 0, base_name, new first);

  std::string eve_name = prefix_ + "Eve" + name;
  std::string base_eve_name = "objEve" + name;
  module_->AddOpcode(2000 + base_id, 0, eve_name, new first);
  module_->AddOpcode(2000 + base_id, 1, eve_name, new second);

  AddCheck(eve_name, base_eve_name, base_id);
  AddRepnoFinale(eve_name, base_eve_name, base_id);
}


#endif  // SRC_MODULES_OBJECT_MODULE_H_
