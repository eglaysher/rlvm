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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// -----------------------------------------------------------------------

#ifndef SRC_MACHINEBASE_MAPPEDRLMODULE_HPP_
#define SRC_MACHINEBASE_MAPPEDRLMODULE_HPP_

#include <string>

#include "MachineBase/RLModule.hpp"
#include <boost/function.hpp>

// -----------------------------------------------------------------------

/**
 * Special case RLModule where each opcode added is transformed with a
 * mapping function.
 */
class MappedRLModule : public RLModule {
 public:
  typedef boost::function<RLOperation*(RLOperation* op)> MappingFunction;

 protected:
  MappedRLModule(const MappingFunction& fun, const std::string& in_module_name,
                 int in_module_type, int in_module_number);
  ~MappedRLModule();

  /** Adds a map_function_(op) to this modules set of opcodes.
   *
   * @note The RLModule class takes ownership of any RLOperation
   * objects passed in this way.
   *
   * @param opcode The opcode number of this operation
   * @param overload The overload number of this operation
   * @param name A cstring with the printable name (or NULL)
   * @param op An RLOperation functor which represents the
   *           implementation of this operation.
   */
  virtual void addOpcode(int opcode, unsigned char overload, const char* name,
                         RLOperation* op);

 private:
  /**
   * Function which takes an RLOperation and returns an RLOperation. Takes
   * ownership of the incoming RLOperation.
   */
  MappingFunction map_function_;
};


#endif  // SRC_MACHINEBASE_MAPPEDRLMODULE_HPP_
