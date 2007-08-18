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
// the Free Software Foundation; either version 2 of the License, or
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

#ifndef __ObjectSettings_hpp__
#define __ObjectSettings_hpp__

#include <vector>

/**
 * Holds certain properties of regarding an object that are set from
 * the Gameexe.ini and are immutable inside of an instance.
 */
struct ObjectSettings
{
  ObjectSettings();
  ObjectSettings(const std::vector<int>& data);

  int layer;
  int spaceKey;
  int objOnOff;
  int timeMod;
  int dispSort;
  int initMod;
  int weatherOnOff;
};

#endif
