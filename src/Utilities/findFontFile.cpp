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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
//
// -----------------------------------------------------------------------

#include "Utilities/findFontFile.h"

#include <string>

#include "Systems/Base/System.hpp"
#include "libReallive/gameexe.h"

#include "boost/filesystem/operations.hpp"

#include <iostream>
using namespace std;

namespace fs = boost::filesystem;

const char* western_platform_fonts[] = {
#if defined(__APPLE__)
  // I would prefer Helvetica except that it is stored as a dfont. :(
  "/Library/Fonts/Arial.ttf",
#else
  "/usr/share/fonts/truetype/ttf-dejavu/DejaVuSans.ttf",
#endif
  NULL
};

const char* ja_platform_fonts[] = {
#if defined(__APPLE__)
  // Leopard
  "/Library/Fonts/ヒラギノ角ゴ Pro W3.otf",
  // Tiger
  "/System/Library/Fonts/ヒラギノ角ゴ Pro W3.otf",
#else
  "/etc/alternatives/fonts-japanese-gothic.ttf",
  // `ttf-japanese-gothic` is a symbolic link on Debian/Ubuntu systems.
  "/usr/share/fonts/truetype/ttf-japanese-gothic.ttf",
  "/usr/share/fonts/truetype/mona/mona.ttf",
  // Throw in Sazanami and Kochi just in case the above is broken.
  "/usr/share/fonts/truetype/sazanami/sazanami-gothic.ttf",
  "/usr/share/fonts/truetype/kochi/kochi-gothic-subst.ttf",
  "/usr/share/fonts/truetype/kochi/kochi-gothic.ttf",
#endif
  NULL
};

// -----------------------------------------------------------------------

fs::path findFontFileFinal(Gameexe& gexe, const std::string& fileName) {
  // HACK: Look for the font in the game
  if (gexe.exists("__GAMEPATH")) {
    std::string gamepath = gexe("__GAMEPATH");
    fs::path gamePathFont = fs::path(gamepath) / fileName;
    if (fs::exists(gamePathFont))
      return gamePathFont;
  }

  char* homeptr = getenv("HOME");
  char* rootptr = getenv("SYSTEMROOT");
  fs::path home;
  if (homeptr != 0) {
    home = homeptr;
  } else if (rootptr != 0) {
    home = rootptr;
    home /= "Fonts";
  }

  fs::path filePath = home / fileName;

  if (fs::exists(filePath))
    return filePath;
  else
    return fs::path();
}

// -----------------------------------------------------------------------

fs::path findFontFile(System& system) {
  Gameexe& gexe = system.gameexe();
  // HACK: If the user has overridden the __GAMEFONT, use it instead.
  if (gexe.exists("__GAMEFONT")) {
    std::string gamefontstr = gexe("__GAMEFONT");
    fs::path gameFont = fs::path(gamefontstr);
    if (fs::exists(gameFont))
      return gameFont;
  }

  if (system.useWesternFont()) {
    // Try to look up a western alternative font.
    for (const char** file = western_platform_fonts; *file; ++file) {
      if (fs::exists(*file))
        return fs::path(*file);
    }
  }

  // Look up platform specific Japanese alternatives.
  for (const char** file = ja_platform_fonts; *file; ++file) {
    if (fs::exists(*file))
      return fs::path(*file);
  }

  return findFontFileFinal(system.gameexe(), "msgothic.ttc");
}
