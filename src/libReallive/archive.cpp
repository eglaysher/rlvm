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

#include "archive.h"
#include "compression.h"
#include "string.h"

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

using namespace std;
using boost::istarts_with;
using boost::iends_with;
using boost::lexical_cast;
namespace fs = boost::filesystem;

namespace libReallive {

Archive::Archive(const string& filename)
  : name(filename), info(filename, Read), second_level_xor_key_(NULL) {
  readTOC();
  readOverrides();
}

Archive::Archive(const string& filename, const std::string& regname)
    : name(filename),
      info(filename, Read),
      second_level_xor_key_(NULL),
      regname_(regname) {
  readTOC();
  readOverrides();

  if (regname == "KEY\\CLANNAD_FV") {
    second_level_xor_key_ =
      libReallive::Compression::clannad_full_voice_xor_mask;
  } else if (regname == "\x4b\x45\x59\x5c\x83\x8a\x83\x67\x83\x8b\x83"
             "\x6f\x83\x58\x83\x5e\x81\x5b\x83\x59\x81\x49") {
    second_level_xor_key_ =
      libReallive::Compression::little_busters_xor_mask;
  } else if (regname ==
             "\x4b\x45\x59\x5c\x83\x8a\x83\x67\x83\x8b\x83\x6f\x83\x58\x83\x5e"
             "\x81\x5b\x83\x59\x81\x49\x82\x64\x82\x77") {
    // "KEY\<little busters in katakana>!EX", with all fullwidth latin
    // characters.
    second_level_xor_key_ =
        libReallive::Compression::little_busters_ex_xor_mask;
  } else if (regname == "StudioMebius\\SNOWSE") {
    second_level_xor_key_ =
        libReallive::Compression::snow_standard_edition_xor_mask;
  }
}

Archive::~Archive() {
  for (accessed_t::iterator it = accessed.begin(); it != accessed.end(); ++it)
    delete it->second;
}

void Archive::readTOC() {
	const char* idx = info.get();
	for (int i = 0; i < 10000; ++i, idx += 8) {
		const int offs = read_i32(idx);
		if (offs)
      scenarios[i] = FilePos(info.get() + offs, read_i32(idx + 4));
	}
}

void Archive::readOverrides() {
  // Iterate over all files in the directory and override the table of contents
  // if there is a free SEENXXXX.TXT file.
  fs::path seen_dir = fs::path(name).branch_path();
  fs::directory_iterator end;
  for (fs::directory_iterator it(seen_dir); it != end; ++it) {
    string filename = it->path().filename().string();
    if (filename.size() == 12 &&
        istarts_with(filename, "seen") &&
        iends_with(filename, ".txt") &&
        isdigit(filename[4]) &&
        isdigit(filename[5]) &&
        isdigit(filename[6]) &&
        isdigit(filename[7])) {
      Mapping* mapping = new Mapping((seen_dir / filename).string(), Read);
      maps_to_delete_.push_back(mapping);

      int index = lexical_cast<int>(filename.substr(4, 4));
      scenarios[index] = FilePos(mapping->get(), mapping->size());
    }
  }
}

Scenario*
Archive::scenario(int index) {
	accessed_t::const_iterator at = accessed.find(index);
	if (at != accessed.end())
    return at->second;
	scenarios_t::const_iterator st = scenarios.find(index);
	if (st != scenarios.end())
    return accessed[index] =
        new Scenario(st->second, index, regname_, second_level_xor_key_);
	return NULL;
}

int Archive::getProbableEncodingType() const {
  // Directly create Header objects instead of Scenarios. We don't want to
  // parse the entire SEEN file here.
  for (const_iterator it = scenarios.begin(); it != scenarios.end(); ++it) {
    Header header(it->second.data, it->second.length);
    if (header.rldev_metadata.text_encoding() != 0)
      return header.rldev_metadata.text_encoding();
  }

  return 0;
}

void
Archive::reset() {
	for (accessed_t::iterator it = accessed.begin(); it != accessed.end(); ++it) delete it->second;
	accessed.clear();
}

}
