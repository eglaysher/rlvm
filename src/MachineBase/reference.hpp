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

#ifndef __Reference_hpp__
#define __Reference_hpp__

/**
 * @file   reference.hpp
 * @author Elliot Glaysher
 * @date   Sat Oct  7 11:16:10 2006
 * 
 * @brief  Declares the iterated interface to RLMachine's memory
 */

#include <iterator>

template<typename T>
class MemoryReferenceIterator;

class RLMachine;

/** 
 * Accessor class passed back to user when the iterator is
 * dereferenced. Each IntAcessor will (probably) be a short-lived
 * temporary object which is immediatly casted to an int, or it may
 * have a value assigned to it.
 *
 * @note If you are having weird problems with code that dereferences
 * MemoryReferenceIterators, you may need to overload an operator in
 * this class.
 */
class IntAccessor {
private:
  /// Pointer to the real memory reference that we work with whenever
  /// we operate with an IntAccessor
  MemoryReferenceIterator<IntAccessor>* it;

public:
  IntAccessor(MemoryReferenceIterator<IntAccessor>* i);
  ~IntAccessor();

  operator int() const;

  IntAccessor& operator=(const int newValue);
  IntAccessor& operator=(const IntAccessor& rhs);
};

/** 
 * Accessor class passed back to user when the iterator is
 * dereferenced. Each StringAcessor will (probably) be a short-lived
 * temporary object which is immediatly casted to an string, or it may
 * have a value assigned to it.
 *
 * @note If you are having weird problems with code that dereferences
 * MemoryReferenceIterators, you may need to overload an operator in
 * this class.
 */
class StringAccessor {
private:
  /// Pointer to the real memory reference that we work with whenever
  /// we operate with an StringAccessor
  MemoryReferenceIterator<StringAccessor>* it;

public:
  StringAccessor(MemoryReferenceIterator<StringAccessor>* i);
  ~StringAccessor();

  operator std::string() const;

  StringAccessor& operator=(const std::string& newValue);
  StringAccessor& operator=(const StringAccessor& newValue);

  bool operator==(const std::string& rhs);
};

/** 
 * MemoryReferenceIterator represents iterators into an RLMachine's
 * memory.  Since changing the RLMachine's memory can change the
 * pointed memory address of a MemoryReference, we create iterators
 * that point to the current state of an MemoryReference. This also
 * solves the problem where some functions in RealLive accept two
 * memory addresses, and do something on that range.
 */
template<typename ACCESS>
class MemoryReferenceIterator
  : public std::iterator<std::random_access_iterator_tag, ACCESS> {
private:
  int m_type;
  int m_location;
  RLMachine* m_machine;
  // Can this be templated?
  friend class StringAccessor;
  friend class IntAccessor;

public:
  // Explicit reference creation
  MemoryReferenceIterator(RLMachine* inMachine, const int inType, 
                          const int inLocation);

  int type() const { return type; }
  int location() const { return location; }
  // -------------------------------------------------------- Iterated Interface
  ACCESS operator*()     { return ACCESS(this); }
  
  MemoryReferenceIterator& operator++()   { ++m_location; return *this; }
  MemoryReferenceIterator& operator--()   { --m_location; return *this; }
  MemoryReferenceIterator& operator+=(int step) { m_location += step; return *this; }
  MemoryReferenceIterator& operator-=(int step) { m_location -= step; return *this; }

  MemoryReferenceIterator operator++(int) { 
    MemoryReferenceIterator tmp(*this);
    ++m_location; 
    return tmp; 
  }
  MemoryReferenceIterator operator--(int) {
    MemoryReferenceIterator tmp(*this);
    --m_location; 
    return tmp; 
  }

  MemoryReferenceIterator operator+(int step) {
    MemoryReferenceIterator tmp(*this);
    return tmp += step;
  }
  MemoryReferenceIterator operator-(int step) {
    MemoryReferenceIterator tmp(*this);
    return tmp -= step;
  }

  int operator-(const MemoryReferenceIterator& rhs) {
    return m_location - rhs.m_location;
  }

  bool operator<(const MemoryReferenceIterator& rhs) {
    return m_location < rhs.m_location;
  }

  bool operator==(const MemoryReferenceIterator<ACCESS>& rhs) const {
    return m_machine == rhs.m_machine && m_type == rhs.m_type && 
      m_location == rhs.m_location;
  }

  bool operator!=(const MemoryReferenceIterator<ACCESS>& rhs) const {
    return ! operator==(rhs);
  }
};

// -----------------------------------------------------------------------

template<typename ACCESS>
MemoryReferenceIterator<ACCESS>::MemoryReferenceIterator(
  RLMachine* inMachine, const int inType, const int inLocation)
  : m_machine(inMachine), m_type(inType), m_location(inLocation)
{ }

// -----------------------------------------------------------------------

/// Defines a MemoryReferenceIterator that operates on the numeric memory
typedef MemoryReferenceIterator<IntAccessor> IntReferenceIterator;

/// Defines a MemoryReferenceIterator that operates on the string memory
typedef MemoryReferenceIterator<StringAccessor> StringReferenceIterator;

#endif
