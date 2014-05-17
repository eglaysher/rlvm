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

#include "systems/base/graphics_object.h"

class RLModule;
class RLOperation;

class ObjectModule {
 public:
  typedef int (GraphicsObject::*Getter)() const;
  typedef void (GraphicsObject::*Setter)(const int);

  ObjectModule(const std::string& prefix, RLModule* module);
  virtual ~ObjectModule();

  void AddSingleObjectCommands(int base_id, const std::string& name,
                               Getter getter, Setter setter);
  void AddDoubleObjectCommands(int base_id,
                               const std::string& name,
                               Getter getter_one,
                               Setter setter_one,
                               Getter getter_two,
                               Setter setter_two);

 private:
  void AddCheck(const std::string& eve_name,
                const std::string& base_eve_name,
                int base_id);

  void AddNormalFinale(const std::string& eve_name,
                       const std::string& base_eve_name,
                       int base_id);

  std::string prefix_;
  RLModule* module_;
};

#endif  // SRC_MODULES_OBJECT_MODULE_H_
