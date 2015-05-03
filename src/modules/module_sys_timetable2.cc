// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2013 Elliot Glaysher
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

#include "modules/module_sys_timetable2.h"

#include <cmath>

#include "machine/rlmachine.h"
#include "machine/rlmodule.h"
#include "utilities/exception.h"
#include "utilities/math_util.h"

// static
int TimeTableMapper::GetTypeForTag(const libreallive::ExpressionPiece& sp) {
  switch (sp.GetOverloadTag()) {
    case 48:
      return 0;
    case 65584:
      return 1;
    case 49:
      return 2;
    case 50:
      return 3;
    case 51:
      return 4;
    case 52:
      return 5;
    case 53:
      return 6;
    case 54:
      return 7;
    case 55:
      return 8;
    default: {
      std::ostringstream oss;
      oss << "Invalid timetable2 tag: " << sp.GetOverloadTag();
      throw rlvm::Exception(oss.str());
    }
  }
}

// -----------------------------------------------------------------------

int Sys_timetable2::operator()(RLMachine& machine,
                               int now_time,
                               int rep_time,
                               int start_time,
                               int start_num,
                               TimeTable2List::type index_list) {
  now_time = now_time + rep_time;

  if (start_time > now_time)
    return start_num;

  int value = start_num;

  for (TimeTable2List::type::iterator it = index_list.begin();
       it != index_list.end();
       ++it) {
    switch (it->type) {
      case 0: {
        int end_time = std::get<0>(it->first);
        int end_num = std::get<1>(it->first);
        if (now_time > start_time && now_time <= end_time) {
          return InterpolateBetween(start_time, now_time, end_time,
                                    value, end_num, 0);
        } else {
          value = end_num;
        }

        start_time = end_time;
        break;
      }
      case 1: {
        int end_time = std::get<0>(it->second);
        int end_num = std::get<1>(it->second);
        int mod = std::get<2>(it->second);

        if (now_time > start_time && now_time <= end_time) {
          return InterpolateBetween(start_time, now_time, end_time,
                                    value, end_num, mod);
        } else {
          value = end_num;
        }

        start_time = end_time;
        break;
      }
      case 2: {
        value = it->third;
        break;
      }
      case 3: {
        int end_time = it->fourth;
        if (now_time > start_time && now_time <= end_time) {
          return value;
        }

        start_time = end_time;
        break;
      }
      case 7: {
        int end_time = std::get<0>(it->eighth);
        int end_num = std::get<1>(it->eighth);
        if (now_time > start_time && now_time <= end_time) {
          int count = std::get<2>(it->eighth);
          return Jump(start_time, now_time, end_time, value, end_num, count);
        } else {
          value = end_num;
        }

        // Finally, set the start time to our end time.
        start_time = end_time;
        break;
      }
      case 8: {
        int end_time = std::get<0>(it->ninth);
        int end_num = std::get<1>(it->ninth);

        if (now_time > start_time && now_time <= end_time) {
          return value;
        } else {
          value = end_num;
        }

        start_time = end_time;
        break;
      }
      default: {
        std::ostringstream oss;
        oss << "We don't handle " << it->type << " yet.";
        throw rlvm::Exception(oss.str());
      }
    }
  }

  return value;
}

int Sys_timetable2::Jump(int start_time,
                         int now_time,
                         int end_time,
                         int start_num,
                         int end_num,
                         int count) {
  int duration = end_time - start_time;
  int one_cycle = duration / count;

  int current_time = now_time - start_time;
  int current_cycle = current_time % one_cycle;

  double percent = static_cast<double>(current_cycle) / one_cycle;
  return start_num +
         ((end_num - start_num) * (1 - std::pow(percent * 2 - 1, 2)));
}

// -----------------------------------------------------------------------

struct Sys_timetablelen2 : public Sys_timetable2 {
  virtual int operator()(RLMachine& machine,
                         int now_time,
                         int rep_time,
                         int start_time,
                         int start_num,
                         TimeTable2List::type index_list) {
    // Modify each part of the index list to keep a running sum of times. This
    // changes a list of durations into a list of absolute times.
    int total = 0;

    for (TimeTable2List::type::iterator it = index_list.begin();
         it != index_list.end();
         ++it) {
      switch (it->type) {
        case 0: {
          total += std::get<0>(it->first);
          std::get<0>(it->first) = total;
          break;
        }
        case 1: {
          total += std::get<0>(it->second);
          std::get<0>(it->second) = total;
          break;
        }
        case 2: {
          // Set has no time, therefore it isn't modified.
          break;
        }
        case 3: {
          total += it->fourth;
          it->fourth = total;
          break;
        }
        case 7: {
          total += std::get<0>(it->eighth);
          std::get<0>(it->eighth) = total;
          break;
        }
        case 8: {
          total += std::get<0>(it->ninth);
          std::get<0>(it->ninth) = total;
          break;
        }
        default: {
          std::ostringstream oss;
          oss << "We don't handle " << it->type << " yet.";
          throw rlvm::Exception(oss.str());
        }
      }
    }

    return Sys_timetable2::operator()(
        machine, now_time, rep_time, start_time, start_num, index_list);
  }
};

// -----------------------------------------------------------------------

void AddTimetable2Opcode(RLModule& module) {
  module.AddOpcode(810, 0, "timetable2", new Sys_timetable2);
  module.AddOpcode(811, 0, "timetablelen2", new Sys_timetablelen2);
}
