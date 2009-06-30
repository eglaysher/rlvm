// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2009 Elliot Glaysher
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

#ifndef __TestMachine_hpp__
#define __TestMachine_hpp__

#include "MachineBase/RLMachine.hpp"

#include <map>
#include <string>

class System;
class RLModule;
class RLOperation;
namespace libReallive {
class Archive;
}

// A class that wraps an RLMachine, listens for module attachments, and then
// records the names of the opcodes in a structure for fast lookup by module
// name/opcode number.
class TestMachine : public RLMachine {
 public:
  TestMachine(System& in_system, libReallive::Archive& in_archive);

  // Index all the RLOperations before passing to parent.
  virtual void attachModule(RLModule* module);

  // Invokes a named opcode
  void exe(const std::string& name, unsigned char overload);

 private:
  void runOpcode(const std::string& name, unsigned char overload,
                 int argc, const std::string& argument_string);

  typedef std::map< std::pair<std::string, unsigned char>, RLOperation*>
  OpcodeRegistry;
  OpcodeRegistry registry_;
};


#endif
