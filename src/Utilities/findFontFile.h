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

#ifndef __findFontFile_h__
#define __findFontFile_h__

// This file is seperated out from Utilities because of compile errors
// under OSX, where all of my Point/Size/Rect classes clash with a
// MacTypes.h file.

#include <boost/filesystem.hpp>

class Gameexe;
class RLMachine;

/**
 * Hack. Finds msgothic.ttc which is the only font file we support
 * right now. Once I go in and allow the selection of arbitrary fonts,
 * this method and the other findFontFile need to go away.
 */
boost::filesystem::path findFontFile(RLMachine& machine);

// -----------------------------------------------------------------------

/**
 * Attempts to find the file fileName in the home directory. Returns a
 * non-empty fs::path object on success.
 *
 * @todo This method contains an ugly hack to make things better for
 *       the user while I've not wrotten correct font support; the
 *       Gameexe can have a key "__FONTFOLDER" which will first be
 *       checked for the incoming file name.
 */
boost::filesystem::path findFontFile(
  Gameexe& gexe, const std::string& fileName);

#endif
