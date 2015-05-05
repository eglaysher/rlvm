// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006, 2007 Elliot Glaysher
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

#ifndef SRC_MACHINE_REFERENCE_H_
#define SRC_MACHINE_REFERENCE_H_

#include <string>
#include <iterator>

template <typename T>
class MemoryReferenceIterator;
class Memory;

// Accessor class passed back to user when the iterator is
// dereferenced. Each IntAcessor will (probably) be a short-lived
// temporary object which is immediately cast to an int, or it may
// have a value assigned to it.
class IntAccessor {
 public:
  explicit IntAccessor(MemoryReferenceIterator<IntAccessor>* i);
  ~IntAccessor();

  // Read from the memory location, and return the value.
  operator int() const;

  IntAccessor& operator=(const int new_value);
  IntAccessor& operator=(const IntAccessor& rhs);

 private:
  // Pointer to the real memory reference that we work with whenever
  // we operate with an IntAccessor
  MemoryReferenceIterator<IntAccessor>* it;

  // Pointer to the store register (if this is that kind of IntAccessor).
  int* store_register_;
};

// Accessor class passed back to user when the iterator is
// dereferenced. Each StringAccessor will (probably) be a short-lived
// temporary object which is immediately casted to an string, or it may
// have a value assigned to it.
class StringAccessor {
 public:
  explicit StringAccessor(MemoryReferenceIterator<StringAccessor>* i);
  ~StringAccessor();

  operator std::string() const;

  StringAccessor& operator=(const std::string& new_value);
  StringAccessor& operator=(const StringAccessor& new_value);

  bool operator==(const std::string& rhs);

 private:
  // Pointer to the real memory reference that we work with whenever
  // we operate with an StringAccessor.
  MemoryReferenceIterator<StringAccessor>* it;
};

// MemoryReferenceIterator represents iterators into an RLMachine's
// memory.  Since changing the RLMachine's memory can change the
// pointed memory address of a MemoryReference, we create iterators
// that point to the current state of an MemoryReference. This also
// solves the problem where some functions in RealLive accept two
// memory addresses, and do something on that range.
template <typename ACCESS>
class MemoryReferenceIterator
    : public std::iterator<std::random_access_iterator_tag, ACCESS> {
 public:
  MemoryReferenceIterator();

  // Explicit store register creation
  explicit MemoryReferenceIterator(int* store_register);

  // Explicit reference creation
  MemoryReferenceIterator(Memory* in_machine,
                          const int in_type,
                          const int in_location);

  int type() const { return type_; }
  int location() const { return location_; }

  // -------------------------------------------------------- Iterated Interface
  ACCESS operator*() { return ACCESS(this); }

  MemoryReferenceIterator& operator++() {
    ++location_;
    return *this;
  }

  MemoryReferenceIterator& operator--() {
    --location_;
    return *this;
  }

  MemoryReferenceIterator& operator+=(int step) {
    location_ += step;
    return *this;
  }

  MemoryReferenceIterator& operator-=(int step) {
    location_ -= step;
    return *this;
  }

  MemoryReferenceIterator operator++(int postfix) {
    MemoryReferenceIterator tmp(*this);
    ++location_;
    return tmp;
  }

  MemoryReferenceIterator operator--(int postfix) {
    MemoryReferenceIterator tmp(*this);
    --location_;
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
    return location_ - rhs.location_;
  }

  bool operator<(const MemoryReferenceIterator& rhs) {
    return location_ < rhs.location_;
  }

  bool operator==(const MemoryReferenceIterator<ACCESS>& rhs) const {
    return memory_ == rhs.memory_ && type_ == rhs.type_ &&
           location_ == rhs.location_;
  }

  bool operator!=(const MemoryReferenceIterator<ACCESS>& rhs) const {
    return !operator==(rhs);
  }

  MemoryReferenceIterator<ACCESS> changeMemoryTo(Memory* new_mem_obj) const {
    return MemoryReferenceIterator<ACCESS>(new_mem_obj, type_, location_);
  }

 private:
  int* store_register_ = NULL;
  Memory* memory_ = NULL;
  int type_ = -1;
  int location_ = 0;

  // Can this be templated?
  friend class StringAccessor;
  friend class IntAccessor;
};

template <typename ACCESS>
MemoryReferenceIterator<ACCESS>::MemoryReferenceIterator() {}

template <typename ACCESS>
MemoryReferenceIterator<ACCESS>::MemoryReferenceIterator(int* store_register)
    : store_register_(store_register) {}

template <typename ACCESS>
MemoryReferenceIterator<ACCESS>::MemoryReferenceIterator(Memory* memory,
                                                         const int in_type,
                                                         const int in_location)
    : memory_(memory),
      type_(in_type),
      location_(in_location) {}

// -----------------------------------------------------------------------

// Defines a MemoryReferenceIterator that operates on the numeric memory
typedef MemoryReferenceIterator<IntAccessor> IntReferenceIterator;

// Defines a MemoryReferenceIterator that operates on the string memory
typedef MemoryReferenceIterator<StringAccessor> StringReferenceIterator;

#endif  // SRC_MACHINE_REFERENCE_H_
