// This file is part of libReallive, a dependency of RLVM. 
// 
// -----------------------------------------------------------------------
//
// Copyright (c) 2006 Peter Jolly
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

#ifndef LIBREALLIVE_H
#define LIBREALLIVE_H

/**
 * @namespace libReallive
 * @author Haeleth
 * @brief libReallive, a library for manipulating Reallive bytecode
 * 
 * libReallive is a library for manipulating Reallive bytecode. It was
 * orriginally written by Haeleth, and was copy/pasted into
 * RLVM. While I've modified it to suit my needs (for example, adding
 * support for RLv1.3 goto_with constructs, parameter parsing code in
 * expression.cpp, et cetera), 99% of this code was written by
 * Haeleth.
 *
 * In this namespace, you will find classses and function for reading
 * and manipulating Reallive SEEN files. This module is not documented
 * as well, since it is a third party library.
 */

#include "defs.h"     
#include "bytecode.h" 
#include "scenario.h" 
#include "archive.h"

#endif
