// This file is part of RLVM, a RealLive virtual machine clone.
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

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

/**
 * @file    Module_Mem.cpp
 * @author  Elliot Glaysher
 * @date    Sat Oct  7 11:18:42 2006
 * @ingroup ModulesOpcodes
 * @brief   Defines the Memory module (mod<1:11>).
 */

#include "Modules/Module_Mem.hpp"
#include "MachineBase/RLOperation.hpp"

#include <cmath>
#include <iostream>
#include <algorithm>
#include <numeric>

// For copy_n, which isn't part of the C++ standard and doesn't come on
// OSX.
#include <boost/multi_array/algorithm.hpp>

using namespace std;
using namespace boost;

/**
 * @defgroup ModuleMemory The Memory Modules (mod<1:11>)
 * @ingroup ModulesOpcodes
 *
 * Module that describes memory manipulation opcodes in the RealLive
 * virtual machine. This module implements commands such as setarray,
 * cpyrng, sums, et cetera.
 *
 * @{
 */

/** 
 * Implement op<1:Mem:00000, 0>, fun setarray(int, intC+).
 * 
 * Sets a block of integers, starting with origin, to the given
 * values. values is an arbitrary number of integer expressions, each
 * of which is assigned in turn to the next variable.
 */
struct Mem_setarray : public RLOp_Void_2< IntReference_T, Argc_T<IntConstant_T > > {
  void operator()(RLMachine& machine, IntReferenceIterator origin, 
                  vector<int> values) {
    copy(values.begin(), values.end(), origin);
  }
};

// -----------------------------------------------------------------------

/** 
 * Implement op<1:Mem:00001, 0>, fun setrng(int, int).
 * 
 * Set block of integers to zero.
 */
struct Mem_setrng_0 : public RLOp_Void_2< IntReference_T, IntReference_T > {
  void operator()(RLMachine& machine, IntReferenceIterator first,
                  IntReferenceIterator last) {
    ++last; // RealLive ranges are inclusive
    fill(first, last, 0);
  }
};

// -----------------------------------------------------------------------

/** 
 * Implement op<1:Mem:00001, 1>, fun setrng(int, int, intC).
 * 
 * Set block of integers to the constant passed in.
 */
struct Mem_setrng_1 : public RLOp_Void_3< IntReference_T, IntReference_T, 
                                          IntConstant_T > {
  void operator()(RLMachine& machine, IntReferenceIterator first,
                  IntReferenceIterator last, int value) {
    ++last; // RealLive ranges are inclusive
    fill(first, last, value);
  }
};

// -----------------------------------------------------------------------

/** 
 * Implement op<1:Mem:00002, 0>, fun(int, intC, intC).
 *
 * Copies a block of values of length count from source to dest. The
 * function appears to succeed even if the ranges overlap.
 *
 * @note copy_n is not part of the C++ standard, and while it's part of 
 * STL on the machines at work, it doesn't exist on OSX's implementation,
 * so grab a copy that boost includes.
 */
struct Mem_cpyrng : public RLOp_Void_3< IntReference_T, IntReference_T,
                                        IntConstant_T > {
  void operator()(RLMachine& machine, IntReferenceIterator source,
                  IntReferenceIterator dest, int count) {
    vector<int> tmpCopy;
    boost::detail::multi_array::copy_n(source, count, back_inserter(tmpCopy));
    std::copy(tmpCopy.begin(), tmpCopy.end(), dest);
  }
};

// -----------------------------------------------------------------------

/** 
 * Implement op<1:Mem:00003, 0>, fun setarray_stepped(int, int, intC+).
 * 
 * Sets every stepth memory block starting at origin with the sequence
 * of passed in values.
 */
struct Mem_setarray_stepped 
  : public RLOp_Void_3< IntReference_T, IntConstant_T, Argc_T<IntConstant_T > > {
  void operator()(RLMachine& machine, IntReferenceIterator origin, 
                  int step, vector<int> values) {
    // Sigh. No more simple STL statements
    for(vector<int>::iterator it = values.begin(); it != values.end(); ++it) {
      *origin = *it;
      advance(origin, step);
    }
  }
};

