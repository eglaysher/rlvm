/*
  rlBabel: interface to GAMEEXE.INI

  Copyright (c) 2006 Peter Jolly.

  This library is free software; you can redistribute it and/or modify it under
  the terms of the GNU Lesser General Public License as published by the Free
  Software Foundation; either version 2.1 of the License, or (at your option) any
  later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
  details.

  You should have received a copy of the GNU Lesser General Public License
  along with this library; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

  As a special exception to the GNU Lesser General Public License, you may
  include a publicly distributed version of the library in a "work that uses the
  Library" to produce a scenario file containing portions of the library, and
  distribute that scenario file under terms of your choice, without any of the
  additional requirements listed in clause 6 of the GNU Lesser General Public
  License.  A "publicly distributed version of the library" means either an
  unmodified library as distributed by Haeleth, or a modified version of the
  library that is distributed under the conditions defined in clause 2 of the
  GNU Lesser General Public License.  Note that this exception does not
  invalidate any other reasons why the scenario file might be covered by the
  GNU Lesser General Public License.
*/
  
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

  const std::string m_key;
  Gameexe& m_objectToLookupOn;

  /** 
   * Private; only allow construction by Gameexe
   */
  GameexeInterpretObject(const std::string& key, Gameexe& objectToLookupOn)
    : m_key(key), m_objectToLookupOn(objectToLookupOn)
  {
    
  }

  /**
   * Disallow copying.
   */
  GameexeInterpretObject(const GameexeInterpretObject&);

public:

  /** 
   * Finds an int value, returning a default if non-existant.
   * 
   * @param defaultValue Default integer value to return if key not found
   * @return 
   */
  const int to_int(const int defaultValue);

  /** 
   * Finds an int value, throwing if non-existant.
   * 
   * @return The first int value from the Gameexe in the row key
   * @throw Error if the key doesn't exist
   */
  const int to_int();

  /// Allow implicit casts to int with no default value
  operator int() {
    return to_int();
  }

  /** 
   * Finds a string value, throwing if non-existant.
   * 
   * @return The first string value from the Gameexe in that row
   * @throw Error if the key doesn't exist
   */
  const std::string to_string(const std::string& defaultValue);

  /** 
   * Finds a string value, throwing if non-existant.
   * 
   * @return The first string value from the Gameexe in that row
   * @throw Error if the key doesn't exist
   */
  const std::string to_string();

  /// Allow implicit casts to string
  operator std::string() {
    return to_string();
  }

  /** 
   * Finds a vector of ints, throwing if non-existant.
   * 
   * @return The full row in the Gameexe (if it's an int row)
   * @throw Error if the key doesn't exist
   */
  const std::vector<int>& to_intVector();

  /** 
   * Checks to see if the key exists.
   * 
   * @return True if exists, false otherwise
   */
  bool exists();

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
 * 
 */
class Gameexe
{

private:
  /// Allow access from the helper class
  friend class GameexeInterpretObject;

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
  GameexeInterpretObject operator()(const A& firstKey)
  {
    std::ostringstream ss;
    addToStream(firstKey, ss);
    return GameexeInterpretObject(ss.str(), *this);
  }

  /** 
   * Access the key "firstKey"."secondKey"
   */
  template<typename A, typename B>
  GameexeInterpretObject operator()(const A& firstKey, const B& secondKey)
  {
    std::ostringstream ss;
    addToStream(firstKey, ss);
    ss << ".";
    addToStream(secondKey, ss);
    return GameexeInterpretObject(ss.str(), *this);
  }

  /// @}


  /// @{
  /**
   * @name Raw interface for Gameexe.ini data access
   * 
   * This is the internal interface used by GameexeInterpretObject,
   * but it is exposed to the user for the handfull of cases where the 
   */

  /**
   * Raw interface for 
   */
  const std::vector<int>& getIntArray(const std::string& key);

  int getIntAt(const std::string& key, int index);

  /** 
   * Returns whether key exists in the stored data
   */
  bool exists(const std::string& key) {
    return data_.find(key) != data_.end();
  }

  /** 
   * Returns the number of keys in the Gameexe.ini file.
   */
  int size() const {
    return data_.size();
  }

  /** 
   * Internal function that returns an array of int values.
   */
  std::string getStringAt(const std::string& key, int index) {
    int cindex = getIntAt(key, index);
    return cdata_.at(cindex);
  }

  void setStringAt(const std::string& key, const std::string& value) {
    vec_type toStore;
    cdata_.push_back(value);
    toStore.push_back(cdata_.size() - 1);
    data_[key] = toStore;
  }

  void setIntAt(const std::string& key, const int value) {
    vec_type toStore;
    toStore.push_back(value);
    data_[key] = toStore;
  }

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

#endif
