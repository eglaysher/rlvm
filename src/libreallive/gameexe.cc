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

#include "libreallive/gameexe.h"

#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/fstream.hpp>

#include <iostream>
#include <fstream>
#include <iomanip>

#include "libreallive/defs.h"

using namespace boost;
using namespace std;
namespace fs = boost::filesystem;

#define is_space(c) (c == '\r' || c == '\n' || c == ' ' || c == '\t')
#define is_num(c) (c == '-' || (c >= '0' && c <= '9'))
#define is_data(c) (c == '"' || is_num(c))

/**
 * A boost::TokenizerFunction used to extract valid pieces of data
 * from the value part of a gameexe key/value pair.
 */
class gameexe_token_extractor {
 public:
  void reset() {}

  template <typename InputIterator, typename Token>
  bool operator()(InputIterator& next, InputIterator end, Token& tok) {
    tok = Token();
    // Advance to the next data character
    for (; next != end && (!is_data(*next)); ++next)
      ;

    if (next == end)
      return false;

    if (*next == '"') {
      tok += '"';
      next++;
      for (; next != end && *next != '"'; ++next)
        tok += *next;
      tok += '"';
      next++;
    } else {
      char lastChar = '\0';

      // Eat the current character and all
      while (next != end) {
        if (*next == '-') {
          // Dashes are ambiguous. They are both seperators and the negative
          // sign and we have to tokenize differently based on what it's
          // doing. If the previous character is a number, we are being used as
          // a range separator.
          if (lastChar >= '0' && lastChar <= '9') {
            // Skip the dash so we don't treat the next number as negative.
            next++;
            break;
          } else {
            // Consume the dash for the next parts.
            tok += *next;
          }
        } else if (is_num(*next)) {
          // All other numbers are consumed here.
          tok += *next;
        } else {
          // We only deal with numbers in this branch.
          break;
        }

        lastChar = *next;
        ++next;
      }
    }

    return true;
  }
};

// -----------------------------------------------------------------------

Gameexe::Gameexe() {}

// -----------------------------------------------------------------------

Gameexe::Gameexe(const fs::path& gameexefile) : data_(), cdata_() {
  fs::ifstream ifs(gameexefile);
  if (!ifs) {
    ostringstream oss;
    oss << "Could not find Gameexe.ini file! (Looking in " << gameexefile
        << ")";
    throw libreallive::Error(oss.str());
  }

  string line;
  while (getline(ifs, line)) {
    parseLine(line);
  }
}

Gameexe::~Gameexe() {}

// -----------------------------------------------------------------------

void Gameexe::parseLine(const std::string& line) {
  size_t firstHash = line.find_first_of('#');
  if (firstHash != string::npos) {
    // Extract what's the key and value
    size_t firstEqual = line.find_first_of('=');
    string key = line.substr(firstHash + 1, firstEqual - firstHash - 1);
    string value = line.substr(firstEqual + 1);

    // Get rid of extra whitespace
    trim(key);
    trim(value);

    Gameexe_vec_type vec;

    // Extract all numeric and data values from the value
    typedef boost::tokenizer<gameexe_token_extractor> ValueTokenizer;
    ValueTokenizer tokenizer(value);
    for (ValueTokenizer::iterator it = tokenizer.begin(); it != tokenizer.end();
         ++it) {
      const string& tok = *it;
      if (tok[0] == '"') {
        string unquoted = tok.substr(1, tok.size() - 2);
        cdata_.push_back(unquoted);
        vec.push_back(cdata_.size() - 1);
      } else if (tok != "-") {
        try {
          vec.push_back(lexical_cast<int>(tok));
        }
        catch (boost::bad_lexical_cast& e) {
          cerr << "Couldn't int-ify '" << tok << "'" << endl;
          vec.push_back(0);
        }
      }
    }
    data_.insert(make_pair(key, vec));
  }
}

// -----------------------------------------------------------------------

const std::vector<int>& Gameexe::getIntArray(
    GameexeData_t::const_iterator key) {
  if (key == data_.end()) {
    static std::vector<int> falseVector;
    return falseVector;
  }

  return key->second;
}

// -----------------------------------------------------------------------

int Gameexe::getIntAt(GameexeData_t::const_iterator key, int index) {
  if (key == data_.end())
    throwUnknownKey("TMP");

  return key->second.at(index);
}

// -----------------------------------------------------------------------

bool Gameexe::exists(const std::string& key) {
  return data_.find(key) != data_.end();
}

// -----------------------------------------------------------------------

std::string Gameexe::getStringAt(GameexeData_t::const_iterator key, int index) {
  int cindex = getIntAt(key, index);
  return cdata_.at(cindex);
}

// -----------------------------------------------------------------------

void Gameexe::setStringAt(const std::string& key, const std::string& value) {
  Gameexe_vec_type toStore;
  cdata_.push_back(value);
  toStore.push_back(cdata_.size() - 1);
  data_.erase(key);
  data_.insert(make_pair(key, toStore));
}

// -----------------------------------------------------------------------

