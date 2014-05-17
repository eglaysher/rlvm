// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2007 Elliot Glaysher
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

#ifndef SRC_MACHINE_MAPPED_RLMODULE_H_
#define SRC_MACHINE_MAPPED_RLMODULE_H_

#include <functional>
#include <string>

#include "machine/rlmodule.h"

// Special case RLModule where each opcode added is transformed with a
// mapping function.
class MappedRLModule : public RLModule {
 public:
  typedef std::function<RLOperation*(RLOperation* op)> MappingFunction;

 protected:
  MappedRLModule(const MappingFunction& fun,
                 const std::string& in_module_name,
                 int in_module_type,
                 int in_module_number);
  virtual ~MappedRLModule();

  // Adds a map_function_(op) to this modules set of opcodes. Takes ownership
  // of |op|.
  virtual void AddOpcode(int opcode,
                         unsigned char overload,
                         const std::string& name,
                         RLOperation* op) override;

 private:
  // Function which takes an RLOperation and returns an RLOperation. Takes
  // ownership of the incoming RLOperation.
  MappingFunction map_function_;
};

#endif  // SRC_MACHINE_MAPPED_RLMODULE_H_
