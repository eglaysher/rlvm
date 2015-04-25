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

#ifndef SRC_LIBREALLIVE_ARCHIVE_H_
#define SRC_LIBREALLIVE_ARCHIVE_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "libreallive/defs.h"
#include "libreallive/scenario.h"
#include "libreallive/filemap.h"

namespace libreallive {

namespace compression {
struct XorKey;
}  // namespace compression

// Interface to a loaded SEEN.TXT file.
class Archive {
 public:
  // Read an archive, assuming no per-game xor key. (Used in unit testing).
  explicit Archive(const string& filename);

  // Creates an interface to a SEEN.TXT file. Uses |regname| to look up
  // per-game xor key for newer games.
  Archive(const string& filename, const string& regname);
  ~Archive();

  typedef std::map<int, FilePos>::const_iterator const_iterator;
  const_iterator begin() { return scenarios_.cbegin(); }
  const_iterator end() { return scenarios_.cend(); }

  // Returns a specific scenario by |index| number or NULL if none exist.
  Scenario* GetScenario(int index);

  // Does a quick pass through all scenarios in the archive, looking for any
  // with non-default encoding. This short circuits when it finds one.
  int GetProbableEncodingType() const;

 private:
  typedef std::map<int, FilePos> scenarios_t;
  typedef std::map<int, std::unique_ptr<Scenario>> accessed_t;

  void ReadTOC();

  void ReadOverrides();

  scenarios_t scenarios_;
  accessed_t accessed_;
  string name_;
  Mapping info_;

  // Mappings to unarchived SEEN\d{4}.TXT files on disk.
  std::vector<std::unique_ptr<Mapping>> maps_to_delete_;

  // Now that VisualArts is using per game xor keys, this is equivalent to the
  // game's second level xor key.
  const compression::XorKey* second_level_xor_key_;

  // The #REGNAME key from the Gameexe.ini file. Passed down to Scenario for
  // prettier error messages.
  std::string regname_;
};

}  // namespace libreallive

#endif  // SRC_LIBREALLIVE_ARCHIVE_H_
