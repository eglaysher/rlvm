// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of libreallive, a dependency of RLVM.
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

#ifndef SRC_LIBREALLIVE_GAMEEXE_H_
#define SRC_LIBREALLIVE_GAMEEXE_H_

#include <boost/iterator/iterator_facade.hpp>
#include <boost/filesystem/path.hpp>

#include <map>
#include <sstream>
#include <string>
#include <vector>

class Gameexe;
class GameexeFilteringIterator;

// -----------------------------------------------------------------------

// Storage backend for the Gameexe
typedef std::vector<int> Gameexe_vec_type;
typedef std::multimap<std::string, Gameexe_vec_type> GameexeData_t;

// -----------------------------------------------------------------------

// Encapsulates a line of the Gameexe file that's passed to the
// user. This is a temporary class, which should hopefully be inlined
// away from the target implementation.
//
// This allows us to write code like this:
//
//   vector<string> x = gameexe("WHATEVER", 5).to_strVector();
//   int var = gameexe("EXPLICIT_CAST").ToInt();
//   int var2 = gameexe("IMPLICIT_CAST");
//   gameexe("SOMEVAL") = 5;
//
// This design solves the problem with the old interface, where all
// the default parameters and overloads lead to confusion about
// whether a parameter was part of the key, or was the deafult
// value. Saying that components of the key are part of the operator()
// on Gameexe and that default values are in the casting function in
// GameexeInterpretObject solves this accidental difficulty.
class GameexeInterpretObject {
 public:
  ~GameexeInterpretObject();

  // Extend a key by one key piece
  template<typename A>
  GameexeInterpretObject operator()(const A& nextKey) {
    return object_to_lookup_on_(key_, nextKey);
  }

  // Finds an int value, returning a default if non-existant.
  const int ToInt(const int defaultValue) const;

  // Finds an int value, throwing if non-existant.
  const int ToInt() const;

  // Allow implicit casts to int with no default value
  operator int() const { return ToInt(); }

  // Returns a specific piece of data at index as an int
  int GetIntAt(int index) const;

  // Finds a string value, throwing if non-existant.
  const std::string ToString(const std::string& defaultValue) const;

  // Finds a string value, throwing if non-existant.
  const std::string ToString() const;

  // Allow implicit casts to string
  operator std::string() const { return ToString(); }

  // Returns a piece of data at a certain location as a string.
  const std::string GetStringAt(int index) const;

  // Finds a vector of ints, throwing if non-existant.
  const std::vector<int>& ToIntVector() const;

  operator std::vector<int>() const { return ToIntVector(); }

  // Checks to see if the key exists.
  bool Exists() const;

  const std::string& key() const {
    return key_;
  }

  // Returns the key splitted on periods.
  const std::vector<std::string> GetKeyParts() const;

  // Assign a value. Unlike all the other methods, we can safely
  // templatize this since the functions it calls can be overloaded.
  GameexeInterpretObject& operator=(const std::string& value);

  GameexeInterpretObject& operator=(const int value);

 private:
  // We expose our private interface to tightly couple with Gameexe,
  // since we are a helper class for it.
  friend class Gameexe;
  friend class GameexeFilteringIterator;

  const std::string key_;
  GameexeData_t::const_iterator iterator_;
  Gameexe& object_to_lookup_on_;

  // Private; only allow construction by Gameexe
  GameexeInterpretObject(const std::string& key, Gameexe& objectToLookupOn);
  GameexeInterpretObject(const std::string& key,
                         GameexeData_t::const_iterator it,
                         Gameexe& objectToLookupOn);
};

// New interface to Gameexe, replacing the one inherited from Haeleth,
// which was hard to use and was very C-ish. This interface's goal is
// to make accessing data in the Gameexe as easy as possible.
class Gameexe {
 public:
  explicit Gameexe(const boost::filesystem::path& filename);
  Gameexe();
  ~Gameexe();

  // Parses an individual Gameexe.ini line.
  void parseLine(const std::string& line);

  // Access the key "firstKey"
  template<typename A>
  GameexeInterpretObject operator()(const A& firstKey);

  // Access the key "firstKey"."secondKey"
  template<typename A, typename B>
  GameexeInterpretObject operator()(const A& firstKey, const B& secondKey);

  // Access the key "firstKey"."secondKey"
  template<typename A, typename B, typename C>
  GameexeInterpretObject operator()(const A& firstKey, const B& secondKey,
                                    const C& thirdKey);

