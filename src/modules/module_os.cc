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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
//
// -----------------------------------------------------------------------

#include "modules/module_os.h"

#include <string>

#include "machine/rloperation.h"
#include "machine/rloperation/rlop_store.h"

// -----------------------------------------------------------------------

namespace {

// I have only the vaguest idea of what this is. This is used in the new
// CLANNAD_FV game to do some sort of disk checking. If this returns false,
// CLANNAD_FV will popup a message box telling you to insert the CLANNAD_FV
// disc.
//
// This probably does some sort of file existance checking, but for now, just
// always return true to get over this speed bump.
struct CheckFile
    : public RLStoreOpcode<StrConstant_T, IntConstant_T, StrConstant_T> {
  int operator()(RLMachine& machine, string one, int two, string three) {
    return 1;
  }
};

}  // namespace

OsModule::OsModule() : RLModule("Os", 1, 005) {
  AddOpcode(114, 0, "(unknown)", new CheckFile);
}
