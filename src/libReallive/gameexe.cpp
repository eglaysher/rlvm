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
  
#include "gameexe.h"
#include "defs.h"

#include <iostream>

using namespace std;

#define is_space(c) (c == '\r' || c == '\n' || c == ' ' || c == '\t')
#define is_num(c)   (c == '-' || (c >= '0' && c <= '9'))
#define is_data(c)  (c == '"' || is_num(c))

// const int num_paths = 7;
// char* paths[num_paths] = {
// 	"rlBabel.ini", "gameexe.kor", "gameexe.ini", 
// 	"KINETICDATA\\rlBabel.ini","KINETICDATA\\gameexe.ini", 
// 	"REALLIVEDATA\\rlBabel.ini", "REALLIVEDATA\\gameexe.ini" 
// };

Gameexe::Gameexe()
{}

// @todo OMFG! Teh evil! sprintf!?! This needs a rewrite!
Gameexe::Gameexe(const std::string& gameexefile)
  : data_(), cdata_()
{
  char ini_path[2048], *filename;
	FILE *ini = NULL;
	
    ini = fopen(gameexefile.c_str(), "r");
    if(!ini)
      throw libReallive::Error("Could not find Gameexe.ini file!");

	char buf[2048]; // longest line seen in the wild is 571 chars long
	while (fgets(buf, 2048, ini)) {
		if (buf[0] == '#') {
			int l = strlen(buf);
			while (is_space(buf[l - 1])) buf[--l] = 0;
			// Set "buf" to null-terminated stripped key, and "key" to null-terminated stripped value.
			char *val = buf;
			while (val++ < buf + l && *val != '=');
			if (val < buf + l) {
				char *key = val - 1;
				while (is_space(*key)) --key;
				key[1] = 0;
				val++;
				key = buf + 1;
				// Parse value
				vec_type vec;
				while (*val) {
					while (*val && !is_data(*val)) ++val;
					if (!*val) break;
					char *token = val;
					if (*val == '"') {
						// string
						++token;
						while (*(++val) != '"') if (*val == '\\' && val[1] == '"') ++val;
						*val++ = 0;
						cdata_.push_back(std::string(token));
						vec.push_back(cdata_.size() - 1);
					}
					else {
						// integer
						while (is_num(*val)) ++val;
						if (*val) *val++ = 0;
						vec.push_back(atoi(token));
					}
				}
				data_[std::string(key)] = vec;
			}
		}
	}
	fclose(ini);
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

const int GameexeInterpretObject::to_int(const int defaultValue) {
  const std::vector<int>& ints = m_objectToLookupOn.getIntArray(m_key);
  if(ints.size() == 0)
    return defaultValue;

  return ints[0];
}

// -----------------------------------------------------------------------

const int GameexeInterpretObject::to_int() {
  const std::vector<int>& ints = m_objectToLookupOn.getIntArray(m_key);
  if(ints.size() == 0)
    m_objectToLookupOn.throwUnknownKey(m_key);

  return ints[0];
}

// -----------------------------------------------------------------------

const std::string GameexeInterpretObject::to_string(const std::string& defaultValue) {
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

const std::string GameexeInterpretObject::to_string() {
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

const std::vector<int>& GameexeInterpretObject::to_intVector() {
  const std::vector<int>& ints = m_objectToLookupOn.getIntArray(m_key);
  if(ints.size() == 0)
    m_objectToLookupOn.throwUnknownKey(m_key);

  return ints;    
}

// -----------------------------------------------------------------------

bool GameexeInterpretObject::exists()
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
