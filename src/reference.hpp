#ifndef __Reference_hpp__
#define __Reference_hpp__

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
  IntAccessor(MemoryReferenceIterator<IntAccessor>* i) : it(i) {}

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
  StringAccessor(MemoryReferenceIterator<StringAccessor>* i) : it(i) {}

  operator std::string() const;

  StringAccessor& operator=(const std::string& newValue);
  StringAccessor& operator=(const StringAccessor& newValue);

  bool operator==(const std::string& rhs);
};

/** 
 * MemoryReferenceIterator serves the job of iterators over a piece of
 * memory. Since changing the RLMachine's memory can change the
 * pointed memory address of a MemoryReference, we create iterators
 * that point to the current state of an MemoryReference. This also
 * solves the problem where some functions in RealLive accept two
 * memory addresses, and do something on that range.
 * 
 */
template<typename ACCESS>
class MemoryReferenceIterator
  : public std::iterator<std::random_access_iterator_tag, ACCESS> {
//  : public std::iterator<std::bidirectional_iterator_tag, IntAccessor> {
private:
  int m_type;
  int m_location;
  RLMachine* m_machine;
  // Can this be templated?
  friend class StringAccessor;
  friend class IntAccessor;

public:
  // Explicit reference creation
  MemoryReferenceIterator(RLMachine* inMachine, const int inType, const int inLocation)
    : m_machine(inMachine), m_type(inType), m_location(inLocation) { }     

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

/* Fix this if something complains
  IntAccessor operator[](int offset) {
    return IntAccessor
  }
*/

  bool operator==(const MemoryReferenceIterator<ACCESS>& rhs) const {
    return m_machine == rhs.m_machine && m_type == rhs.m_type && 
      m_location == rhs.m_location;
  }

  bool operator!=(const MemoryReferenceIterator<ACCESS>& rhs) const {
    return ! operator==(rhs);
  }
};

// -----------------------------------------------------------------------

typedef MemoryReferenceIterator<IntAccessor> IntReferenceIterator;
typedef MemoryReferenceIterator<StringAccessor> StringReferenceIterator;

#endif
