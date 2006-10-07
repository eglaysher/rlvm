
#include "filemap.h"
#include "mman.h"
#ifdef WIN32
#include <windows.h>
#else
#include <fcntl.h>
#ifndef O_BINARY
const int O_BINARY = 0;
#endif
const HANDLE INVALID_HANDLE_VALUE = -1;
#endif

#include <algorithm>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

namespace libReallive {

#ifdef WIN32
const int O_BINARY = 0;
const int O_RDONLY = GENERIC_READ;
const int O_RDWR = GENERIC_READ | GENERIC_WRITE;
inline HANDLE
open(const char* filename, int mode, int ignored)
{
	return CreateFile(filename, mode, FILE_SHARE_READ | FILE_SHARE_WRITE,
	                  NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
}
inline void
close(HANDLE fp)
{
	CloseHandle(fp);
}
#endif

void
Mapping::mclose()
{
	if (mem) munmap(mem, len);
	if (fp != INVALID_HANDLE_VALUE) close(fp);
}

void
Mapping::mopen()
{
	struct stat st;
	if (stat(fn_.c_str(), &st) != 0) {
          if (errno != ENOENT) throw Error("Could not open file");
          if (mode_ == Read) throw Error("File not found");
          if (msz_ == 0) throw Error("Cannot create empty file");
          len = msz_;
	}
	else {
          len = std::max(msz_, st.st_size);
	}
	fp = open(fn_.c_str(), O_BINARY | (mode_ == Read ? O_RDONLY : O_RDWR), 0644);
	if (fp == INVALID_HANDLE_VALUE) throw Error("Could not open file");
	mem = mmap(0, len, mode_ == Read ? PROT_READ : PROT_READ | PROT_WRITE, 0, fp, 0);
	if (mem == MAP_FAILED) {
		close(fp);
		throw Error("Could not map memory");
	}
}

Mapping::Mapping(string filename, Mode mode, off_t min_size)
: fp(INVALID_HANDLE_VALUE), mem(NULL), fn_(filename), mode_(mode), msz_(min_size)
{
	mopen();
}

Mapping::~Mapping()
{
	mclose();
}

void
Mapping::replace(string newfilename)
{
	mclose();
	if (unlink(fn_.c_str()) != 0 || rename(newfilename.c_str(), fn_.c_str()) != 0) {
		throw Error("Failed to replace file");
	}
	mopen();
}

}
