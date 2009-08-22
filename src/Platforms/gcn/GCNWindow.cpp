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
// -----------------------------------------------------------------------

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

#include "Platforms/gcn/GCNWindow.hpp"

#include "Platforms/gcn/gcnUtils.hpp"
#include "Systems/Base/Rect.hpp"

#include <iostream>
using namespace std;

ImageRect GCNWindow::s_border;

// -----------------------------------------------------------------------
// GCNWindow
// -----------------------------------------------------------------------
GCNWindow::GCNWindow(GCNPlatform* platform)
  : platform_(platform) {
  if (s_border.image == NULL) {
    s_border.image.reset(getThemeImage(IMG_VSCROLL_GREY));

    static int xpos[] = {0, 4, 7, 11};
    static int ypos[] = {0, 4, 15, 19};
    s_border.setCoordinates(xpos, ypos);
  }
}

// -----------------------------------------------------------------------

GCNWindow::~GCNWindow() { }

// -----------------------------------------------------------------------

void GCNWindow::centerInWindow(const Size& screen_size) {
  setPosition((screen_size.width() / 2) - (getWidth() / 2),
              (screen_size.height() / 2) - (getHeight() / 2));
}

// -----------------------------------------------------------------------

void GCNWindow::draw(gcn::Graphics* graphics) {
  GCNGraphics *g = static_cast<GCNGraphics*>(graphics);
  g->drawImageRect(0, 0, getWidth(), getHeight(), s_border);

  drawChildren(graphics);
}

