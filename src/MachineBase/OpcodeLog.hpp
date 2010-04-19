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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// -----------------------------------------------------------------------

#ifndef SRC_MACHINEBASE_OPCODELOG_HPP_
#define SRC_MACHINEBASE_OPCODELOG_HPP_

#include <iosfwd>
#include <map>
#include <string>

// An optional component to an RLMachine that counts the number of instnaces of
// an opcode. An OpcodeLog can be used to count the number of times an opcode
// was run during the lifetime of a program, or the number of times an
// undefined opcode was encountered, et cetera.
class OpcodeLog {
 public:
  /// Storage type
  typedef std::map<std::string, int> Storage;

 public:
  OpcodeLog();
  ~OpcodeLog();

  // Increments the number of times we've encountered "name".
  void increment(const std::string& name);

  Storage::const_iterator begin() const { return storage_.begin(); }
  Storage::const_iterator end() const { return storage_.end(); }
  size_t size() const { return storage_.size(); }

 private:
  // Counts the instances of an opcode encountered.
  Storage storage_;
};

// Pretty prints the contents of an OpcodeLog.
std::ostream& operator<<(std::ostream& os, const OpcodeLog& log);

#endif  // SRC_MACHINEBASE_OPCODELOG_HPP_
