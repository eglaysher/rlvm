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

#include "boost/date_time/gregorian/gregorian.hpp"
#include "boost/date_time/posix_time/posix_time_types.hpp"

namespace datetime {

// -----------------------------------------------------------------------

int GetYear() { return boost::gregorian::day_clock::local_day().year(); }

// -----------------------------------------------------------------------

int GetMonth() { return boost::gregorian::day_clock::local_day().month(); }

// -----------------------------------------------------------------------

int GetDay() { return boost::gregorian::day_clock::local_day().day(); }

// -----------------------------------------------------------------------

int GetDayOfWeek() {
  return boost::gregorian::day_clock::local_day().day_of_week();
}

// -----------------------------------------------------------------------

int GetHour() {
  return boost::posix_time::second_clock::local_time().time_of_day().hours();
}

// -----------------------------------------------------------------------

int GetMinute() {
  return boost::posix_time::second_clock::local_time().time_of_day().minutes();
}

// -----------------------------------------------------------------------

int GetSecond() {
  return boost::posix_time::second_clock::local_time().time_of_day().seconds();
}

// -----------------------------------------------------------------------

int GetMs() {
  return boost::posix_time::second_clock::local_time()
      .time_of_day()
      .fractional_seconds();
}

}  // namespace datetime
