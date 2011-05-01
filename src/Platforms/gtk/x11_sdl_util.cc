// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2011 Elliot Glaysher
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

#include "Platforms/gtk/x11_sdl_util.h"

#include <X11/Xlib.h>
#include <SDL/SDL.h>
#include <SDL/SDL_syswm.h>

#include "Systems/Base/Rect.hpp"

Rect GetSDLWindowPosition() {
  SDL_SysWMinfo info;
  SDL_VERSION(&info.version);
  if (SDL_GetWMInfo(&info)) {
    info.info.x11.lock_func();

    // Retrieve the location of the SDL window.
    Window root;
    int x, y;
    unsigned int width, height;
    unsigned int border_width, depth;
    if (!XGetGeometry(info.info.x11.display, info.info.x11.window, &root,
                      &x, &y, &width, &height, &border_width, &depth)) {
      info.info.x11.unlock_func();
      return Rect();
    }

    Window child;
    if (!XTranslateCoordinates(info.info.x11.display, info.info.x11.window,
                               root, 0, 0, &x, &y, &child)) {
      info.info.x11.unlock_func();
      return Rect();
    }

    info.info.x11.unlock_func();

    return Rect(Point(x, y), Size(width, height));
  }

  return Rect();
}
