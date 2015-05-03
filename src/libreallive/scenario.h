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

#ifndef SRC_LIBREALLIVE_SCENARIO_H_
#define SRC_LIBREALLIVE_SCENARIO_H_

#include <string>

#include "libreallive/defs.h"
#include "libreallive/bytecode.h"

namespace libreallive {

namespace compression {
struct XorKey;
}  // namespace compression

#include "libreallive/scenario_internals.h"

class Scenario {
 public:
  Scenario(const char* data, const size_t length, int scenarioNum,
           const std::string& regname,
           const compression::XorKey* second_level_xor_key);
  Scenario(const FilePos& fp, int scenarioNum,
           const std::string& regname,
           const compression::XorKey* second_level_xor_key);
  ~Scenario();

  // Get the scenario number
  int scene_number() const { return scenario_number_; }

  // Get the text encoding used for this scenario
  int encoding() const { return header.rldev_metadata_.text_encoding(); }

  // Access to metadata in the script. Don't worry about information loss;
  // valid values are 0, 1, and 2.
  int savepoint_message() const { return header.savepoint_message_; }
  int savepoint_selcom()  const { return header.savepoint_selcom_;  }
  int savepoint_seentop() const { return header.savepoint_seentop_; }

  // Access to script
  typedef BytecodeList::const_iterator const_iterator;
  typedef BytecodeList::iterator iterator;

  const_iterator begin() const  { return script.elts_.cbegin(); }
  const_iterator end() const    { return script.elts_.cend();   }

  // Locate the entrypoint
  const_iterator FindEntrypoint(int entrypoint) const;

 private:
  Header header;
  Script script;
  int scenario_number_;
};

}  // namespace libreallive

#endif  // SRC_LIBREALLIVE_SCENARIO_H_
