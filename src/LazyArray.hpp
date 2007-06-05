// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006 Elliot Glaysher
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

#ifndef __LazyArray_hpp__
#define __LazyArray_hpp__

#include <stdexcept>
#include <boost/iterator/iterator_facade.hpp>

// Forward declaration
template<typename T>
class AllocatedLazyArrayIterator;
template<typename Value>
class FullLazyArrayIterator;

// -----------------------------------------------------------------------

/**
 * Container that implements an array where all objects are lazily
 * evaluated when first accessed. RealLive contains a large number of
 * arrays of fairly heavyweight objects.
 *
 * For example, is the user (in the average case) *really* going to
 * use all 256 object slots? It's much more efficient use of memory to
 * lazily allocate the 
 *
 * @todo Think about caching firstEntry for the iterators...
 */
template<typename T>
class LazyArray
{
public:
  typedef FullLazyArrayIterator<T> fullIterator;
  typedef AllocatedLazyArrayIterator<T> allocIterator;

private:
  int m_size;
  mutable T** m_array;

  template<class> friend class FullLazyArrayIterator;
  template<class> friend class AllocatedLazyArrayIterator;

  T* rawDeref(int pos);

public:
  LazyArray(int size);
  ~LazyArray();

  T& operator[](int pos);
  const T& operator[](int pos) const;

  int size() const { return m_size; }

  // Iterate across all items, allocated or not. It is the users
  // responsibility to call the isValid() method on the iterator
  // before trying to use the contents.
  FullLazyArrayIterator<T> full_begin()
  { return FullLazyArrayIterator<T>(0, this); }
  FullLazyArrayIterator<T> full_end()
  { return FullLazyArrayIterator<T>(m_size, this); }

  // Iterate across the already allocated items
  AllocatedLazyArrayIterator<T> allocated_begin();
  AllocatedLazyArrayIterator<T> allocated_end() 
  { return AllocatedLazyArrayIterator<T>(m_size, this); }
};

// -----------------------------------------------------------------------

template<typename Value>
class FullLazyArrayIterator 
  : public boost::iterator_facade<
        FullLazyArrayIterator<Value>
      , Value
      , boost::forward_traversal_tag
    >
{
public:
  FullLazyArrayIterator()
    : m_currentPosition(0), m_array(0) {}

  explicit FullLazyArrayIterator(int pos, LazyArray<Value>* array)
    : m_currentPosition(pos), m_array(array) {}

  template<class OtherValue>
  FullLazyArrayIterator(FullLazyArrayIterator<OtherValue> const& other)
    : m_currentPosition(other.m_currentPosition), 
      m_array(other.m_array)
  {}

  /// Test to see if this item has been allocated yet, allowing the
  /// user to skip
  bool valid() const
  {
    return m_array->rawDeref(m_currentPosition) != NULL;
  }

  size_t pos() const { return m_currentPosition; }

private:
  friend class boost::iterator_core_access;
  template<class> friend class FullLazyArrayIterator;
  template<class> friend class LazyArray;

  template<typename OtherValue>
  bool equal(FullLazyArrayIterator<OtherValue> const& other) const
  {
    return m_currentPosition == other.m_currentPosition &&
      m_array == other.m_array;
  }

  void increment()
  {
    m_currentPosition++;
  }

  Value& dereference() const
  {
    return (*m_array)[m_currentPosition];
  }

  int m_currentPosition;
  LazyArray<Value>* m_array;
};

// -----------------------------------------------------------------------

template<typename Value>
class AllocatedLazyArrayIterator 
  : public boost::iterator_facade<
        AllocatedLazyArrayIterator<Value>
      , Value
      , boost::forward_traversal_tag
    >
{
public:
  AllocatedLazyArrayIterator()
    : m_currentPosition(0), m_array(0) {}

  explicit AllocatedLazyArrayIterator(int pos, LazyArray<Value>* array)
    : m_currentPosition(pos), m_array(array) {}

  template<class OtherValue>
  AllocatedLazyArrayIterator(AllocatedLazyArrayIterator<OtherValue> const& other)
    : m_currentPosition(other.m_currentPosition), 
      m_array(other.m_array)
  {}

private:
  friend class boost::iterator_core_access;
  template<class> friend class AllocatedLazyArrayIterator;
  template<class> friend class LazyArray;

  template<typename OtherValue>
  bool equal(AllocatedLazyArrayIterator<OtherValue> const& other) const
  {
    return m_currentPosition == other.m_currentPosition &&
      m_array == other.m_array;
  }

  void increment()
  {
    m_currentPosition++;
    while(m_currentPosition < m_array->size() &&
          m_array->rawDeref(m_currentPosition) == NULL)
      m_currentPosition++;
  }

  Value& dereference() const
  {
    return *(m_array->rawDeref(m_currentPosition));
  }

  int m_currentPosition;
  LazyArray<Value>* m_array;
};

// -----------------------------------------------------------------------
// LazyArray<T> Implementation
// -----------------------------------------------------------------------

template<typename T>
LazyArray<T>::LazyArray(int size)
  : m_size(size), m_array(new T*[size])
{
  for(int i = 0; i < m_size; ++i)
    m_array[i] = NULL;
//  std::fill(m_array, m_array + m_size, NULL);
}

// -----------------------------------------------------------------------

template<typename T>
LazyArray<T>::~LazyArray()
{
  std::for_each(m_array, m_array + m_size, boost::checked_deleter<T>());
  delete [] m_array;
}

// -----------------------------------------------------------------------

template<typename T>
T* LazyArray<T>::rawDeref(int pos)
{
  return m_array[pos];
}

// -----------------------------------------------------------------------

template<typename T>
T& LazyArray<T>::operator[](int pos)
{
  if(pos < 0 || pos >= m_size)
    throw std::out_of_range("LazyArray::operator[]");

  if(m_array[pos] == NULL)
  {
    m_array[pos] = new T();
  }
  
  return *(m_array[pos]);
}

// -----------------------------------------------------------------------

template<typename T>
const T& LazyArray<T>::operator[](int pos) const
{
  if(pos < 0 || pos >= m_size)
    throw std::out_of_range("LazyArray::operator[]");

  if(m_array[pos] == NULL)
  {
    m_array[pos] = new T();
  }
  
  return *(m_array[pos]);
}

// -----------------------------------------------------------------------

template<typename T>
AllocatedLazyArrayIterator<T> LazyArray<T>::allocated_begin()
{
  // Find the first 
  int firstEntry = 0;
  while(m_array[firstEntry] == NULL && firstEntry < m_size)
    firstEntry++;

  return AllocatedLazyArrayIterator<T>(firstEntry, this);
}

#endif
