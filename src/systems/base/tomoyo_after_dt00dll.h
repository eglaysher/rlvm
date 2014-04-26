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

#ifndef SRC_SYSTEMS_BASE_TOMOYO_AFTER_DT00DLL_H_
#define SRC_SYSTEMS_BASE_TOMOYO_AFTER_DT00DLL_H_

#include <string>

#include "machine/reallive_dll.h"

// This file exists to suppress spew. Comment once at the beginning that the
// RPG minigame DLL hasn't been reverse engineered and then ignore calls.
class TomoyoAfterDT00DLL : public RealLiveDLL {
 public:
  TomoyoAfterDT00DLL();
  virtual ~TomoyoAfterDT00DLL();

  // Overridden from RealLiveDLL:
  virtual int CallDLL(RLMachine& machine,
                      int func,
                      int arg1,
                      int arg2,
                      int arg3,
                      int arg4) override;
  virtual const std::string& GetDLLName() const override;
};

#endif  // SRC_SYSTEMS_BASE_TOMOYO_AFTER_DT00DLL_H_
