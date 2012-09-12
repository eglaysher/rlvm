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

#include "Systems/Base/ToneCurve.hpp"

#include <fstream>
#include <sstream>
#include <string>
#include <boost/scoped_array.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include "MachineBase/Memory.hpp"
#include "MachineBase/RLMachine.hpp"
#include "Utilities/Exception.hpp"
#include "Utilities/File.hpp"
#include "libReallive/gameexe.h"
#include "libReallive/intmemref.h"
#include "xclannad/endian.hpp"
#include "xclannad/file.h"

#include <iostream>

using namespace std;
using boost::scoped_array;

namespace fs = boost::filesystem;

ToneCurve::ToneCurve() {
}

ToneCurve::ToneCurve(Gameexe& gameexe) {
  GameexeInterpretObject filename_key = gameexe("TONECURVE_FILENAME");
  if (!filename_key.exists()) {
    // It is perfectly valid not to have a tone curve key. All operations in this
    // class become noops.
    return;
  }

  string tonecurve = filename_key.to_string("");
  if (tonecurve == "") {
    // It is perfectly valid not to have a tone curve. All operations in this
    // class become noops.
    return;
  }

  fs::path basename = gameexe("__GAMEPATH").to_string();
  fs::path filename = correctPathCase(basename / "dat" / tonecurve);

  int size;
  scoped_array<char> data;
  if (loadFileData(filename, data, size)) {
    ostringstream oss;
    oss << "Could not read contents of file \"" << filename << "\".";
    throw rlvm::Exception(oss.str());
  }

  if (read_little_endian_int(data.get()) != 1000) {
    ostringstream oss;
    oss << "File '" << filename << "' is not a TCC file!";
    throw rlvm::Exception(oss.str());
  }

  effect_count_ = read_little_endian_int(data.get() + 4);
  tcc_info_.clear();
  int offset = 0xFE8;
  for (int i = 0; i < effect_count_; i++) {
    ToneCurveColorMap red;
    ToneCurveColorMap green;
    ToneCurveColorMap blue;
    ToneCurveRGBMap rgb;	
    unsigned char* ptr = &red[0];
    memcpy(ptr, data.get() + offset, 256);
    offset += 256;
    ptr = &green[0];
    memcpy(ptr, data.get() + offset, 256);
    offset += 256;
    ptr = &blue[0];
    memcpy(ptr, data.get() + offset, 256);
    offset += 256;
    rgb[0] = red;
    rgb[1] = green;
    rgb[2] = blue;
    tcc_info_.push_back(rgb);
    offset += 0x40;
  }
}

int ToneCurve::getEffectCount() const {
  return effect_count_;
}

ToneCurveRGBMap ToneCurve::getEffect(int index) {
  if(index >= getEffectCount() || index < 0) {
    ostringstream oss;
    oss << "Requested tone curve index " << index << " exceeds the amount of effects in the tone curve file.";
    throw rlvm::Exception(oss.str());    		
  }
  
  return tcc_info_[index];
}

ToneCurve::~ToneCurve() {
  tcc_info_.clear();
}
