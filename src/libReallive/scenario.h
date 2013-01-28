// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of libReallive, a dependency of RLVM.
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

#ifndef SCENARIO_H
#define SCENARIO_H

#include "defs.h"
#include "bytecode.h"

namespace libReallive {

namespace Compression {
struct XorKey;
}  // namespace Compression

#include "scenario_internals.h"

class Scenario {
  Header header;
  Script script;
  int scenarioNum;
public:
  Scenario(const char* data, const size_t length, int scenarioNum,
           const std::string& regname,
           const Compression::XorKey* second_level_xor_key);
  Scenario(const FilePos& fp, int scenarioNum,
           const std::string& regname,
           const Compression::XorKey* second_level_xor_key);

  // Get the scenario number
  int sceneNumber() const { return scenarioNum; }

  // Get the text encoding used for this scenario
  int encoding() const { return header.rldev_metadata.text_encoding(); }

  // Access to script
  typedef BytecodeList::const_iterator const_iterator;
  typedef BytecodeList::iterator iterator;

  // Access to metadata in the script. Don't worry about information loss;
  // valid values are 0, 1, and 2.
  int savepointMessage() const { return header.savepoint_message; }
  int savepointSelcom()  const { return header.savepoint_selcom;  }
  int savepointSeentop() const { return header.savepoint_seentop; }

  /// Locate the entrypoint
  const_iterator findEntrypoint(int entrypoint) const;

  iterator begin();
  iterator end();
  const_iterator begin() const;
  const_iterator end() const;
  const size_t size() const;
};

// Inline definitions for Scenario

inline
Scenario::Scenario(const char* data, const size_t length, int sn,
                   const std::string& regname,
                   const Compression::XorKey* second_level_xor_key)
  : header(data, length),
    script(header, data, length, regname,
           header.use_xor_2, second_level_xor_key),
    scenarioNum(sn)
{
}

inline
Scenario::Scenario(const FilePos& fp, int sn,
                   const std::string& regname,
                   const Compression::XorKey* second_level_xor_key)
  : header(fp.data, fp.length),
    script(header, fp.data, fp.length, regname,
           header.use_xor_2, second_level_xor_key),
    scenarioNum(sn)
{
}

inline Scenario::iterator
Scenario::begin()
{
  return script.elts.begin();
}

inline Scenario::iterator
Scenario::end()
{
  return script.elts.end();
}

inline Scenario::const_iterator
Scenario::begin() const
{
  return script.elts.begin();
}

inline Scenario::const_iterator
Scenario::end() const
{
  return script.elts.end();
}

inline const size_t
Scenario::size() const
{
  return script.elts.size();
}

}

#endif
