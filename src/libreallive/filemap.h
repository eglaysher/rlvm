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

#ifndef SRC_LIBREALLIVE_FILEMAP_H_
#define SRC_LIBREALLIVE_FILEMAP_H_

#include <cstdio>
#include <string>

#ifdef WIN32
#include <windows.h>
#else
typedef int HANDLE;
#endif

#include "libreallive/defs.h"

namespace libreallive {

enum Mode { Read, Write };

class Mapping {
 public:
  Mapping(string filename, Mode mode, off_t min_size = 0);
  ~Mapping();

  // Return a pointer to the internal memory.
  char* get() { return (char*)mem; }

  size_t size() const { return len; }

 private:
  void mopen();
  void mclose();

  HANDLE fp;
  void* mem;
  bool mapped;
  size_t len;

  string fn_;
  Mode mode_;
  off_t msz_;
};

}  // namespace libreallive

#endif  // SRC_LIBREALLIVE_FILEMAP_H_
