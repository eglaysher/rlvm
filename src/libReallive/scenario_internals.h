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
  void assign(const char* input);
  const string& to_string() const { return as_string; }
  const int text_encoding() const { return encoding; }

 private:
  string as_string;
  int encoding;
};

class Header {
public:
  Header(const char* data, const size_t length);

  // Starting around the release of Little Busters!, scenario files has a
  // second round of xor done to them. When will they learn?
  bool use_xor_2;

  long zminusone, zminustwo, savepoint_message,
       savepoint_selcom, savepoint_seentop;
  std::vector<string> dramatis_personae;
  Metadata rldev_metadata;
};

class Script {
public:
  const pointer_t getEntrypoint(int entrypoint) const;

private:
  friend class Scenario;

  Script(const Header& hdr, const char* data, const size_t length,
         const std::string& regname,
         bool use_xor_2, const Compression::XorKey* second_level_xor_key);

  // Recalculate all internal data lazily
  mutable bool uptodate;
  mutable size_t lencache;

  BytecodeList elts;
  bool strip;

  // Entrypoint handeling
  typedef std::map<int, pointer_t> pointernumber;
  pointernumber entrypointAssociations;
};
