// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2008 Elliot Glaysher
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

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

/**
 * @file   Module_Os.cpp
 * @author Elliot Glaysher
 * @date   Sun Aug 10 13:34:48 2008
 *
 * @brief  Implements commands that interact with the OS.
 */

#include "Modules/Module_Os.hpp"

#include "MachineBase/RLOperation.hpp"
#include "MachineBase/RLOperation/RLOp_Store.hpp"

#include <iostream>
#include <string>
using namespace std;

// -----------------------------------------------------------------------

/**
 * I have only the vaguest idea of what this is. This is used in the new
 * CLANNAD_FV game to do some sort of disk checking. If this returns false,
 * CLANNAD_FV will popup a message box telling you to insert the CLANNAD_FV
 * disc.
 *
 * This probably does some sort of file existance checking, but for now, just
 * always return true to get over this speed bump.
 */
struct Os_CheckFile
  : public RLOp_Store_3<StrConstant_T, IntConstant_T, StrConstant_T> {
  int operator()(RLMachine& machine, string one, int two, string three) {
    return 1;
  }
};

// -----------------------------------------------------------------------

OsModule::OsModule()
  : RLModule("Os", 1, 005) {
  addOpcode(114, 0, "(unknown)", new Os_CheckFile);
}
