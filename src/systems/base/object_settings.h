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

#ifndef SRC_SYSTEMS_BASE_OBJECT_SETTINGS_H_
#define SRC_SYSTEMS_BASE_OBJECT_SETTINGS_H_

#include <vector>

// Holds certain properties of regarding an object that are set from
// the Gameexe.ini and are immutable inside of an instance.
struct ObjectSettings {
  ObjectSettings();
  explicit ObjectSettings(const std::vector<int>& data);

  // Purpose unknown
  int layer;

  // SpaceKey determines whether an object is considered part of the
  // UI layer or not; if it's non-zero, then the object is hidden
  // temporarily if the player hides the text window to view the
  // picture properly (typically by pressing the space key).
  int space_key;

  // ObjOnOff determines whether an object can be toggled on and off
  // independently by the player.  Valid values are 0, 1, and 2.
  // Values of 1 and 2 correspond to [Set]ShowObject1() and
  // [Set]ShowObject2(), and to \#SYSCOMs 18 and 19 respectively.
  int obj_on_off;

  // Purpose Unknown
  int time_mod;

  // Purpose Unknown
  int disp_sort;

  // Purpose Unknown. (Set to 1 in objects 08[1234] in CLANNAD)
  int init_mod;

  // WeatherOnOff determines whether the object is considered a
  // weather effect object or not; if it's non-zero, the object will
  // be shown and hidden with [Set]ShowWeather(), \#SYSCOM 17.
  int weather_on_off;
};

#endif  // SRC_SYSTEMS_BASE_OBJECT_SETTINGS_H_
