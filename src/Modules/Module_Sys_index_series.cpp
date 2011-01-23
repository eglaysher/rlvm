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

#include "Modules/Module_Sys_index_series.hpp"

#include <cmath>
#include <sstream>
#include <iostream>

#include "MachineBase/RLMachine.hpp"
#include "MachineBase/RLModule.hpp"

using namespace std;

// -----------------------------------------------------------------------

int Sys_index_series::operator()(RLMachine& machine,
                                 int index,
                                 int offset,
                                 int init,
                                 IndexList::type index_list) {
  index = index + offset;
  int value = init;

  for (IndexList::type::iterator it = index_list.begin();
       it != index_list.end(); ++it) {
    switch (it->type) {
      case 0: {
        throw rlvm::Exception(
            "Don't know how to handle type 0 index_series statements");
        break;
      }
      case 1: {
        // This is the only thing we reliably can do.
        int start = it->second.get<0>();
        int end = it->second.get<1>();
        int endval = it->second.get<2>();
        mode0(index, start, end, endval, value, init);
        break;
      }
      case 2: {
        int start = it->third.get<0>();
        int end = it->third.get<1>();
        int endval = it->third.get<2>();
        if (it->third.get<3>() == 0) {
          mode0(index, start, end, endval, value, init);
        } else if (it->third.get<3>() == 1) {
          mode1(index, start, end, endval, value, init);
        } else if (it->third.get<3>() == 2) {
          mode2(index, start, end, endval, value, init);
        } else {
          std::ostringstream oss;
          oss << "Don't know how to handle type " << it->third.get<3>()
              << " index_series statements";
          throw rlvm::Exception(oss.str());
        }
        break;
      }
    }
  }

  return value;
}

void Sys_index_series::mode0(int index,
                             int start,
                             int end,
                             int endval,
                             int& value,
                             int& init) {
  if (index > start && index < end) {
    double percentage = double(index - start) / double(end - start);
    int amount = endval - init;
    value += (percentage * amount);
  } else if (index >= end) {
    // Prevent us from going over the endval.
    value = endval;
    init = endval;
  }
}

void Sys_index_series::mode1(int index,
                             int start,
                             int end,
                             int endval,
                             int& value,
                             int& init) {
  if (index > start && index < end) {
    double percentage = double(index - start) / double(end - start);
    double log_percentage = std::log(percentage + 1) / std::log(2);
    int amount = endval - init;
    value += (amount - ((1 - log_percentage) * amount));
  } else if (index >= end) {
    // Prevent us from going over the endval.
    value = endval;
    init = endval;
  }
}

void Sys_index_series::mode2(int index,
                             int start,
                             int end,
                             int endval,
                             int& value,
                             int& init) {
  if (index > start && index < end) {
    double percentage = double(index - start) / double(end - start);
    double log_percentage = std::log(percentage + 1) / std::log(2);
    int amount = endval - init;
    value += (log_percentage * amount);
  } else if (index >= end) {
    // Prevent us from going over the endval.
    value = endval;
    init = endval;
  }
}

// -----------------------------------------------------------------------

void addIndexSeriesOpcode(RLModule& module) {
  module.addOpcode(800, 0, "index_series", new Sys_index_series);
}
