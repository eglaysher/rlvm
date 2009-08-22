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

/**
 * A Reference to an individual voice sample in a voice archive (independent of
 * the voice archive type).
 */
class VoiceSample {
 public:
  virtual ~VoiceSample();

  /**
   * Returns waveform data.
   *
   * @param[out] size The size of the returned buffer.
   * @return A buffer in WAV format.
   */
  virtual char* decode(int* size) = 0;
};

/**
 * Abstr
 */
class VoiceArchive : public boost::enable_shared_from_this<VoiceArchive> {
 public:
  explicit VoiceArchive(int file_no);
  ~VoiceArchive();

  virtual boost::shared_ptr<VoiceSample> findSample(int sample_num) = 0;

  int fileNumber() const { return file_no_; }

 private:
  int file_no_;
};  // end of class VoiceArchive

#endif  // SRC_SYSTEMS_BASE_VOICEARCHIVE_HPP_
