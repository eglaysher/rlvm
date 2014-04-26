// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2008 Elliot Glaysher
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
//
// -----------------------------------------------------------------------

#include "systems/base/cgm_table.h"

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "libreallive/gameexe.h"
#include "libreallive/intmemref.h"
#include "machine/memory.h"
#include "machine/rlmachine.h"
#include "utilities/exception.h"
#include "utilities/file.h"
#include "xclannad/endian.hpp"
#include "xclannad/file.h"

namespace fs = boost::filesystem;

static unsigned char cgm_xor_key[256] = {
    0x8b, 0xe5, 0x5d, 0xc3, 0xa1, 0xe0, 0x30, 0x44, 0x00, 0x85, 0xc0, 0x74,
    0x09, 0x5f, 0x5e, 0x33, 0xc0, 0x5b, 0x8b, 0xe5, 0x5d, 0xc3, 0x8b, 0x45,
    0x0c, 0x85, 0xc0, 0x75, 0x14, 0x8b, 0x55, 0xec, 0x83, 0xc2, 0x20, 0x52,
    0x6a, 0x00, 0xe8, 0xf5, 0x28, 0x01, 0x00, 0x83, 0xc4, 0x08, 0x89, 0x45,
    0x0c, 0x8b, 0x45, 0xe4, 0x6a, 0x00, 0x6a, 0x00, 0x50, 0x53, 0xff, 0x15,
    0x34, 0xb1, 0x43, 0x00, 0x8b, 0x45, 0x10, 0x85, 0xc0, 0x74, 0x05, 0x8b,
    0x4d, 0xec, 0x89, 0x08, 0x8a, 0x45, 0xf0, 0x84, 0xc0, 0x75, 0x78, 0xa1,
    0xe0, 0x30, 0x44, 0x00, 0x8b, 0x7d, 0xe8, 0x8b, 0x75, 0x0c, 0x85, 0xc0,
    0x75, 0x44, 0x8b, 0x1d, 0xd0, 0xb0, 0x43, 0x00, 0x85, 0xff, 0x76, 0x37,
    0x81, 0xff, 0x00, 0x00, 0x04, 0x00, 0x6a, 0x00, 0x76, 0x43, 0x8b, 0x45,
    0xf8, 0x8d, 0x55, 0xfc, 0x52, 0x68, 0x00, 0x00, 0x04, 0x00, 0x56, 0x50,
    0xff, 0x15, 0x2c, 0xb1, 0x43, 0x00, 0x6a, 0x05, 0xff, 0xd3, 0xa1, 0xe0,
    0x30, 0x44, 0x00, 0x81, 0xef, 0x00, 0x00, 0x04, 0x00, 0x81, 0xc6, 0x00,
    0x00, 0x04, 0x00, 0x85, 0xc0, 0x74, 0xc5, 0x8b, 0x5d, 0xf8, 0x53, 0xe8,
    0xf4, 0xfb, 0xff, 0xff, 0x8b, 0x45, 0x0c, 0x83, 0xc4, 0x04, 0x5f, 0x5e,
    0x5b, 0x8b, 0xe5, 0x5d, 0xc3, 0x8b, 0x55, 0xf8, 0x8d, 0x4d, 0xfc, 0x51,
    0x57, 0x56, 0x52, 0xff, 0x15, 0x2c, 0xb1, 0x43, 0x00, 0xeb, 0xd8, 0x8b,
    0x45, 0xe8, 0x83, 0xc0, 0x20, 0x50, 0x6a, 0x00, 0xe8, 0x47, 0x28, 0x01,
    0x00, 0x8b, 0x7d, 0xe8, 0x89, 0x45, 0xf4, 0x8b, 0xf0, 0xa1, 0xe0, 0x30,
    0x44, 0x00, 0x83, 0xc4, 0x08, 0x85, 0xc0, 0x75, 0x56, 0x8b, 0x1d, 0xd0,
    0xb0, 0x43, 0x00, 0x85, 0xff, 0x76, 0x49, 0x81, 0xff, 0x00, 0x00, 0x04,
    0x00, 0x6a, 0x00, 0x76};

CGMTable::CGMTable() {}

CGMTable::CGMTable(Gameexe& gameexe) {
  GameexeInterpretObject filename_key = gameexe("CGTABLE_FILENAME");
  if (!filename_key.Exists()) {
    // It is perfectly valid not to have a CG table key. All operations in this
    // class become noops.
    return;
  }

  std::string cgtable = filename_key.ToString("");
  if (cgtable == "") {
    // It is perfectly valid not to have a CG table. All operations in this
    // class become noops.
    return;
  }

  fs::path basename = gameexe("__GAMEPATH").ToString();
  fs::path filename = CorrectPathCase(basename / "dat" / cgtable);

  int size;
  std::unique_ptr<char[]> data;
  if (LoadFileData(filename, data, size)) {
    std::ostringstream oss;
    oss << "Could not read contents of file \"" << filename << "\".";
    throw rlvm::Exception(oss.str());
  }

  if (strncmp(data.get(), "CGTABLE", 7) != 0) {
    std::ostringstream oss;
    oss << "File '" << filename << "' is not a CGM file!";
    throw rlvm::Exception(oss.str());
  }

  int record_size = 36;
  if (data[7] == '2') {
    // I'm not sure when this started, but in Kud Wafter, the record size for
    // an entry in a CGM file seems to have changed. Possibly as a version 2
    // for the file? None of the rest of the added bytes appear to be set,
    // though.
    record_size = 60;
  }

  int cgm_size = read_little_endian_int(data.get() + 0x10);
  for (int i = 0; i < size - 0x20; i++) {
    data[i + 0x20] ^= cgm_xor_key[i & 0xff];
  }

  int dest_size = cgm_size * record_size;
  std::unique_ptr<char[]> dest_orig(new char[dest_size + 1024]);
  char* dest = dest_orig.get();
  char* src = data.get() + 0x28;
  ARCINFO::Extract2k(dest, src, dest + dest_size, data.get() + size);
  dest = dest_orig.get();
  for (int i = 0; i < cgm_size; i++) {
    char* s = dest + i * record_size;
    int n = read_little_endian_int(dest + i * record_size + 32);
    cgm_info_[s] = n;
  }
}

CGMTable::~CGMTable() {}

int CGMTable::GetTotal() const { return cgm_info_.size(); }

int CGMTable::GetViewed() const { return cgm_data_.size(); }

int CGMTable::GetPercent() const {
  // Prevent divide by zero
  if (GetTotal())
    return (GetViewed() / double(GetTotal())) * 100;
  else
    return 0;
}

int CGMTable::GetFlag(const std::string& filename) const {
  CGMMap::const_iterator it = cgm_info_.find(filename);
  if (it == cgm_info_.end())
    return -1;

  return it->second;
}

int CGMTable::GetStatus(const std::string& filename) const {
  int flag = GetFlag(filename);
  if (flag == -1)
    return -1;

  if (cgm_data_.find(flag) != cgm_data_.end())
    return 1;

  return 0;
}

void CGMTable::SetViewed(RLMachine& machine, const std::string& filename) {
  int flag = GetFlag(filename);

  if (flag != -1) {
    // Set the intZ[] flag
    machine.memory().SetIntValue(
        libreallive::IntMemRef(libreallive::INTZ_LOCATION, 0, flag), 1);

    cgm_data_.insert(flag);
  }
}
