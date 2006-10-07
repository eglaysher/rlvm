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

/**
 * Interface to Gameexe. This code is not mine. I've already removed
 * the Singleton antipattern. (WTF, people!)
 *
 * @todo This class needs to be redesigned and reimplemented with a
 * more intuitive interface. This feels very C-ish and contains
 * *sprintfs*!
 */
class Gameexe {
public:
	// Return data, or defval if nothing matches the given key or index (or if it's a string not an integer).
	// For example, to read the line "#WINDOW.012.MOJI_REP=x,y", use
	//
	//   int x = Gameexe::instance().getInt("WINDOW.012.MOJI_REP", 0);
	//   int y = Gameexe::instance().getInt("WINDOW.012.MOJI_REP", 1);
	int getInt(const char* key, const int index = 0, const int defval = 0) const;
	
	// As above, but specialised for blocks.  For example, the above example could
	// also use
	//
	//   int x = Gameexe::instance().getInt("WINDOW", 12 ,"MOJI_REP", 0);
	//
	// Currently, this will fail if the ini file declared these as #WINDOW.12.MOJI_REP.  However,
	// this syntax has never been seen in the wild.
	int getInt(const char* section, const int secidx, const char* key, const int index = 0, const int defval = 0) const;
	int getInt(const char* section, const int sec1idx, const int sec2idx, const char* key, const int index = 0, const int defval = 0) const;
	
	// Similarly:
	const char* getStr(const char* key, const int index = 0, const char* defval = NULL) const;
	
	// Check for the existence of a key.
	bool mem(const char* key) const;

  size_t size() const { return data_.size(); }

  Gameexe(const std::string& file);
private:
	// Implementation details.
	typedef std::vector<int> vec_type;
	typedef hash_map<std::string, vec_type> data_t;
	data_t data_;
	std::vector<std::string> cdata_;
};

inline int
Gameexe::getInt(const char* key, const int index, const int defval) const
{
	data_t::const_iterator it = data_.find(std::string(key));
	if (it == data_.end()) return defval;
	const vec_type& vec = it->second;
	return vec.size() >= index + 1 ? vec[index] : defval;
}

inline bool
Gameexe::mem(const char* key) const
{
	return data_.find(std::string(key)) != data_.end();
}

inline int
Gameexe::getInt(const char* section, const int secidx, const char* key, const int index, const int defval) const
{
	char buf[128];
	sprintf(buf, "%s.%03d.%s", section, secidx, key);
	return getInt(buf, index, defval);
}

inline int
Gameexe::getInt(const char* section, const int sec1idx, const int sec2idx, const char* key, const int index, const int defval) const
{
	char buf[128];
	sprintf(buf, "%s.%03d.%03d.%s", section, sec1idx, sec2idx, key);
	return getInt(buf, index, defval);
}

inline const char*
Gameexe::getStr(const char* key, const int index, const char* defval) const
{
	int cidx = getInt(key, index, -1);
	return cidx < 0 || cidx >= cdata_.size() ? defval : cdata_[cidx].c_str();
}

#endif
