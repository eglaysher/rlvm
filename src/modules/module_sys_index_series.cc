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
//
// -----------------------------------------------------------------------

#include "modules/module_sys_index_series.h"

#include <sstream>
#include <tuple>

#include "machine/rlmachine.h"
#include "machine/rlmodule.h"
#include "utilities/math_util.h"

using std::get;

// -----------------------------------------------------------------------

int Sys_index_series::operator()(RLMachine& machine,
                                 int index,
                                 int offset,
                                 int init,
                                 IndexList::type index_list) {
  int value = init;
  bool previous_term_finished = false;

  for (IndexList::type::iterator it = index_list.begin();
       it != index_list.end();
       ++it) {
    switch (it->type) {
      case 0: {
        if (previous_term_finished) {
          value = it->first;
          init = it->first;
        }
        break;
      }
      case 1: {
        // This is the only thing we reliably can do.
        int start = get<0>(it->second) + offset;
        int end = get<1>(it->second) + offset;
        int endval = get<2>(it->second);
        Adder(
            index, start, end, endval, 0, value, init, previous_term_finished);
        break;
      }
      case 2: {
        int start = get<0>(it->third) + offset;
        int end = get<1>(it->third) + offset;
        int endval = get<2>(it->third);
        int mod = get<3>(it->third);
        Adder(index,
              start,
              end,
              endval,
              mod,
              value,
              init,
              previous_term_finished);
        break;
      }
    }
  }

  return value;
}

void Sys_index_series::Adder(int index,
                             int start,
                             int end,
                             int endval,
                             int mod,
                             int& value,
                             int& init,
                             bool& previous_term_finished) {
  if (index > start && index < end) {
    int amount = endval - init;
    value += Interpolate(start, index, end, amount, mod);
    previous_term_finished = false;
  } else if (index >= end) {
    // Prevent us from going over the endval.
    value = endval;
    init = endval;
    previous_term_finished = true;
  }
}

// -----------------------------------------------------------------------

void AddIndexSeriesOpcode(RLModule& module) {
  module.AddOpcode(800, 0, "index_series", new Sys_index_series);
}
