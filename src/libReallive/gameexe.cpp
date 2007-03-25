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

  As a special exception to the GNU Lesser General Public License (LGPL), you 
  may include a publicly distributed version of the library alongside a "work 
  that uses the Library" to produce a composite work that includes the library, 
  and distribute that work under terms of your choice, without any of the 
  additional requirements listed in clause 6 of the LGPL.

  A "publicly distributed version of the library" means either an unmodified 
  binary as distributed by Haeleth, or a modified version of the library that is 
  distributed under the conditions defined in clause 2 of the LGPL, a\nd a 
  "composite work that includes the library" means a RealLive program which 
  links to the library, either through the LoadDLL() interface or a #DLL 
  directive, and/or includes code from the library's Kepago header.

  Note that this exception does not invalidate any other reasons why any part of 
  the work might be covered by the LGPL.
*/
// -----------------------------------------------------------------------

#include "Precompiled.hpp"

// -----------------------------------------------------------------------
  
#include "gameexe.h"
#include "defs.h"

#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/trim.hpp>

#include <iostream>
#include <fstream>

using namespace boost;
using namespace std;

#define is_space(c) (c == '\r' || c == '\n' || c == ' ' || c == '\t')
#define is_num(c)   (c == '-' || (c >= '0' && c <= '9'))
#define is_data(c)  (c == '"' || is_num(c))

/**
 * A boost::TokenizerFunction used to extract valid pieces of data
 * from the value part of a gameexe key/value pair.
 */
class gameexe_token_extractor
{
public:
  void reset() { }

  template<typename InputIterator, typename Token>
  bool operator()(InputIterator& next, InputIterator end, Token& tok)
  {
    tok = Token();    
    // Advance to the next data character
    for(; next != end && ( !is_data(*next) ); ++next);

    if(next == end)
      return false;

    if(*next == '"')
    {
      tok += '"';
      next++;
      for(; next != end && *next != '"'; ++next)
        tok += *next;
      tok += '"';
      next++;
    }
    else
    {
      // Eat the current character and all 
      for(; next != end && is_num(*next); ++next)
        tok += *next;
    }

    return true;
  }
};

// -----------------------------------------------------------------------

Gameexe::Gameexe()
{}

// -----------------------------------------------------------------------

Gameexe::Gameexe(const std::string& gameexefile)
  : data_(), cdata_()
{
  ifstream ifs(gameexefile.c_str());
  if(!ifs)
    throw libReallive::Error("Could not find Gameexe.ini file!");

  string line;
  while(getline(ifs, line))
  {
    size_t firstHash = line.find_first_of('#');
    if(firstHash != string::npos)
    {
      // Extract what's the key and value
      size_t firstEqual = line.find_first_of('=');
      string key = line.substr(firstHash + 1, firstEqual - firstHash - 1);
      string value = line.substr(firstEqual + 1);

      // Get rid of extra whitespace
      trim(key);
      trim(value);

      vec_type vec;

      // Extract all numeric and data values from the value
      typedef boost::tokenizer<gameexe_token_extractor> ValueTokenizer;
      ValueTokenizer tokenizer(value);
      for(ValueTokenizer::iterator it = tokenizer.begin(); 
          it != tokenizer.end(); ++it)
      {
        const string& tok = *it;
        if(tok[0] == '"')
        {
          string unquoted = tok.substr(1, tok.size() - 2);
          cdata_.push_back(unquoted);
          vec.push_back(cdata_.size() - 1);
        }
        else if(tok != "-")
          vec.push_back(lexical_cast<int>(tok));
      }
      data_[key] = vec;
    }
  }
}

Gameexe::~Gameexe()
{
}

// -----------------------------------------------------------------------

const std::vector<int>& Gameexe::getIntArray(const std::string& key)
{
  data_t::const_iterator it = data_.find(key);
  if(it == data_.end())
  {
    static std::vector<int> falseVector;
    return falseVector;
  }

  return it->second;
}

// -----------------------------------------------------------------------

int Gameexe::getIntAt(const std::string& key, int index)
{
  data_t::const_iterator it = data_.find(key);
  if(it == data_.end()) 
    throwUnknownKey(key);

  return it->second.at(index);
}

// -----------------------------------------------------------------------

void Gameexe::throwUnknownKey(const std::string& key) 
{
  std::ostringstream ss;
  ss << "Unknown Gameexe key '" << key << "'";
  throw libReallive::Error(ss.str());
}

// -----------------------------------------------------------------------

const int GameexeInterpretObject::to_int(const int defaultValue) const {
  const std::vector<int>& ints = m_objectToLookupOn.getIntArray(m_key);
  if(ints.size() == 0)
    return defaultValue;

  return ints[0];
}

// -----------------------------------------------------------------------

const int GameexeInterpretObject::to_int() const {
  const std::vector<int>& ints = m_objectToLookupOn.getIntArray(m_key);
  if(ints.size() == 0)
    m_objectToLookupOn.throwUnknownKey(m_key);

  return ints[0];
}

// -----------------------------------------------------------------------

const std::string GameexeInterpretObject::to_string(
  const std::string& defaultValue) const
{
  try 
  {
    return m_objectToLookupOn.getStringAt(m_key, 0);
  } 
  catch(...) 
  {
    return defaultValue;
  }
}

// -----------------------------------------------------------------------

const std::string GameexeInterpretObject::to_string() const
{
  try 
  {
    return m_objectToLookupOn.getStringAt(m_key, 0);
  } 
  catch(...) 
  {
    m_objectToLookupOn.throwUnknownKey(m_key);
  }
}

// -----------------------------------------------------------------------

const std::vector<int>& GameexeInterpretObject::to_intVector() const
{
  const std::vector<int>& ints = m_objectToLookupOn.getIntArray(m_key);
  if(ints.size() == 0)
    m_objectToLookupOn.throwUnknownKey(m_key);

  return ints;    
}

// -----------------------------------------------------------------------

bool GameexeInterpretObject::exists() const
{
  return m_objectToLookupOn.exists(m_key);
}

// -----------------------------------------------------------------------

GameexeInterpretObject& GameexeInterpretObject::operator=(const std::string& value)
{
  // Set the key to incoming int
  m_objectToLookupOn.setStringAt(m_key, value);
  return *this;
}

// -----------------------------------------------------------------------

GameexeInterpretObject& GameexeInterpretObject::operator=(const int value)
{
  // Set the key to incoming int
  m_objectToLookupOn.setIntAt(m_key, value);
  return *this;
}
