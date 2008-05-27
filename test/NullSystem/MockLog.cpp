// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//

#include "MockLog.hpp"

// -----------------------------------------------------------------------
// MockLog::Record
// -----------------------------------------------------------------------
MockLog::Record::Record(const std::string& name, const std::string& argStr)
  : function_name_(name), argument_str_(argStr)
{}

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
