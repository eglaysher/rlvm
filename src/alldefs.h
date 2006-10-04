#include <cstdio>
#include <string>
#include <map>
#include <vector>
#include <memory>
#include <algorithm>
#include <boost/utility.hpp>
#include <boost/ptr_container/ptr_list.hpp>

using std::string;

namespace libReallive {

inline void
insert_i16(string& dest, int dpos, const int i16)
{
	dest[dpos++] = i16 & 0xff;
	dest[dpos]   = (i16 >> 8) & 0xff;
}
inline void
insert_i32(string& dest, int dpos, const int i32)
{
	dest[dpos++] = i32 & 0xff;
	dest[dpos++] = (i32 >> 8) & 0xff;
	dest[dpos++] = (i32 >> 16) & 0xff;
	dest[dpos]   = (i32 >> 24) & 0xff;
}
inline void
insert_i32(char* dest, const int i32)
{
	*dest++ = i32 & 0xff;
	*dest++ = (i32 >> 8) & 0xff;
	*dest++ = (i32 >> 16) & 0xff;
	*dest   = (i32 >> 24) & 0xff;
}

inline void 
append_i16(string& dest, const int i16)
{
	size_t dpos = dest.size();
	dest.resize(dpos + 2, i16 & 0xff);
	dest[++dpos] = (i16 >> 8) & 0xff;
}
inline void 
append_i32(string& dest, const int i32)
{
	size_t dpos = dest.size();
	dest.resize(dpos + 4, i32 & 0xff);
	dest[++dpos] = (i32 >> 8) & 0xff;
	dest[++dpos] = (i32 >> 16) & 0xff;
	dest[++dpos] = (i32 >> 24) & 0xff;
}

inline int
read_i16(const char* src)
{
	return src[0] | (src[1] << 8);
}
inline int
read_i16(const string& src, const int spos)
{
	return read_i16(src.data() + spos);
}
inline long
read_i32(const char* src)
{
	return src[0] | (src[1] << 8) | (src[2] << 16) | (src[3] << 24);
}
inline long
read_i32(const string& src, const int spos)
{
	return read_i32(src.data() + spos);
}

class Error : public std::exception {
	string description;
public:
  	virtual const char* what() const throw() { return description.c_str(); }
  	Error(string what) : description(what) {}
  	virtual ~Error() throw() {}
};

                              
}                              
