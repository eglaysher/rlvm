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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//  
// -----------------------------------------------------------------------

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

#include "Module_Sys_Date.hpp"
#include "MachineBase/RLModule.hpp"
#include "MachineBase/RLOperation.hpp"
#include "MachineBase/RLOperation/RLOp_Store.hpp"
#include "MachineBase/RLOperation/References.hpp"

#include "dateUtil.hpp"

// -----------------------------------------------------------------------

/** 
 * Implements op<1:Sys:01100, 0>, fun GetYear().
 * 
 * Returns the current four digit year.
 */
struct Sys_GetYear : public RLOp_Store_Void {
  int operator()(RLMachine& machine) {
	return datetime::getYear();
  }
};

// -----------------------------------------------------------------------

struct Sys_GetMonth : public RLOp_Store_Void {
  int operator()(RLMachine& machine) {
	return datetime::getMonth();
  }
};

// -----------------------------------------------------------------------

struct Sys_GetDay : public RLOp_Store_Void {
  int operator()(RLMachine& machine) {
	return datetime::getDay();
  }
};

// -----------------------------------------------------------------------

struct Sys_GetDayOfWeek : public RLOp_Store_Void {
  int operator()(RLMachine& machine) {
	return datetime::getDayOfWeek();
  }
};

// -----------------------------------------------------------------------

struct Sys_GetHour : public RLOp_Store_Void {
  int operator()(RLMachine& machine) {
	return datetime::getHour();
  }
};

// -----------------------------------------------------------------------

struct Sys_GetMinute : public RLOp_Store_Void {
  int operator()(RLMachine& machine) {
	return datetime::getMinute();
  }
};

// -----------------------------------------------------------------------

struct Sys_GetSecond : public RLOp_Store_Void {
  int operator()(RLMachine& machine) {
	return datetime::getSecond();
  }
};

// -----------------------------------------------------------------------

struct Sys_GetMs : public RLOp_Store_Void {
  int operator()(RLMachine& machine) {
	return datetime::getMs();
  }
};

// -----------------------------------------------------------------------

struct Sys_GetDate : public RLOp_Void_4< IntReference_T, IntReference_T,
                                         IntReference_T, IntReference_T> {
  void operator()(RLMachine& machine, IntReferenceIterator y, 
                  IntReferenceIterator m, IntReferenceIterator d,
                  IntReferenceIterator wd) {
    *y = (int)Sys_GetYear()(machine);
    *m = (int)Sys_GetMonth()(machine);
    *d = (int)Sys_GetDay()(machine);
    *wd = (int)Sys_GetDayOfWeek()(machine);
  }
};

// -----------------------------------------------------------------------

struct Sys_GetTime : public RLOp_Void_4< IntReference_T, IntReference_T,
                                         IntReference_T, IntReference_T> {
  void operator()(RLMachine& machine, IntReferenceIterator hh, 
                  IntReferenceIterator mm, IntReferenceIterator ss,
                  IntReferenceIterator ms) {
    *hh = (int)Sys_GetHour()(machine);
    *mm = (int)Sys_GetMinute()(machine);
    *ss = (int)Sys_GetSecond()(machine);
    *ms = (int)Sys_GetMs()(machine);
  }
};

// -----------------------------------------------------------------------

/**
 * @bug A random thought. Could these present a weird race condition
 *      it the call between any two calls flips the counter?
 */
struct Sys_GetDateTime : public RLOp_Void_8<
  IntReference_T, IntReference_T, IntReference_T, IntReference_T,
  IntReference_T, IntReference_T, IntReference_T, IntReference_T> 
{
  void operator()(RLMachine& machine, 
                  IntReferenceIterator y, IntReferenceIterator m, 
                  IntReferenceIterator d, IntReferenceIterator wd,
                  IntReferenceIterator hh, IntReferenceIterator mm, 
                  IntReferenceIterator ss, IntReferenceIterator ms) 
  {
    *y = (int)Sys_GetYear()(machine);
    *m = (int)Sys_GetMonth()(machine);
    *d = (int)Sys_GetDay()(machine);
    *wd = (int)Sys_GetDayOfWeek()(machine);
    *hh = (int)Sys_GetHour()(machine);
    *mm = (int)Sys_GetMinute()(machine);
    *ss = (int)Sys_GetSecond()(machine);
    *ms = (int)Sys_GetMs()(machine);    
  }
};

// -----------------------------------------------------------------------

void addSysDateOpcodes(RLModule& m)
{
  m.addOpcode(1100, 0, new Sys_GetYear);
  m.addOpcode(1101, 0, new Sys_GetMonth);
  m.addOpcode(1102, 0, new Sys_GetDay);
  m.addOpcode(1103, 0, new Sys_GetDayOfWeek);
  m.addOpcode(1104, 0, new Sys_GetHour);
  m.addOpcode(1105, 0, new Sys_GetMinute);
  m.addOpcode(1106, 0, new Sys_GetSecond);
  m.addOpcode(1107, 0, new Sys_GetMs);
  m.addOpcode(1110, 0, new Sys_GetDate);
  m.addOpcode(1111, 0, new Sys_GetTime);
  m.addOpcode(1112, 0, new Sys_GetDateTime);
}
