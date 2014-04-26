// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2009 Elliot Glaysher
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
// -----------------------------------------------------------------------

#ifndef SRC_SYSTEMS_BASE_VOICE_ARCHIVE_H_
#define SRC_SYSTEMS_BASE_VOICE_ARCHIVE_H_

#include <boost/filesystem/path.hpp>

#include <memory>
#include <vector>

class VoiceArchive;

const int WAV_HEADER_SIZE = 0x2c;

// A Reference to an individual voice sample in a voice archive (independent of
// the voice archive type).
class VoiceSample {
 public:
  virtual ~VoiceSample();

  // Returns waveform data, putting the size of the buffer in |size|.
  virtual char* Decode(int* size) = 0;

  static const char* MakeWavHeader(int rate, int ch, int bps, int size);
};

// Abstract representation of an archive on disk with a bunch of voice samples
// in it.
class VoiceArchive : public std::enable_shared_from_this<VoiceArchive> {
 public:
  explicit VoiceArchive(int file_number);
  virtual ~VoiceArchive();

  int file_number() const { return file_number_; }

  virtual std::shared_ptr<VoiceSample> FindSample(int sample_num) = 0;

 protected:
  // A sortable list with metadata pointing into an archive.
  struct Entry {
    Entry(int koe_num, int length, int offset);

    int koe_num;
    int length;
    int offset;

    bool operator<(const Entry& rhs) const { return koe_num < rhs.koe_num; }

    bool operator<(int rhs) const { return koe_num < rhs; }
  };

  // Reads and parses' VisualArt's simple audio table format into a
  // vector<Entry>.
  void ReadVisualArtsTable(boost::filesystem::path file,
                           int entry_length,
                           std::vector<Entry>& entries);

 private:
  int file_number_;
};  // end of class VoiceArchive

#endif  // SRC_SYSTEMS_BASE_VOICE_ARCHIVE_H_
