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


#ifndef __Exception_hpp__
#define __Exception_hpp__

#include <exception>
#include <stdexcept>
#include <string>

namespace rlvm {

class Exception : public std::exception
{
protected:
  std::string description;
public:
  virtual const char* what() const throw();
  Exception(std::string what);
  virtual ~Exception() throw();
};

class UnimplementedOpcode : public Exception
{
private:
  /// Printable name of the opcode. Either "funname (opcode<W:X:Y:Z>)"
  /// or "opcode<W:X:Y:Z>".
  std::string name_;

  void setDescription();

public:
  UnimplementedOpcode(const std::string& funName,
                      int modtype, int module, int opcode, int overload);
  UnimplementedOpcode(int modtype, int module, int opcode, int overload);
  ~UnimplementedOpcode() throw();

  /// Returns the name of the function that wasn't implemented.
  const std::string& opcodeName() const { return name_; }
};

}

#endif
