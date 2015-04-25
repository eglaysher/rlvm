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

#include "libreallive/archive.h"

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <cstring>
#include <string>

#include "libreallive/compression.h"

using boost::istarts_with;
using boost::iends_with;
namespace fs = boost::filesystem;

namespace libreallive {

Archive::Archive(const std::string& filename)
    : name_(filename), info_(filename, Read), second_level_xor_key_(NULL) {
  ReadTOC();
  ReadOverrides();
}

Archive::Archive(const std::string& filename, const std::string& regname)
    : name_(filename),
      info_(filename, Read),
      second_level_xor_key_(NULL),
      regname_(regname) {
  ReadTOC();
  ReadOverrides();

  if (regname == "KEY\\CLANNAD_FV") {
    second_level_xor_key_ =
        libreallive::compression::clannad_full_voice_xor_mask;
  } else if (regname ==
             "\x4b\x45\x59\x5c\x83\x8a\x83\x67\x83\x8b\x83"
             "\x6f\x83\x58\x83\x5e\x81\x5b\x83\x59\x81\x49") {
    second_level_xor_key_ = libreallive::compression::little_busters_xor_mask;
  } else if (regname ==
             "\x4b\x45\x59\x5c\x83\x8a\x83\x67\x83\x8b\x83\x6f\x83\x58\x83\x5e"
             "\x81\x5b\x83\x59\x81\x49\x82\x64\x82\x77") {
    // "KEY\<little busters in katakana>!EX", with all fullwidth latin
    // characters.
    second_level_xor_key_ =
        libreallive::compression::little_busters_ex_xor_mask;
  } else if (regname == "StudioMebius\\SNOWSE") {
    second_level_xor_key_ =
        libreallive::compression::snow_standard_edition_xor_mask;
  } else if (regname ==
             "\x4b\x45\x59\x5c\x83\x4e\x83\x68\x82\xed\x82\xd3\x82"
             "\xbd\x81\x5b") {
    // "KEY\<Kud Wafter in hiragana>"
    second_level_xor_key_ =
        libreallive::compression::kud_wafter_xor_mask;
  } else if (regname ==
             "\x4b\x45\x59\x5c\x83\x4e\x83\x68\x82\xed\x82\xd3\x82"
             "\xbd\x81\x5b\x81\x79\x91\x53\x94\x4e\x97\xee\x91\xce"
             "\x8f\xdb\x94\xc5\x81\x7a") {
    second_level_xor_key_ =
        libreallive::compression::kud_wafter_all_ages_xor_mask;
  }
}

Archive::~Archive() {}

Scenario* Archive::GetScenario(int index) {
  accessed_t::const_iterator at = accessed_.find(index);
  if (at != accessed_.end())
    return at->second.get();
  scenarios_t::const_iterator st = scenarios_.find(index);
  if (st != scenarios_.end()) {
    Scenario* scene =
        new Scenario(st->second, index, regname_, second_level_xor_key_);
    accessed_[index].reset(scene);
    return scene;
  }
  return NULL;
}

int Archive::GetProbableEncodingType() const {
  // Directly create Header objects instead of Scenarios. We don't want to
  // parse the entire SEEN file here.
  for (auto it = scenarios_.cbegin(); it != scenarios_.cend(); ++it) {
    Header header(it->second.data, it->second.length);
    if (header.rldev_metadata_.text_encoding() != 0)
      return header.rldev_metadata_.text_encoding();
  }

  return 0;
}

void Archive::ReadTOC() {
  const char* idx = info_.get();
  for (int i = 0; i < 10000; ++i, idx += 8) {
    const int offs = read_i32(idx);
    if (offs)
      scenarios_[i] = FilePos(info_.get() + offs, read_i32(idx + 4));
  }
}

void Archive::ReadOverrides() {
  // Iterate over all files in the directory and override the table of contents
  // if there is a free SEENXXXX.TXT file.
  fs::path seen_dir = fs::path(name_).branch_path();
  fs::directory_iterator end;
  for (fs::directory_iterator it(seen_dir); it != end; ++it) {
    std::string filename = it->path().filename().string();
    if (filename.size() == 12 && istarts_with(filename, "seen") &&
        iends_with(filename, ".txt") && isdigit(filename[4]) &&
        isdigit(filename[5]) && isdigit(filename[6]) && isdigit(filename[7])) {
      Mapping* mapping = new Mapping((seen_dir / filename).string(), Read);
      maps_to_delete_.emplace_back(mapping);

      int index = std::stoi(filename.substr(4, 4));
      scenarios_[index] = FilePos(mapping->get(), mapping->size());
    }
  }
}

}  // namespace libreallive
