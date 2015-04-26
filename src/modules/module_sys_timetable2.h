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

#ifndef SRC_MODULES_MODULE_SYS_TIMETABLE2_H_
#define SRC_MODULES_MODULE_SYS_TIMETABLE2_H_

class RLModule;

#include "machine/rloperation.h"
#include "machine/rloperation/argc_t.h"
#include "machine/rloperation/complex_t.h"
#include "machine/rloperation/rlop_store.h"
#include "machine/rloperation/special_t.h"

// Maps the time table commands high tags down to normal.
struct TimeTableMapper {
  static int GetTypeForTag(const libreallive::ExpressionPiece& sp);
};

// Defines timetable2's input pattern.
typedef Complex_T<IntConstant_T, IntConstant_T> TT_Move2;
typedef Complex_T<IntConstant_T, IntConstant_T, IntConstant_T> TT_Move3;
typedef IntConstant_T TT_Set;
typedef IntConstant_T TT_Wait;
typedef Complex_T<IntConstant_T, IntConstant_T, IntConstant_T> TT_Turn;
typedef Complex_T<IntConstant_T, IntConstant_T, IntConstant_T> TT_Turnup;
typedef Complex_T<IntConstant_T, IntConstant_T, IntConstant_T> TT_Loop;
typedef Complex_T<IntConstant_T, IntConstant_T, IntConstant_T> TT_Jump;
typedef Complex_T<IntConstant_T, IntConstant_T> TT_WaitSet;

typedef Special_T<TimeTableMapper,
                  TT_Move2,
                  TT_Move3,
                  TT_Set,
                  TT_Wait,
                  TT_Turn,
                  TT_Turnup,
                  TT_Loop,
                  TT_Jump,
                  TT_WaitSet> TimeTable2Entry;
typedef Argc_T<TimeTable2Entry> TimeTable2List;

// Implementation of the math performing timetable2 command.
struct Sys_timetable2 : public RLStoreOpcode<IntConstant_T,
                                             IntConstant_T,
                                             IntConstant_T,
                                             IntConstant_T,
                                             TimeTable2List> {
  // Main entrypoint
  virtual int operator()(RLMachine& machine,
                         int now_time,
                         int rep_time,
                         int start_time,
                         int start_num,
                         TimeTable2List::type index_list) override;

  int Jump(int start_time,
           int now_time,
           int end_time,
           int start_num,
           int end_num,
           int count);
};

// index_series has its own file.
void AddTimetable2Opcode(RLModule& module);

#endif  // SRC_MODULES_MODULE_SYS_TIMETABLE2_H_
