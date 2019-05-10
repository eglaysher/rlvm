// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2007 Elliot Glaysher
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

#ifndef SRC_MACHINE_SAVE_GAME_HEADER_H_
#define SRC_MACHINE_SAVE_GAME_HEADER_H_

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <string>

// Header structure written to and read from the start of each save
// game file. This structure is at the top of the file since it is
// what gets queried by SaveDate, SaveTime, etc.
struct SaveGameHeader {
  SaveGameHeader();
  explicit SaveGameHeader(const std::string& in_title);
  ~SaveGameHeader();

  // The title of the current saved game
  std::string title;

  // The time the save file was created.
  boost::posix_time::ptime save_time;

  // boost::serialization support
  template <class Archive>
  void serialize(Archive& ar, unsigned int version) {
    ar& title& save_time;
  }
};

#endif  // SRC_MACHINE_SAVE_GAME_HEADER_H_
