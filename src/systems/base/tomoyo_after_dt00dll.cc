// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2013 Elliot Glaysher
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

#include "systems/base/tomoyo_after_dt00dll.h"

#include <iostream>

TomoyoAfterDT00DLL::TomoyoAfterDT00DLL() {
  std::cerr << "WARNING: Tomoyo After: Dungeons & Takafumis is implemented in"
            << " a DLL and hasn't been reverse engineered yet." << std::endl;
}

TomoyoAfterDT00DLL::~TomoyoAfterDT00DLL() {}

int TomoyoAfterDT00DLL::CallDLL(RLMachine& machine,
                                int func,
                                int arg1,
                                int arg2,
                                int arg3,
                                int arg4) {
  // Perform no spew.
  return 0;
}

const std::string& TomoyoAfterDT00DLL::GetDLLName() const {
  static std::string n("DT00");
  return n;
}
