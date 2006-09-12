#ifndef MMAP_H
#define MMAP_H

#include "defs.h"
#include <stdio.h>

#ifdef WIN32
#include <windows.h>
#else
typedef int HANDLE;
#endif

namespace LIBRL_NAMESPACE {

enum Mode { Read, Write };

class Mapping {
	void* mem;
	size_t len;
	HANDLE fp;
	
	string fn_;
	Mode mode_;
	off_t msz_;
	
	void mopen();
	void mclose();
public:
	// Return a pointer to the internal memory.
	char* get() { return (char*)mem; }
	
	// Close the file, delete it, rename a new file to the old file's name, and reopen it.
	void replace(string newfilename);
	
	Mapping(string filename, Mode mode, off_t min_size = 0);
	~Mapping();
};

}

#endif