  // Returns iterators that filter on a possible value.
  GameexeFilteringIterator filtering_begin(const std::string& filter);
  GameexeFilteringIterator filtering_end();

  // Returns whether key exists in the stored data
  bool Exists(const std::string& key);

  // Returns the number of keys in the Gameexe.ini file.
  size_t size() const {
    return data_.size();
  }

  // Exposed for testing.
  void SetStringAt(const std::string& key, const std::string& value);
  void SetIntAt(const std::string& key, const int value);

 private:
  const std::vector<int>& GetIntArray(GameexeData_t::const_iterator key);
  int GetIntAt(GameexeData_t::const_iterator key, int index);
  std::string GetStringAt(GameexeData_t::const_iterator key, int index);

  // Returns an iterator for the incoming key. May not be valid. This
  // is a function only for tight coupling with
  // GameexeInterpretObject.
  GameexeData_t::const_iterator Find(const std::string& key);

  // Regrettable artifact of hack to get all integers in streams to
  // have setw(3).
  void AddToStream(const std::string& x, std::ostringstream& ss);

  // Hack to get all integers in streams to have setw(3).
  void AddToStream(const int& x, std::ostringstream& ss);

  void ThrowUnknownKey(const std::string& key);

 private:
  // Allow access from the helper class
  friend class GameexeInterpretObject;
  friend class GameexeFilteringIterator;

  // Implementation detail of how parsed Gameexe.ini data is stored in
  // the class. This was stolen directly from Haeleth's parser in
  // rlBabel. Eventually, this should be redone, since everything is
  // really a vector of ints, unless you want a string in which case
  // that int is an index into a vector of strings on the side.
  GameexeData_t data_;
  std::vector<std::string> cdata_;
};

// -----------------------------------------------------------------------

template<typename A>
GameexeInterpretObject Gameexe::operator()(const A& firstKey) {
  std::ostringstream ss;
  AddToStream(firstKey, ss);
  return GameexeInterpretObject(ss.str(), *this);
}

// -----------------------------------------------------------------------

template<>
inline GameexeInterpretObject Gameexe::operator()(const std::string& firstKey) {
  return GameexeInterpretObject(firstKey, *this);
}

// -----------------------------------------------------------------------

template<typename A, typename B>
GameexeInterpretObject Gameexe::operator()(const A& firstKey,
                                           const B& secondKey) {
  std::ostringstream ss;
  AddToStream(firstKey, ss);
  ss << ".";
  AddToStream(secondKey, ss);
  return GameexeInterpretObject(ss.str(), *this);
}

// -----------------------------------------------------------------------

template<typename A, typename B, typename C>
GameexeInterpretObject Gameexe::operator()(const A& firstKey,
                                           const B& secondKey,
                                           const C& thirdKey) {
  std::ostringstream ss;
  AddToStream(firstKey, ss);
  ss << ".";
  AddToStream(secondKey, ss);
  ss << ".";
  AddToStream(thirdKey, ss);
  return GameexeInterpretObject(ss.str(), *this);
}

// -----------------------------------------------------------------------

class GameexeFilteringIterator
  : public boost::iterator_facade<
  GameexeFilteringIterator,
  GameexeInterpretObject,
  boost::forward_traversal_tag, GameexeInterpretObject> {
 public:
  explicit GameexeFilteringIterator(const std::string& inFilterKeys,
                                    Gameexe& inGexe,
                                    GameexeData_t::const_iterator it)
      : filterKeys(inFilterKeys), gexe(inGexe), currentKey(it) {
    incrementUntilValid();
  }

  GameexeFilteringIterator(GameexeFilteringIterator const& other)
      : filterKeys(other.filterKeys), gexe(other.gexe),
        currentKey(other.currentKey) {
  }

 private:
  friend class boost::iterator_core_access;
  friend class Gameexe;

  bool equal(GameexeFilteringIterator const& other) const {
    // It is deliberate that we only compare the current keys. This
    // means you don't need to
    return currentKey == other.currentKey;
  }

  void increment() {
    currentKey++;
    incrementUntilValid();
  }

  GameexeInterpretObject dereference() const {
    return GameexeInterpretObject(currentKey->first, currentKey, gexe);
  }

  void incrementUntilValid();

  const std::string filterKeys;
  Gameexe& gexe;
  GameexeData_t::const_iterator currentKey;
};

// -----------------------------------------------------------------------

#endif  // SRC_LIBREALLIVE_GAMEEXE_H_
