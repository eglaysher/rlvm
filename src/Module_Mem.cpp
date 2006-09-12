#include "Module_Mem.hpp"
#include "RLOperation.hpp"

#include <cmath>
#include <iostream>
#include <algorithm>
#include <numeric>

// For copy_n, which isn't part of the C++ standard and doesn't come on
// OSX.
#include <boost/multi_array/algorithm.hpp>

using namespace std;
using namespace boost;

struct Mem_setarray : public RLOp_Void_2< IntReference_T, Argc_T<IntConstant_T > > {
  void operator()(RLMachine& machine, IntReferenceIterator origin, 
                  vector<int> values) {
    copy(values.begin(), values.end(), origin);
  }
};

// -----------------------------------------------------------------------

struct Mem_setrng_0 : public RLOp_Void_2< IntReference_T, IntReference_T > {
  void operator()(RLMachine& machine, IntReferenceIterator first,
                  IntReferenceIterator last) {
    ++last; // RealLive ranges are inclusive
    fill(first, last, 0);
  }
};

// -----------------------------------------------------------------------

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
 * @note copy_n is not part of the C++ standard, and while it's part of 
 * STL on the machines at work, it doesn't exist on OSX's implementation,
 * so grab a copy that boost includes.
 * @warning I'm assuming Haeleth is wrong about the type of dest in
 * reallive.kfn, since it doesn't make semantic sense as written.
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

struct Mem_setrng_stepped_0
  : public RLOp_Void_3< IntReference_T, IntConstant_T, IntConstant_T > {
  void operator()(RLMachine& machine, IntReferenceIterator origin, 
                  int step, int count) {
    // Sigh. No more simple STL statements
    for(int i = 0; i < count; ++i) {
      *origin = 0;
      advance(origin, step);
    }
  }
};

// -----------------------------------------------------------------------

struct Mem_setrng_stepped_1
  : public RLOp_Void_4< IntReference_T, IntConstant_T, IntConstant_T,
                        IntConstant_T > {
  void operator()(RLMachine& machine, IntReferenceIterator origin, 
                  int step, int count, int value) {
    // Sigh. No more simple STL statements
    for(int i = 0; i < count; ++i) {
      *origin = value;
      advance(origin, step);
    }
  }
};

// -----------------------------------------------------------------------

// op<1:Mem:00005, 0> is something weird that needs to be filled in.

// -----------------------------------------------------------------------

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

struct Mem_sum : public RLOp_Store_2< IntReference_T, IntReference_T > {
  int operator()(RLMachine& machine, IntReferenceIterator first,
                  IntReferenceIterator last) {
    last++;
    return accumulate(first, last, 0);
  }
};

// -----------------------------------------------------------------------

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
