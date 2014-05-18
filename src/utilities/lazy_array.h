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

#ifndef SRC_UTILITIES_LAZY_ARRAY_H_
#define SRC_UTILITIES_LAZY_ARRAY_H_

#include <boost/checked_delete.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/serialization/split_member.hpp>

#include <algorithm>
#include <memory>
#include <ostream>
#include <stdexcept>

// Forward declaration
template <typename T>
class AllocatedLazyArrayIterator;
template <typename Value>
class FullLazyArrayIterator;

// Container that implements an array where all objects are lazily
// evaluated when first accessed. RealLive contains a large number of
// arrays of fairly heavyweight objects.
//
// For example, is the user (in the average case) *really* going to
// use all 256 object slots? It's much more efficient use of memory to
// lazily allocate the
//
// Testing with CLANNAD shows use of 90 objects allocated in the
// foreground layer at exit. Planetarian leaves 3 objects
// allocated. Kanon leaves 10.
//
// TODO(erg): Think about caching firstEntry for the iterators...
template <typename T>
class LazyArray {
 public:
  typedef FullLazyArrayIterator<T> full_iterator;
  typedef AllocatedLazyArrayIterator<T> alloc_iterator;

 public:
  // Creates an empty LazyArray with a static size.
  explicit LazyArray(int size);
  ~LazyArray();

  T& operator[](int pos);
  const T& operator[](int pos) const;

  // Returns the size of the array.
  int size() const { return size_; }

  bool exists(int index) const { return array_[index] != NULL; }

  // Deletes an object at |index| if it exists.
  void DeleteAt(int index);

  // Go through each item in the array, and deletes it. The array's
  // size is maintained.
  void Clear();

  // Copies the contents of one LazyArray to another. This method will
  // reuse already allocated objects in otherArray (and simply calling
  // T::operator=(const T&) on them), but will allocate a new object
  // if otherArray[i] == NULL.
  void CopyTo(LazyArray<T>& otherArray);

  // Iterate across all items, allocated or not. It is the users
  // responsibility to call the isValid() method on the iterator
  // before trying to use the contents.
  FullLazyArrayIterator<T> full_begin() {
    return FullLazyArrayIterator<T>(0, this);
  }
  FullLazyArrayIterator<T> full_end() {
    return FullLazyArrayIterator<T>(size_, this);
  }

  // Iterate across the already allocated items
  AllocatedLazyArrayIterator<T> begin();
  AllocatedLazyArrayIterator<T> end() {
    return AllocatedLazyArrayIterator<T>(size_, this);
  }

 private:
  int size_;
  mutable std::unique_ptr<T*[]> array_;

  template <class>
  friend class FullLazyArrayIterator;
  template <class>
  friend class AllocatedLazyArrayIterator;

  T* rawDeref(int pos);

  friend class boost::serialization::access;

  // boost::serialization loading
  template <class Archive>
  void load(Archive& ar, unsigned int version) {
    // Allocate our new array
    ar& size_;
    array_.reset(new T* [size_]);

    for (int i = 0; i < size_; ++i) {
      ar& array_[i];
    }
  }

  // boost::serialization saving
  template <class Archive>
  void save(Archive& ar, unsigned int version) const {
    // Place the total allocated size
    ar& size_;

    for (int i = 0; i < size_; ++i) {
      ar& array_[i];
    }
  }

  BOOST_SERIALIZATION_SPLIT_MEMBER()
};

// -----------------------------------------------------------------------

template <typename Value>
class FullLazyArrayIterator
    : public boost::iterator_facade<FullLazyArrayIterator<Value>,
                                    Value,
                                    boost::forward_traversal_tag> {
 public:
  FullLazyArrayIterator() : current_position_(0), array_(0) {}

  FullLazyArrayIterator(int pos, LazyArray<Value>* array)
      : current_position_(pos), array_(array) {}

  template <class OtherValue>
  explicit FullLazyArrayIterator(FullLazyArrayIterator<OtherValue> const& other)
      : current_position_(other.current_position_), array_(other.array_) {}

  // Test to see if this item has been allocated yet, allowing the
  // user to skip
  bool valid() const { return array_->rawDeref(current_position_) != NULL; }

  size_t pos() const { return current_position_; }

 private:
  friend class boost::iterator_core_access;
  template <class>
  friend class FullLazyArrayIterator;
  template <class>
  friend class LazyArray;

  template <typename OtherValue>
  bool equal(FullLazyArrayIterator<OtherValue> const& other) const {
    return current_position_ == other.current_position_ &&
           array_ == other.array_;
  }

  void increment() { current_position_++; }

  Value& dereference() const { return (*array_)[current_position_]; }

  int current_position_;
  LazyArray<Value>* array_;
};

