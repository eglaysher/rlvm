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

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

#include "MachineBase/OpcodeLog.hpp"
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <string>
#include <boost/bind.hpp>

using namespace std;

// -----------------------------------------------------------------------
// OpcodeLog
// -----------------------------------------------------------------------
OpcodeLog::OpcodeLog() {}
OpcodeLog::~OpcodeLog() {}

// -----------------------------------------------------------------------

void OpcodeLog::increment(const std::string& name) {
  storage_[name]++;
}

// -----------------------------------------------------------------------

static bool nameLessThan(const OpcodeLog::Storage::value_type& lhs,
                         const OpcodeLog::Storage::value_type& rhs) {
  return lhs.first.size() < rhs.first.size();
}

// -----------------------------------------------------------------------

std::ostream& operator<<(std::ostream& os, const OpcodeLog& log) {
  if (log.size()) {
    int max_function_name_len =
        max_element(log.begin(), log.end(), nameLessThan)->first.size();

    os << setw(max_function_name_len) << left << "Name" << "  " << "Count"
       << endl;

    for (int i = 0; i < max_function_name_len; ++i)
      os << "-";

    os << "  " << "-----" << endl;

    for (OpcodeLog::Storage::const_iterator it = log.begin(); it != log.end();
        ++it) {
      os << setw(max_function_name_len) << left <<  it->first << "  "
         << it->second
         << endl;
    }
  } else {
    os << "No undefined opcodes called!";
  }

  return os;
}
