// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2008 Elliot Glaysher
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
// -----------------------------------------------------------------------

#ifndef SRC_MACHINE_REALLIVE_DLL_H_
#define SRC_MACHINE_REALLIVE_DLL_H_

#include <string>

class RLMachine;

// Interface for common RealLive DLLs which have support compiled into rlvm.
class RealLiveDLL {
 public:
  // Builds a RealLiveDLL for the DLL named |name|. Throws an exception on rlvm
  // not supporting this particular extension.
  static RealLiveDLL* BuildDLLNamed(RLMachine& machine,
                                    const std::string& name);

  virtual ~RealLiveDLL();

  // RealLive DLLs have essentially one entrypoint, taking up to five integers
  // and yielding an integer. All values not given an explicit value in the
  // bytecode default to zero.
  virtual int CallDLL(RLMachine& machine,
                      int one,
                      int two,
                      int three,
                      int four,
                      int five) = 0;

  // Returns the DLL's name.
  virtual const std::string& GetDLLName() const = 0;
};  // end of class RealLiveDLL

#endif  // SRC_MACHINE_REALLIVE_DLL_H_
