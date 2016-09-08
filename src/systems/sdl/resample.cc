// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2016 Elliot Glaysher
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
//
// Every library that I've seen that isn't tied to some sort of mixer library
// in itself seems to either require you to do conversions, bring in its own
// preferred file reading code, and/or pass in raw file descriptors for
// input/output.
//
// If I had any idea about audio programming (and wasn't facing a hard one
// week deadline), I could do a better job of this, but since I don't, write
// and read from tempfiles. This file is glue code to zresample.
//
// Patches welcome.

#include "systems/sdl/resample.h"

#include <stdlib.h>
#include <unistd.h>
#include <iostream>

#include "systems/sdl/zresample.h"
#include "vendor/xclannad/wavfile.h"

char* EnsureDataIsCorrectBitrate(char* incoming_data, int* length) {
  // TODO(erg): Fast path by just returning incoming_data if the bitrate is the
  // same as the desired rate.

  // Write |incoming_data| to a file.
  char dir_template[] = "/tmp/rlvm-temp-XXXXXX";
  char* tmp_dirname = mkdtemp(dir_template);
  if (tmp_dirname == nullptr) {
    std::cerr << "Warning! Failed to make temp dir for audio conversion."
              << std::endl;
    return incoming_data;
  }

  std::string infile_name = tmp_dirname;
  infile_name += "/input.wav";
  std::string outfile_name = tmp_dirname;
  outfile_name += "/output.wav";

  FILE* infile = fopen(infile_name.c_str(), "wb");
  fwrite(incoming_data, *length, 1, infile);
  fclose(infile);

  zresample_main(infile_name.c_str(), outfile_name.c_str(), WAVFILE::freq);

  FILE* f = fopen(outfile_name.c_str(), "rb");
  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  *length = fsize;
  fseek(f, 0, SEEK_SET);

  char* outdata = new char[fsize];
  fread(outdata, fsize, 1, f);
  fclose(f);

  unlink(infile_name.c_str());
  unlink(outfile_name.c_str());
  rmdir(tmp_dirname);

  return outdata;
}
