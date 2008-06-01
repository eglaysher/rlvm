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

#include "MockLog.hpp"
#include "tut/tut.hpp"
#include <algorithm>

using namespace std;

// -----------------------------------------------------------------------
// MockLog::Record
// -----------------------------------------------------------------------
MockLog::Record::Record(const std::string& name, const std::string& argStr)
  : function_name_(name), argument_str_(argStr)
{}

// -----------------------------------------------------------------------

bool MockLog::Record::operator==(const Record& rhs) const {
  return function_name_ == rhs.function_name_ &&
    argument_str_ == rhs.argument_str_;
}

// -----------------------------------------------------------------------
// MockLog
// -----------------------------------------------------------------------
MockLog::MockLog(const std::string& object_name)
  : object_name_(object_name)
{}

// -----------------------------------------------------------------------

MockLog::~MockLog() {}

// -----------------------------------------------------------------------

/// Records a function with no arguments.
void MockLog::recordFunction(const std::string& name) {
  recordFunctionInternal(name, "");
}

// -----------------------------------------------------------------------

/// Records a function with no arguments.
void MockLog::ensure(const std::string& name) {
  ensureInternal(name, "");
}

// -----------------------------------------------------------------------

void MockLog::ensureInternal(const std::string& name, const std::string& arguments) const {
  Record r(name, arguments);
  bool found = std::find(records_.begin(), records_.end(), r) != records_.end();

  ostringstream oss;
  oss << "Didn't find an instance of " << name << "(" << arguments << ") in "
      << *this;
  tut::ensure(oss.str(), found);
}

// -----------------------------------------------------------------------

void MockLog::recordFunctionInternal(const std::string& name, 
                                     const std::string& arguments) {
  records_.push_back(Record(name, arguments));
}

// -----------------------------------------------------------------------

std::ostream& operator<<(std::ostream& os, const MockLog& log)
{
  os << "Log for \"" << log.object_name_ << "\"" << std::endl
     << "--------------------------------------" << std::endl;
  for (std::vector<MockLog::Record>::const_iterator it = log.records_.begin(); 
       it != log.records_.end(); ++it) {
    os << "  " << it->function_name_ << "(" << it->argument_str_ << ")" 
       << std::endl;
  }

  return os;
}
