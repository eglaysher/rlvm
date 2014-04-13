// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006 Elliot Glaysher
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

#ifndef SRC_SYSTEMS_BASE_SYSTEM_ERROR_H_
#define SRC_SYSTEMS_BASE_SYSTEM_ERROR_H_

#include <exception>
#include <string>

// Kill the program Errors. This exception class should only be used
// in cases where there is corrupted state or something; most things
// that a bad opcode could do should not fall under this class, and
// should be rlvm::Exception instead.
class SystemError : public std::exception {
 public:
  explicit SystemError(std::string what);
  virtual ~SystemError() throw();
  virtual const char* what() const throw() override;

 private:
  std::string description;
};

#endif  // SRC_SYSTEMS_BASE_SYSTEM_ERROR_H_
