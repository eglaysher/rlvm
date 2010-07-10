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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
// -----------------------------------------------------------------------

#ifndef SRC_SYSTEMS_BASE_VOICEARCHIVE_HPP_
#define SRC_SYSTEMS_BASE_VOICEARCHIVE_HPP_

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

class VoiceArchive;

const int WAV_HEADER_SIZE = 0x2c;

// A Reference to an individual voice sample in a voice archive (independent of
// the voice archive type).
class VoiceSample {
 public:
  virtual ~VoiceSample();

  // Returns waveform data, putting the size of the buffer in |size|.
  virtual char* decode(int* size) = 0;

  static const char* MakeWavHeader(int rate, int ch, int bps, int size);
};

// Abstract representation of an archive on disk with a bunch of voice samples
// in it.
class VoiceArchive : public boost::enable_shared_from_this<VoiceArchive> {
 public:
  explicit VoiceArchive(int file_no);
  ~VoiceArchive();

  virtual boost::shared_ptr<VoiceSample> findSample(int sample_num) = 0;

  int fileNumber() const { return file_no_; }

 protected:
  // A sortable list with metadata pointing into an archive.
  struct Entry {
    Entry(int koe_num, int length, int offset);

    int koe_num;
    int length;
    int offset;

    bool operator<(const Entry& rhs) const {
      return koe_num < rhs.koe_num;
    }

    bool operator<(int rhs) const {
      return koe_num < rhs;
    }
  };

 private:
  int file_no_;
};  // end of class VoiceArchive

#endif  // SRC_SYSTEMS_BASE_VOICEARCHIVE_HPP_
