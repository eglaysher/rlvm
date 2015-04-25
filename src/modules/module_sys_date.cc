// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2007 Elliot Glaysher
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

#include "modules/module_sys_date.h"

#include "machine/general_operations.h"
#include "machine/rlmodule.h"
#include "machine/rloperation.h"
#include "machine/rloperation/rlop_store.h"
#include "machine/rloperation/references.h"
#include "utilities/date_util.h"

namespace {

struct GetDate : public RLOpcode<IntReference_T,
                                    IntReference_T,
                                    IntReference_T,
                                    IntReference_T> {
  void operator()(RLMachine& machine,
                  IntReferenceIterator y,
                  IntReferenceIterator m,
                  IntReferenceIterator d,
                  IntReferenceIterator wd) {
    *y = datetime::GetYear();
    *m = datetime::GetMonth();
    *d = datetime::GetDay();
    *wd = datetime::GetDayOfWeek();
  }
};

struct GetTime : public RLOpcode<IntReference_T,
                                    IntReference_T,
                                    IntReference_T,
                                    IntReference_T> {
  void operator()(RLMachine& machine,
                  IntReferenceIterator hh,
                  IntReferenceIterator mm,
                  IntReferenceIterator ss,
                  IntReferenceIterator ms) {
    *hh = datetime::GetHour();
    *mm = datetime::GetMinute();
    *ss = datetime::GetSecond();
    *ms = datetime::GetMs();
  }
};

struct GetDateTime : public RLOpcode<IntReference_T,
                                        IntReference_T,
                                        IntReference_T,
                                        IntReference_T,
                                        IntReference_T,
                                        IntReference_T,
                                        IntReference_T,
                                        IntReference_T> {
  void operator()(RLMachine& machine,
                  IntReferenceIterator y,
                  IntReferenceIterator m,
                  IntReferenceIterator d,
                  IntReferenceIterator wd,
                  IntReferenceIterator hh,
                  IntReferenceIterator mm,
                  IntReferenceIterator ss,
                  IntReferenceIterator ms) {
    *y = datetime::GetYear();
    *m = datetime::GetMonth();
    *d = datetime::GetDay();
    *wd = datetime::GetDayOfWeek();
    *hh = datetime::GetHour();
    *mm = datetime::GetMinute();
    *ss = datetime::GetSecond();
    *ms = datetime::GetMs();
  }
};

}  // namespace

void AddSysDateOpcodes(RLModule& m) {
  m.AddOpcode(1100, 0, "GetYear", ReturnIntValue(datetime::GetYear));
  m.AddOpcode(1101, 0, "GetMonth", ReturnIntValue(datetime::GetMonth));
  m.AddOpcode(1102, 0, "GetDay", ReturnIntValue(datetime::GetDay));
  m.AddOpcode(1103, 0, "GetDayOfWeek", ReturnIntValue(datetime::GetDayOfWeek));
  m.AddOpcode(1104, 0, "GetHour", ReturnIntValue(datetime::GetHour));
  m.AddOpcode(1105, 0, "GetMinute", ReturnIntValue(datetime::GetMinute));
  m.AddOpcode(1106, 0, "GetSecond", ReturnIntValue(datetime::GetSecond));
  m.AddOpcode(1107, 0, "GetMs", ReturnIntValue(datetime::GetMs));
  m.AddOpcode(1110, 0, "GetDate", new GetDate);
  m.AddOpcode(1111, 0, "GetTime", new GetTime);
  m.AddOpcode(1112, 0, "GetDateTime", new GetDateTime);
}
