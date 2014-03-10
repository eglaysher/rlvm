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

#include "systems/base/object_settings.h"

#include <vector>

// -----------------------------------------------------------------------
// ObjectSettings
// -----------------------------------------------------------------------

ObjectSettings::ObjectSettings()
    : layer(0),
      space_key(0),
      obj_on_off(0),
      time_mod(0),
      disp_sort(0),
      init_mod(0),
      weather_on_off(0) {}

// -----------------------------------------------------------------------

ObjectSettings::ObjectSettings(const std::vector<int>& data)
    : layer(0),
      space_key(0),
      obj_on_off(0),
      time_mod(0),
      disp_sort(0),
      init_mod(0),
      weather_on_off(0) {
  if (data.size() > 0)
    layer = data[0];
  if (data.size() > 1)
    space_key = data[1];
  if (data.size() > 2)
    obj_on_off = data[2];
  if (data.size() > 3)
    time_mod = data[3];
  if (data.size() > 4)
    disp_sort = data[4];
  if (data.size() > 5)
    init_mod = data[5];
  if (data.size() > 6)
    weather_on_off = data[6];
}
