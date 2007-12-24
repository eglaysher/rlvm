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

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/tracking.hpp>

#include "LazyArray.hpp"

#include "testUtils.hpp"
#include "tut.hpp"

#include <iostream>
#include <algorithm>
using namespace std;

const int SIZE = 10;

namespace tut
{

struct LazyArray_data
{
  // Helper class because boost::serialization doesn't like the
  // concept of serializing an int*
  class IntWrapper
  {
  private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
      ar & num;
    }

    int num;

  public:
    IntWrapper() : num(0) {}

    IntWrapper& operator=(int in) {
      num = in;
    }

    operator int() const {
      return num;
    }
  };

  template<typename T>
  void populateIntArray(T& lazyArray)
  {
    // Set each even slot to its number 
    for(int i = 0; i < SIZE; ++i)
      if(i % 2 == 0)
        lazyArray[i] = i;
  }

  template<typename T>
  void checkArray(LazyArray<T>& in)
  {
    // iterating across everything; every other cell should report
    // being unallocated
    FullLazyArrayIterator<T> it = in.full_begin();
    FullLazyArrayIterator<T> end = in.full_end();
    for(int i = 0; it != end && i < SIZE; ++it, ++i)
    {
      if(it.pos() % 2 == 0)
      {
        ensure("Invalid in valid position", it.valid());
        ensure_equals("Pointing to correct item", it.pos(), i);
        ensure_equals("Correct value", *it, i);
      }
      else
        ensure("Valid in invalid position", !it.valid());
    }
  }
};

typedef test_group<LazyArray_data> tf;
typedef tf::object object;
tf lazy_array_group("LazzyArray");

}

// -----------------------------------------------------------------------


BOOST_CLASS_EXPORT(tut::LazyArray_data::IntWrapper);
BOOST_CLASS_TRACKING(tut::LazyArray_data::IntWrapper, 
                     boost::serialization::track_always);

// -----------------------------------------------------------------------

namespace tut {

// -----------------------------------------------------------------------

/**
 * Test the properties of an empty array
 */
template<>
template<>
void object::test<1>()
{
  // Empty lazy arrays should simply set size
  LazyArray<int> lazyArray(SIZE);
  ensure_equals("Lazy Array didn't remember its size", lazyArray.size(), SIZE);

  ensure("Allocated Lazy iterator is valid on an empty array",
         lazyArray.allocated_begin() == lazyArray.allocated_end());

  // iterating across everything; each cell should report being unallocated
  FullLazyArrayIterator<int> it = lazyArray.full_begin();
  FullLazyArrayIterator<int> end = lazyArray.full_end();
  for(; it != end; ++it)
  {
    ensure("LazyArray with no items says something is valid", !it.valid());
  }
}

// -----------------------------------------------------------------------

template<>
template<>
void object::test<2>()
{
  LazyArray<int> lazyArray(SIZE);
  ensure_equals("Lazy Array didn't remember its size", lazyArray.size(), SIZE);
  populateIntArray(lazyArray);
  checkArray(lazyArray);

  // Test to make sure that when we use AllocatedLazyArrayIterator, we
  // only stop on items that are valid.
  AllocatedLazyArrayIterator<int> ait = lazyArray.allocated_begin();
  AllocatedLazyArrayIterator<int> aend = lazyArray.allocated_end();
  ensure("Allocated Lazy iterator is invalid on an array with items in it",
         ait != aend);

  for(int i = 0; ait != aend && i < SIZE; ++ait, i+=2)
  {
    ensure("Stopped on invalid item!", ait.pos() % 2 == 0);
    ensure_equals("Correct value", *ait, i);
  }
}

// -----------------------------------------------------------------------

template<>
template<>
void object::test<3>()
{
  // Allocate even objects and then these 
  LazyArray<int> lazyArray(SIZE);
  ensure_equals("Lazy Array didn't remember its size", lazyArray.size(), SIZE);

  // Go through each item in the array (by FullLazyArrayIterator) and
  // set the values.
  FullLazyArrayIterator<int> it = lazyArray.full_begin();
  FullLazyArrayIterator<int> end = lazyArray.full_end();
  for(int i = 0; it != end && i < SIZE; ++it, ++i)
  {
    ensure("Position starts off invalid.", !it.valid());
    *it = i;
    ensure("Position ends up valid after writing.", it.valid());
  }
  
  // Now we should be able to iterate across all the items with
  // AllocatedLazyArrayIterator, and make sure their values are correct.
  AllocatedLazyArrayIterator<int> ait = lazyArray.allocated_begin();
  AllocatedLazyArrayIterator<int> aend = lazyArray.allocated_end();
  for(int i = 0; ait != aend && i < SIZE; ++ait, ++i)
  {
    ensure_equals("Correct value in position", *ait, i);
  }
}

// -----------------------------------------------------------------------

/**
 * 
 */
template<>
template<>
void object::test<4>()
{
  stringstream ss;

  LazyArray<IntWrapper> lazyArray(SIZE);
  ensure_equals("Lazy Array didn't remember its size", lazyArray.size(), SIZE);
  populateIntArray(lazyArray);

  {
    boost::archive::text_oarchive oa(ss);
    oa << const_cast<const LazyArray<IntWrapper>&>(lazyArray);
  }

  {
    // Thaw the data that was saved into a different LazyArray and
    // make sure that it's the same data.
    LazyArray<IntWrapper> newArray(SIZE);
    boost::archive::text_iarchive ia(ss);
    ia >> newArray;
    checkArray(newArray);
  }
}


}
