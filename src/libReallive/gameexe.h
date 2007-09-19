// This file is part of libReallive, a dependency of RLVM. 
// 
// -----------------------------------------------------------------------
//
// Copyright (c) 2006, 2007 Peter Jolly
// Copyright (c) 2007 Elliot Glaysher
//
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use, copy,
// modify, merge, publish, distribute, sublicense, and/or sell copies
// of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
// BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// -----------------------------------------------------------------------
  
#ifndef GAMEEXE_H
#define GAMEEXE_H

#include <vector>
#include <string>
#include <sstream>
#include <iomanip>

#ifdef __GNUC__
//// Can't read STL error messages.
//#include <ext/hash_map>
//using namespace __gnu_cxx;
#include <map>
#define hash_map std::map
#else
#include <hash_map>
using stdext::hash_map;
#endif

#include "libReallive/defs.h"

class Gameexe;
class GameexeFilteringIterator;

/**
 * Encapsulates a line of the Gameexe file that's passed to the
 * user. This is a temporary class, which should hopefully be inlined
 * away from the target implementation. 
 *
 * This allows us to write code like this:
 *
 * @code
 * vector<string> x = gameexe("WHATEVER", 5).to_strVector();
 * int var = gameexe("EXPLICIT_CAST").to_int();
 * int var2 = gameexe("IMPLICIT_CAST");
 * gameexe("SOMEVAL") = 5;
 * @endcode
 *
 * This design solves the problem with the old interface, where all
 * the default parameters and overloads lead to confusion about
 * whether a parameter was part of the key, or was the deafult
 * value. Saying that components of the key are part of the operator()
 * on Gameexe and that default values are in the casting function in
 * GameexeInterpretObject solves this accidental difficulty.
 */
class GameexeInterpretObject
{
private:
  /// We expose our private interface to tightly couple with Gameexe,
  /// since we are a helper class for it.
  friend class Gameexe;
  friend class GameexeFilteringIterator;

  const std::string m_key;
  Gameexe& m_objectToLookupOn;

  /** 
   * Private; only allow construction by Gameexe
   */
  GameexeInterpretObject(const std::string& key, Gameexe& objectToLookupOn)
    : m_key(key), m_objectToLookupOn(objectToLookupOn)
  {
    
  }

public:

  /** 
   * Extend a key by one key piece
   */
  template<typename A>
  GameexeInterpretObject operator()(const A& nextKey)
  {
    return m_objectToLookupOn(m_key, nextKey);
  }

  /** 
   * Finds an int value, returning a default if non-existant.
   * 
   * @param defaultValue Default integer value to return if key not found
   * @return 
   */
  const int to_int(const int defaultValue) const;

  /** 
   * Finds an int value, throwing if non-existant.
   * 
   * @return The first int value from the Gameexe in the row key
   * @throw Error if the key doesn't exist
   */
  const int to_int() const;

  /// Allow implicit casts to int with no default value
  operator int() const {
    return to_int();
  }

  /** 
   * Finds a string value, throwing if non-existant.
   * 
   * @return The first string value from the Gameexe in that row
   * @throw Error if the key doesn't exist
   */
  const std::string to_string(const std::string& defaultValue) const;

  /** 
   * Finds a string value, throwing if non-existant.
   * 
   * @return The first string value from the Gameexe in that row
   * @throw Error if the key doesn't exist
   */
  const std::string to_string() const;

  /// Allow implicit casts to string
  operator std::string() const {
    return to_string();
  }

  /** 
   * Finds a vector of ints, throwing if non-existant.
   * 
   * @return The full row in the Gameexe (if it's an int row)
   * @throw Error if the key doesn't exist
   */
  const std::vector<int>& to_intVector() const;

  operator std::vector<int>() const {
    return to_intVector();
  }

  /** 
   * Checks to see if the key exists.
   * 
   * @return True if exists, false otherwise
   */
  bool exists() const;

  const std::string& key() const {
    return m_key;
  }

  /** 
   * Assign a value. Unlike all the other methods, we can safely
   * templatize this since the functions it calls can be overloaded.
   * 
   * @param value Incoming value
   * @return self
   */
  GameexeInterpretObject& operator=(const std::string& value);

  GameexeInterpretObject& operator=(const int value);
};

/**
 * New interface to Gameexe, replacing the one inherited from Haeleth,
 * which was hard to use and was very C-ish. This interface's goal is
 * to make accessing data in the Gameexe as easy as possible.
 */
class Gameexe
{
private:
  /// Allow access from the helper class
  friend class GameexeInterpretObject;
  friend class GameexeFilteringIterator;

  /// @{
  /**
   * @name Data storage
   * 
   * Implementation detail of how parsed Gameexe.ini data is stored in
   * the class. This was stolen directly from Haeleth's parser in
   * rlBabel. Eventually, this should be redone, since everything is
   * really a vector of ints, unless you want a string in which case
   * that int is an index into a vector of strings on the side.
   */
  typedef std::vector<int> vec_type;
  typedef hash_map<std::string, vec_type> data_t;
  data_t data_;
  std::vector<std::string> cdata_;
  /// @}

public:
  /**
   * Create an empty Gameexe, with no configuration data.
   */
  Gameexe();