// -----------------------------------------------------------------------

template <typename Value>
class AllocatedLazyArrayIterator
    : public boost::iterator_facade<AllocatedLazyArrayIterator<Value>,
                                    Value,
                                    boost::forward_traversal_tag> {
 public:
  AllocatedLazyArrayIterator() : current_position_(0), array_(0) {}

  explicit AllocatedLazyArrayIterator(int pos, LazyArray<Value>* array)
      : current_position_(pos), array_(array) {}

  template <class OtherValue>
  explicit AllocatedLazyArrayIterator(
      AllocatedLazyArrayIterator<OtherValue> const& other)
      : current_position_(other.current_position_), array_(other.array_) {}

  size_t pos() const { return current_position_; }

 private:
  friend class boost::iterator_core_access;
  template <class>
  friend class AllocatedLazyArrayIterator;
  template <class>
  friend class LazyArray;

  template <typename OtherValue>
  bool equal(AllocatedLazyArrayIterator<OtherValue> const& other) const {
    return current_position_ == other.current_position_ &&
           array_ == other.array_;
  }

  void increment() {
    current_position_++;
    while (current_position_ < array_->size() &&
           array_->rawDeref(current_position_) == NULL)
      current_position_++;
  }

  Value& dereference() const { return *(array_->rawDeref(current_position_)); }

  int current_position_;
  LazyArray<Value>* array_;
};

// -----------------------------------------------------------------------
// LazyArray<T> Implementation
// -----------------------------------------------------------------------

template <typename T>
LazyArray<T>::LazyArray(int size)
    : size_(size), array_(new T* [size]) {
  for (int i = 0; i < size_; ++i)
    array_[i] = NULL;
}

template <typename T>
LazyArray<T>::~LazyArray() {
  std::for_each(
      array_.get(), array_.get() + size_, std::default_delete<T>());
}

template <typename T>
T* LazyArray<T>::rawDeref(int pos) {
  return array_[pos];
}

template <typename T>
T& LazyArray<T>::operator[](int pos) {
  if (pos < 0 || pos >= size_)
    throw std::out_of_range("LazyArray::operator[]");

  if (array_[pos] == NULL) {
    array_[pos] = new T();
  }

  return *(array_[pos]);
}

template <typename T>
const T& LazyArray<T>::operator[](int pos) const {
  if (pos < 0 || pos >= size_)
    throw std::out_of_range("LazyArray::operator[]");

  if (array_[pos] == NULL) {
    array_[pos] = new T();
  }

  return *(array_[pos]);
}

template <typename T>
void LazyArray<T>::DeleteAt(int i) {
  boost::checked_delete<T>(array_[i]);
  array_[i] = NULL;
}

template <typename T>
void LazyArray<T>::Clear() {
  for (int i = 0; i < size_; ++i) {
    boost::checked_delete<T>(array_[i]);
    array_[i] = NULL;
  }
}

template <typename T>
void LazyArray<T>::CopyTo(LazyArray<T>& otherArray) {
  if (otherArray.size_ < size_)
    throw std::runtime_error(
        "Not enough space in target array in LazyArray::copyTo");

  otherArray.size_ = size_;
  for (int i = 0; i < size_; ++i) {
    T* srcEntry = rawDeref(i);
    T* dstEntry = otherArray.rawDeref(i);

    if (srcEntry && !dstEntry) {
      otherArray.array_[i] = new T(*srcEntry);
    } else if (!srcEntry && dstEntry) {
      boost::checked_delete<T>(otherArray.array_[i]);
      otherArray.array_[i] = NULL;
    } else if (srcEntry && dstEntry) {
      *dstEntry = *srcEntry;
    }
  }
}

template <typename T>
AllocatedLazyArrayIterator<T> LazyArray<T>::begin() {
  // Find the first
  int firstEntry = 0;
  while (firstEntry < size_ && array_[firstEntry] == NULL)
    firstEntry++;

  return AllocatedLazyArrayIterator<T>(firstEntry, this);
}

#endif  // SRC_UTILITIES_LAZY_ARRAY_H_