void Gameexe::setIntAt(const std::string& key, const int value) {
  Gameexe_vec_type toStore;
  toStore.push_back(value);
  data_.erase(key);
  data_.insert(make_pair(key, toStore));
}

// -----------------------------------------------------------------------

GameexeData_t::const_iterator Gameexe::find(const std::string& key) {
  return data_.find(key);
}

// -----------------------------------------------------------------------

void Gameexe::addToStream(const std::string& x, std::ostringstream& ss) {
  ss << x;
}

// -----------------------------------------------------------------------

void Gameexe::addToStream(const int& x, std::ostringstream& ss) {
  ss << std::setw(3) << std::setfill('0') << x;
}

// -----------------------------------------------------------------------

void Gameexe::throwUnknownKey(const std::string& key) {
  std::ostringstream ss;
  ss << "Unknown Gameexe key '" << key << "'";
  throw libreallive::Error(ss.str());
}

// -----------------------------------------------------------------------

GameexeFilteringIterator Gameexe::filtering_begin(const std::string& filter) {
  return GameexeFilteringIterator(filter, *this, data_.begin());
}

// -----------------------------------------------------------------------

GameexeFilteringIterator Gameexe::filtering_end() {
  return GameexeFilteringIterator("", *this, data_.end());
}

// -----------------------------------------------------------------------
// GameexeInterpretObject
// -----------------------------------------------------------------------
GameexeInterpretObject::GameexeInterpretObject(const std::string& key,
                                               Gameexe& objectToLookupOn)
    : key_(key),
      iterator_(objectToLookupOn.find(key)),
      object_to_lookup_on_(objectToLookupOn) {}

// -----------------------------------------------------------------------

GameexeInterpretObject::GameexeInterpretObject(const std::string& key,
                                               GameexeData_t::const_iterator it,
                                               Gameexe& objectToLookupOn)
    : key_(key), iterator_(it), object_to_lookup_on_(objectToLookupOn) {}

// -----------------------------------------------------------------------

GameexeInterpretObject::~GameexeInterpretObject() {}

// -----------------------------------------------------------------------

const int GameexeInterpretObject::to_int(const int defaultValue) const {
  const std::vector<int>& ints = object_to_lookup_on_.getIntArray(iterator_);
  if (ints.size() == 0)
    return defaultValue;

  return ints[0];
}

// -----------------------------------------------------------------------

const int GameexeInterpretObject::to_int() const {
  const std::vector<int>& ints = object_to_lookup_on_.getIntArray(iterator_);
  if (ints.size() == 0)
    object_to_lookup_on_.throwUnknownKey(key_);

  return ints[0];
}

// -----------------------------------------------------------------------

int GameexeInterpretObject::getIntAt(int index) const {
  return object_to_lookup_on_.getIntAt(iterator_, index);
}

// -----------------------------------------------------------------------

const std::string GameexeInterpretObject::to_string(
    const std::string& defaultValue) const {
  try {
    return object_to_lookup_on_.getStringAt(iterator_, 0);
  }
  catch (...) {
    return defaultValue;
  }
}

// -----------------------------------------------------------------------

const std::string GameexeInterpretObject::to_string() const {
  try {
    return object_to_lookup_on_.getStringAt(iterator_, 0);
  }
  catch (...) {
    object_to_lookup_on_.throwUnknownKey(key_);
  }

  // Shut the -Wall up
  return "";
}

// -----------------------------------------------------------------------

const std::string GameexeInterpretObject::getStringAt(int index) const {
  return object_to_lookup_on_.getStringAt(iterator_, index);
}

// -----------------------------------------------------------------------

const std::vector<int>& GameexeInterpretObject::to_intVector() const {
  const std::vector<int>& ints = object_to_lookup_on_.getIntArray(iterator_);
  if (ints.size() == 0)
    object_to_lookup_on_.throwUnknownKey(key_);

  return ints;
}

// -----------------------------------------------------------------------

bool GameexeInterpretObject::exists() const {
  return object_to_lookup_on_.exists(key_);
}

// -----------------------------------------------------------------------

const std::vector<std::string> GameexeInterpretObject::key_parts() const {
  vector<string> keyparts;
  boost::split(keyparts, key_, is_any_of("."));
  return keyparts;
}

// -----------------------------------------------------------------------

GameexeInterpretObject& GameexeInterpretObject::operator=(
    const std::string& value) {
  // Set the key to incoming int
  object_to_lookup_on_.setStringAt(key_, value);
  return *this;
}

// -----------------------------------------------------------------------

GameexeInterpretObject& GameexeInterpretObject::operator=(const int value) {
  // Set the key to incoming int
  object_to_lookup_on_.setIntAt(key_, value);
  return *this;
}

// -----------------------------------------------------------------------
// GameexeFilteringIterator
// -----------------------------------------------------------------------

void GameexeFilteringIterator::incrementUntilValid() {
  while (currentKey != gexe.data_.end() &&
         !istarts_with(currentKey->first, filterKeys)) {
    currentKey++;
  }
}
