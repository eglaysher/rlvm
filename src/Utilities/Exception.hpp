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

#ifndef SRC_UTILITIES_EXCEPTION_HPP_
#define SRC_UTILITIES_EXCEPTION_HPP_

#include <exception>
#include <stdexcept>
#include <string>

namespace rlvm {

class Exception : public std::exception {
 public:
  explicit Exception(std::string what);
  virtual ~Exception() throw();
  virtual const char* what() const throw();

 protected:
  std::string description;
};

class UnimplementedOpcode : public Exception {
 public:
  UnimplementedOpcode(const std::string& funName,
                      int modtype, int module, int opcode, int overload);
  UnimplementedOpcode(int modtype, int module, int opcode, int overload);
  ~UnimplementedOpcode() throw();

  // Returns the name of the function that wasn't implemented.
  const std::string& opcodeName() const { return name_; }

 private:
  // Printable name of the opcode. Either "funname (opcode<W:X:Y:Z>)"
  // or "opcode<W:X:Y:Z>".
  std::string name_;

  void setDescription();
};

}  // namespace rlvm

#endif  // SRC_UTILITIES_EXCEPTION_HPP_