  /**
   * Create a Gameexe based off the configuration data in the incoming
   * file.
   */
  Gameexe(const std::string& filename);

  /**
   * Constructor
   * 
   */
  ~Gameexe();

  /// @{
  /**
   * @name Streamlined Interface for data access
   * 
   * This is the interface intended for common use. It seperates the
   * construction of the key from the intended type, and default value.
   */

  /** 
   * Access the key "firstKey"
   */
  template<typename A>
  GameexeInterpretObject operator()(const A& firstKey);

  /** 
   * Access the key "firstKey"."secondKey"
   */
  template<typename A, typename B>
  GameexeInterpretObject operator()(const A& firstKey, const B& secondKey);

  /** 
   * Access the key "firstKey"."secondKey"
   */
  template<typename A, typename B, typename C>
  GameexeInterpretObject operator()(const A& firstKey, const B& secondKey,
                                    const C& thirdKey);

  /// @}

  /**
   * @name Iterated interface for keys
   *
   * This interface gives filtering iterators that filter on a
   * possible value.
   * 
   * @{
   */
  GameexeFilteringIterator filtering_begin(const std::string& filter);
  GameexeFilteringIterator filtering_end();
  /// @}

  /// @{
  /**
   * @name Raw interface for Gameexe.ini data access
   * 
   * This is the internal interface used by GameexeInterpretObject,
   * but it is exposed to the user for the handfull of cases where
   * integer and string data are mixed in the same line.
   */

  /**
   * Raw interface for 
   */
  const std::vector<int>& getIntArray(const std::string& key);

  int getIntAt(const std::string& key, int index);

  /** 
   * Returns whether key exists in the stored data
   */
  bool exists(const std::string& key);

  /** 
   * Returns the number of keys in the Gameexe.ini file.
   */
  int size() const {
    return data_.size();
  }

  /** 
   * Internal function that returns an array of int values.
   */
  std::string getStringAt(const std::string& key, int index);
  void setStringAt(const std::string& key, const std::string& value);
  void setIntAt(const std::string& key, const int value);
  /// @}

private:
  /** 
   * Regrettable artifact of hack to get all integers in streams to
   * have setw(3).
   */
  void addToStream(const std::string& x, std::ostringstream& ss) {
    ss << x;
  }

  /** 
   * Hack to get all integers in streams to have setw(3).
   */
  void addToStream(const int& x, std::ostringstream& ss) {
    ss << std::setw(3) << std::setfill('0') << x;
  }

  void throwUnknownKey(const std::string& key);
};

// -----------------------------------------------------------------------

template<typename A>
GameexeInterpretObject Gameexe::operator()(const A& firstKey)
{
  std::ostringstream ss;
  addToStream(firstKey, ss);
  return GameexeInterpretObject(ss.str(), *this);
}

// -----------------------------------------------------------------------

template<>
inline GameexeInterpretObject Gameexe::operator()(const std::string& firstKey)
{
  return GameexeInterpretObject(firstKey, *this);
}

// -----------------------------------------------------------------------

template<typename A, typename B>
GameexeInterpretObject Gameexe::operator()(const A& firstKey, const B& secondKey)
{
  std::ostringstream ss;
  addToStream(firstKey, ss);
  ss << ".";
  addToStream(secondKey, ss);
  return GameexeInterpretObject(ss.str(), *this);
}

// -----------------------------------------------------------------------

template<typename A, typename B, typename C>
GameexeInterpretObject Gameexe::operator()(
  const A& firstKey, const B& secondKey,
  const C& thirdKey)
{
  std::ostringstream ss;
  addToStream(firstKey, ss);
  ss << ".";
  addToStream(secondKey, ss);
  ss << ".";
  addToStream(thirdKey, ss);
  return GameexeInterpretObject(ss.str(), *this);
}

// -----------------------------------------------------------------------

class GameexeFilteringIterator 
  : public boost::iterator_facade<
  GameexeFilteringIterator,
  GameexeInterpretObject,
  boost::forward_traversal_tag, GameexeInterpretObject>
{
public:
  explicit GameexeFilteringIterator(const std::string& inFilterKeys,
                                    Gameexe& inGexe,
                                    Gameexe::data_t::const_iterator it)
    : filterKeys(inFilterKeys), gexe(inGexe), currentKey(it)
  {
    incrementUntilValid();
  }

  GameexeFilteringIterator(GameexeFilteringIterator const& other)
    : filterKeys(other.filterKeys), gexe(other.gexe), 
      currentKey(other.currentKey)
  {}

private:
  friend class boost::iterator_core_access;
  friend class Gameexe;

  bool equal(GameexeFilteringIterator const& other) const
  {
    // It is deliberate that we only compare the current keys. This
    // means you don't need to 
    return currentKey == other.currentKey;
  }

  void increment()
  {
    currentKey++;
    incrementUntilValid();
  }

  GameexeInterpretObject dereference() const
  {
    return GameexeInterpretObject(currentKey->first, gexe);
  }

  void incrementUntilValid();

  const std::string filterKeys;
  Gameexe& gexe;
  Gameexe::data_t::const_iterator currentKey;
};

// -----------------------------------------------------------------------


#endif
