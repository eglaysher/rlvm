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

#ifndef SRC_MODULES_MODULE_SYS_INDEX_SERIES_H_
#define SRC_MODULES_MODULE_SYS_INDEX_SERIES_H_

class RLModule;

#include "machine/rloperation.h"
#include "machine/rloperation/argc_t.h"
#include "machine/rloperation/complex_t.h"
#include "machine/rloperation/rlop_store.h"
#include "machine/rloperation/special_t.h"

// Defines index_series's input pattern.
typedef IntConstant_T ValOnly;
typedef Complex_T<IntConstant_T, IntConstant_T, IntConstant_T> StartEndval;
typedef Complex_T<IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T>
    StartEndvalMode;
typedef Special_T<DefaultSpecialMapper, ValOnly, StartEndval, StartEndvalMode>
    IndexSeriesEntry;
typedef Argc_T<IndexSeriesEntry> IndexList;

// Implementation of the math performing index_series command. Exposed through
// the header for testing.
struct Sys_index_series : public RLStoreOpcode<IntConstant_T,
                                               IntConstant_T,
                                               IntConstant_T,
                                               IndexList> {
  // Main entrypoint
  int operator()(RLMachine& machine,
                 int index,
                 int offset,
                 int init,
                 IndexList::type index_list);

  // Adds a single term.
  void Adder(int index,
             int start,
             int end,
             int endval,
             int mod,
             int& value,
             int& init,
             bool& previous_term_finished);
};

// index_series has its own file.
void AddIndexSeriesOpcode(RLModule& module);

#endif  // SRC_MODULES_MODULE_SYS_INDEX_SERIES_H_
