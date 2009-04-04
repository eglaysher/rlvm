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

#ifndef __OVKVoiceArchive_hpp__
#define __OVKVoiceArchive_hpp__

#include <boost/filesystem/path.hpp>

#include "Systems/Base/VoiceArchive.hpp"

/**
 * A VoiceArchive that reads the Ogg Vorbis archives (OVK files).
 */
class OVKVoiceArchive : public VoiceArchive {
 public:
  OVKVoiceArchive(boost::filesystem::path file, int file_no);
  ~OVKVoiceArchive();

  virtual boost::shared_ptr<VoiceSample> findSample(int sample_num);

 private:
  void readTable(boost::filesystem::path file);

  struct Entry {
    Entry(int koe_num, int length, int offset);

    int koe_num;
    int length;
    int offset;
  };

  // The file to read from
  boost::filesystem::path file_;

  // A list of samples in this archive
  std::vector<Entry> entries_;
};  // class OVKVoiceArchive

#endif
