// This file is part of libreallive, a dependency of RLVM.
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

#ifndef SRC_LIBREALLIVE_REALLIVE_H_
#define SRC_LIBREALLIVE_REALLIVE_H_

// libreallive is a library for manipulating Reallive bytecode. It was
// orriginally written by Haeleth, and was copy/pasted into
// RLVM. While I've modified it to suit my needs (for example, adding
// support for RLv1.3 goto_with constructs, parameter parsing code in
// expression.cpp, et cetera), 99% of this code was written by
// Haeleth.
//
// The Archive and Scenario access
//
// We start with the main class that represents the SEEN.TXT file,
// libreallive::Archive. A SEEN.TXT file contains all of the executed
// code in a Reallive game (barring DLL extensions to the Reallive
// system). A SEEN.TXT file contains number identified Scenarios,
// which represents small pieces of bytecode which are executed in our
// virtual machine. When we construct an Archive, we pass in the
// path to a SEEN.TXT file to load. Currently, the only thing done on
// startup is the parsing of the TOC, which defines which Scenarios
// are in the SEEN.TXT archive.
//
// From the Archive, we can access libreallive::Scenarios using the
// libreallive::Archive::scenario() member. This method will return
// the Scenario relating to the passed in number. Archive has other
// members for manipulating and rewriting the data, but these aren't
// used in RLVM.
//
// The Scenario
//
// The libreallive::Scenario class represents a Scenario, a sequence
// of commands and other metadata. It is divided into the
// libreallive::Header and libreallive::Script. The header contains:
//
// - Debug information
// - "Misc settings"
// - A list of actors that appear in the scene (referred to as the {@em
//   dramatic personae} table, which is used for debugging
// - Metadata which can be added by Haeleth's <a
//   href="http://dev.haeleth.net/rldev.shtml">RLdev</a> compiler.
//
// The Script contains:
//
// - A sequence of semi-parsed/tokenized bytecode elements, which are
//   the elements that RLMachine executes.
// - A list of entrypoints into the scenario
// - A list of pointers (for goto, et cetera)

#include "libreallive/archive.h"
#include "libreallive/bytecode.h"
#include "libreallive/defs.h"
#include "libreallive/scenario.h"

#endif  // SRC_LIBREALLIVE_REALLIVE_H_
