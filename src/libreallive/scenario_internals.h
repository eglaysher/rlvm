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

#ifndef SRC_LIBREALLIVE_SCENARIO_INTERNALS_H_
#define SRC_LIBREALLIVE_SCENARIO_INTERNALS_H_

#include <map>
#include <string>
#include <vector>

class Scenario;

struct FilePos {
  FilePos() : data(NULL), length(0) {}
  FilePos(const char* d, const size_t l) : data(d), length(l) {}

  const char* data;
  size_t length;
};

class Metadata {
 public:
  Metadata();
  const string& to_string() const { return as_string_; }
  const int text_encoding() const { return encoding_; }

  void Assign(const char* input);

 private:
  string as_string_;
  int encoding_;
};

class Header {
 public:
  Header(const char* data, const size_t length);
  ~Header();

  // Starting around the release of Little Busters!, scenario files has a
  // second round of xor done to them. When will they learn?
  bool use_xor_2_;

  long z_minus_one_;
  long z_minus_two_;
  long savepoint_message_;
  long savepoint_selcom_;
  long savepoint_seentop_;
  std::vector<string> dramatis_personae_;
  Metadata rldev_metadata_;
};

class Script {
 public:
  const pointer_t GetEntrypoint(int entrypoint) const;

 private:
  friend class Scenario;

  Script(const Header& hdr, const char* data, const size_t length,
         const std::string& regname,
         bool use_xor_2, const compression::XorKey* second_level_xor_key);
  ~Script();

  BytecodeList elts_;

  // Entrypoint handeling
  typedef std::map<int, pointer_t> pointernumber;
  pointernumber entrypoint_associations_;
};

#endif  // SRC_LIBREALLIVE_SCENARIO_INTERNALS_H_
