// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2009 Elliot Glaysher
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
// -----------------------------------------------------------------------

#include "systems/base/text_waku.h"

#include "systems/base/system.h"
#include "systems/base/text_waku_normal.h"
#include "systems/base/text_waku_type4.h"
#include "libreallive/gameexe.h"

// static
TextWaku* TextWaku::Create(System& system,
                           TextWindow& window,
                           int setno,
                           int no) {
  GameexeInterpretObject waku(system.gameexe()("WAKU", setno, "TYPE"));
  if (waku.ToInt(5) == 5) {
    return new TextWakuNormal(system, window, setno, no);
  } else {
    return new TextWakuType4(system, window, setno, no);
  }
}

TextWaku::~TextWaku() {}

void TextWaku::SetMousePosition(const Point& pos) {}

bool TextWaku::HandleMouseClick(RLMachine& machine,
                                const Point& pos,
                                bool pressed) {
  return false;
}
