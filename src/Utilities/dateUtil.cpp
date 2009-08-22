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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// -----------------------------------------------------------------------

#include "boost/date_time/gregorian/gregorian.hpp"
#include "boost/date_time/posix_time/posix_time_types.hpp"

namespace datetime {

// -----------------------------------------------------------------------

int getYear() {
  return boost::gregorian::day_clock::local_day().year();
}

// -----------------------------------------------------------------------

int getMonth() {
  return boost::gregorian::day_clock::local_day().month();
}

// -----------------------------------------------------------------------

int getDay() {
  return boost::gregorian::day_clock::local_day().day();
}

// -----------------------------------------------------------------------

int getDayOfWeek() {
  return boost::gregorian::day_clock::local_day().day_of_week();
}

// -----------------------------------------------------------------------

int getHour() {
  return boost::posix_time::second_clock::local_time().time_of_day().hours();
}

// -----------------------------------------------------------------------

int getMinute() {
  return boost::posix_time::second_clock::local_time().time_of_day().minutes();
}

// -----------------------------------------------------------------------

int getSecond() {
  return boost::posix_time::second_clock::local_time().time_of_day().seconds();
}

// -----------------------------------------------------------------------

int getMs() {
  return boost::posix_time::second_clock::local_time().time_of_day().fractional_seconds();
}

}