// -----------------------------------------------------------------------

/** 
 * Implement op<1:Mem:00004, 0>, fun setrng_stepped(int, intC, intC).
 * 
 * Sets count number of memory locations to zero, starting at origin
 * and going forward step.
 */
struct Mem_setrng_stepped_0
  : public RLOp_Void_3< IntReference_T, IntConstant_T, IntConstant_T > {
  void operator()(RLMachine& machine, IntReferenceIterator origin, 
                  int step, int count) {
    for(int i = 0; i < count; ++i) {
      *origin = 0;
      advance(origin, step);
    }
  }
};

// -----------------------------------------------------------------------

/** 
 * Implement op<1:Mem:00004, 1>, fun setrng_stepped(int, intC, intC, intC).
 * 
 * Sets count number of memory locations to the passed in constant,
 * starting at origin and going forward step.
 */
struct Mem_setrng_stepped_1
  : public RLOp_Void_4< IntReference_T, IntConstant_T, IntConstant_T,
                        IntConstant_T > {
  void operator()(RLMachine& machine, IntReferenceIterator origin, 
                  int step, int count, int value) {
    for(int i = 0; i < count; ++i) {
      *origin = value;
      advance(origin, step);
    }
  }
};

// -----------------------------------------------------------------------

// op<1:Mem:00005, 0> is something weird that nobody understands.

// -----------------------------------------------------------------------

/** 
 * Implement op<1:Mem:00006, 0>, fun cpyvars(int, intC, int+).
 * 
 * I'm not even going to try for this one. See RLDev.
 */
struct Mem_cpyvars : public RLOp_Void_3< IntReference_T, IntConstant_T,
                                         Argc_T< IntReference_T > > {
  void operator()(RLMachine& machine, IntReferenceIterator origin, 
                  int offset, vector<IntReferenceIterator> values) {
    for(vector<IntReferenceIterator>::iterator it = values.begin(); 
        it != values.end(); ++it) {
      IntReferenceIterator irIt = *it;
      advance(irIt, offset);
      *origin++ = *irIt;
    }    
  }
};

// -----------------------------------------------------------------------

/** 
 * Implement op<1:Mem:00100, 0>, fun sum(int, int).
 * 
 * Returns the sum of all the numbers in the given memory range.
 */
struct Mem_sum : public RLOp_Store_2< IntReference_T, IntReference_T > {
  int operator()(RLMachine& machine, IntReferenceIterator first,
                  IntReferenceIterator last) {
    last++;
    return accumulate(first, last, 0);
  }
};

// -----------------------------------------------------------------------

/** 
 * Implement op<1:Mem:00101, 0>, fun sums((int, int)+).
 * 
 * Returns the sum of all the numbers in all the given memory ranges.
 */
struct Mem_sums : public RLOp_Store_1< Argc_T< Complex2_T< IntReference_T,
                                                         IntReference_T > > >{
  int operator()(RLMachine& machine, 
                 vector<tuple<IntReferenceIterator, IntReferenceIterator> > ranges) {
    int total = 0;
    for(vector<tuple<IntReferenceIterator, IntReferenceIterator> >::iterator it =
          ranges.begin(); it != ranges.end(); ++it) {
      IntReferenceIterator last = it->get<1>();
      ++last;
      total += accumulate(it->get<0>(), last, 0);
    }
    return total;
  }
};

// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------

MemModule::MemModule()
  : RLModule("Mem", 1, 11)
{
  addOpcode(0, 0, new Mem_setarray);
  addOpcode(1, 0, new Mem_setrng_0);
  addOpcode(1, 1, new Mem_setrng_1);
  addOpcode(2, 0, new Mem_cpyrng);
  addOpcode(3, 0, new Mem_setarray_stepped);
  addOpcode(4, 0, new Mem_setrng_stepped_0);
  addOpcode(4, 1, new Mem_setrng_stepped_1);
  // implement op<1:Mem:00005, 0>
  addOpcode(6, 0, new Mem_cpyvars);
  addOpcode(100, 0, new Mem_sum);
  addOpcode(101, 0, new Mem_sums);
}

// @}
