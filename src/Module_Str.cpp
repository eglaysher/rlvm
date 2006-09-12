// This file is part of RLVM, a RealLive virutal machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006 El Riot
//  
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
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
/**
 * @file
 * @ingroup ModuleStr
 * @brief Contains definitions for string handling functions.
 */

#include "Module_Str.hpp"
#include "RLOperation.hpp"

#include <cmath>
#include <iostream>

using namespace std;

/** 
 * @defgroup ModuleStr Module 010, Str.
 * @ingroup Modules
 * 
 * Module that implements string handeling opcodes in the RealLive
 * virtual machine. This moduel implements commands such as strcpy,
 * strcat, strlen, et cetera.
 * @{
 */

/** 
 * @brief Implement op<1:Str:00000, 0>, fun strcpy(str, strC).
 * 
 * Assigns the string value val to the string variable dest.
 */
struct Str_strcpy_0 : public RLOp_Void_2< StrReference_T, StrConstant_T > {
  void operator()(RLMachine& machine, StringReferenceIterator dest, string val) {
    *dest = val;
  }
};

// -----------------------------------------------------------------------

/** 
 * @brief Implement op<1:Str:00000, 1>, fun strcpy(str, strC, intC).
 * 
 * Assigns the first count characters of val to the string variable dest.
 */
struct Str_strcpy_1 : public RLOp_Void_3< StrReference_T, StrConstant_T, 
                                          IntConstant_T > {
  void operator()(RLMachine& machine, StringReferenceIterator dest, string val,
                  int count) {
    fill_n(dest, count, val);
  }
};

// -----------------------------------------------------------------------

/** 
 * @brief Implement op<1:Str:00001, 0), fun strclear(str).
 * 
 * Clears the string variable dest.
 */
struct Str_strclear_0 : public RLOp_Void_1< StrReference_T > {
  void operator()(RLMachine& machine, StringReferenceIterator dest) {
    *dest = "";
  }
};

// -----------------------------------------------------------------------

/** 
 * @brief Implement op<1:Str:00001, 1), fun strclear(str 'first', str 'last').
 * 
 * Clears all string variables in the inclusive range [first, last].
 */
struct Str_strclear_1 : public RLOp_Void_2< StrReference_T, StrReference_T > {
  void operator()(RLMachine& machine, StringReferenceIterator first,
                  StringReferenceIterator last) {
    ++last; // RL ranges are inclusive
    fill(first, last, "");
  }
};

// -----------------------------------------------------------------------

/** 
 * @brief Implement op<1:Str:00003, 0>, fun strlen(strC)
 * 
 * Returns the length of value; Double-byte characters are counted as
 * two bytes.
 */
struct Str_strlen : public RLOp_Store_1< StrConstant_T > {
  int operator()(RLMachine& machine, string value) {
    return value.size();
  }
};

// -----------------------------------------------------------------------

/**
 * @brief Implament op<1:Str:00004, 0>, fun strcmp(strC, strC)
 *
 * Returns the standard strcmp() output of the two strings, ordering
 * the strings in JIS X 0208.
 *
 * @todo THIS NEEDS TO HANDLE JSX ORDERING, NOT JUST ASCII!
 * @warning THIS NEEDS TO HANDLE JSX ORDERING, NOT JUST ASCII!
 */
struct Str_strcmp : public RLOp_Store_2< StrConstant_T, StrConstant_T> {
  int operator()(RLMachine& machine, string lhs, string rhs) {
    return strcmp(lhs.c_str(), rhs.c_str());
  }
};

// -----------------------------------------------------------------------

/** 
 * 
 * 
 */
struct Str_strsub_0 : public RLOp_Void_3< StrReference_T, StrConstant_T,
                                          IntConstant_T> {
  void operator()(RLMachine& machine, StringReferenceIterator dest, string source,
                  int length) {
    
  }
};

// -----------------------------------------------------------------------


// -----------------------------------------------------------------------

/** 
 * @brief Implement op<1:Str:00100, 0>, fun strout(strV 'val'). 
 *
 * Prints a string.
 * 
 * @note This is usually called from within long text strings as \s{intV}.
 * @todo For now, we print to cout, but once we have a display system,
 * this needs to be changed so we output to it.
 */
struct Str_strout : public RLOp_Void_1< StrConstant_T > {
  void operator()(RLMachine& machine, string value) {
    cout << value;
  }
};

// -----------------------------------------------------------------------

/** 
 * @brief Implement op<1:Str:00100, 1>, fun intout(intV 'val').  
 *
 * Prints an integer.
 * 
 * @note This is usually called from within long text strings as \i{intV}.
 * @todo For now, we print to cout, but once we have a display system,
 * this needs to be changed so we output to it.
 */
struct Str_intout : public RLOp_Void_1< IntConstant_T > {
  void operator()(RLMachine& machine, int value) {
    cout << value;
  }
};

// -----------------------------------------------------------------------

/** 
 * @brief Implement op<1:Str:00200, 1>, fun strused(str).  
 *
 * Returns 0 if the string variable var is empty, otherwise 1. 
 */
struct Str_strused : public RLOp_Store_1< StrReference_T > {
  int operator()(RLMachine& machine, StringReferenceIterator it) {
    return string(*it) != "";
  }
};

StrModule::StrModule()
  : RLModule("Str", 1, 10) 
{
  addOpcode(  0, 0, new Str_strcpy_0);
  addOpcode(  0, 1, new Str_strcpy_1);
  addOpcode(  1, 0, new Str_strclear_0);
  addOpcode(  1, 1, new Str_strclear_1);
  addOpcode(  2, 0, new Str_strlen);
  addOpcode(  3, 0, new Str_strcmp);
  addOpcode(100, 0, new Str_strout);
  addOpcode(100, 1, new Str_intout);
  addOpcode(200, 0, new Str_strused);
}

// @}
