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
  const char* data;
  size_t length;
  FilePos() : data(NULL), length(0) {}
  FilePos(const char* d, const size_t l) : data(d), length(l) {}
};

class Metadata {
  string as_string;
  int encoding;
public:
  Metadata();
  void assign(const char* input);
  const string& to_string() const { return as_string; }
  const int text_encoding() const { return encoding; }
};

class Header {
  friend class Scenario;
  Header(const char* data, const size_t length);
public:
  /// Starting around the release of Little Busters!, scenario files has a
  /// second round of xor done to them. When will they learn?
  bool use_xor_2;

  long zminusone, zminustwo, savepoint_message,
       savepoint_selcom, savepoint_seentop;
  std::vector<string> dramatis_personae;
  Metadata rldev_metadata;
  const size_t dramatis_length() const {
    size_t rv = 0;
    for (std::vector<string>::const_iterator it = dramatis_personae.begin();
         it != dramatis_personae.end(); ++it) rv += it->size() + 5;
    return rv;
  }
};

class Script {
private:
  friend class Scenario;
  // Recalculate all internal data lazily
  mutable bool uptodate;
  mutable size_t lencache;

  BytecodeList elts;
  bool strip;

  Script(const Header& hdr, const char* data, const size_t length,
         const Compression::XorKey* second_level_xor_key);

  void recalculate(const bool force = false);

  // Recalculate offset data (do this before serialising)
  void update_offsets();

  // Pointer/label handling.
  typedef std::vector<pointer_t> pointer_list;
  typedef std::map<pointer_t, pointer_list> labelmap;

  labelmap labels;

  // Entrypoint handeling
  typedef std::map<int, pointer_t> pointernumber;
  pointernumber entrypointAssociations;

  void update_pointers(pointer_t&, pointer_t&);
  void remove_label(pointer_t&, pointer_t&);
  void remove_elt(pointer_t&);
public:
  // Flag size/offset recalculation as necessary (call when any data
  // has changed)
  void invalidate() { uptodate = false; }

  const size_t size() { recalculate(); return lencache; }

  const pointer_t getEntrypoint(int entrypoint) const;
};
