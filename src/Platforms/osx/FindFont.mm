// -*- tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
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

#include "FindFont.h"

#include <Cocoa/Cocoa.h>

#include "libReallive/gameexe.h"
#include "Utilities.h"

namespace fs = boost::filesystem;

extern "C" int findFontName(const char** fontName, const char* gamePath)
{
  Gameexe empty;
  empty("__GAMEPATH") = gamePath;
  fs::path pathToMsgothic = findFontFile(empty, "msgothic.ttc");
  if(pathToMsgothic.empty())
  {
    /* Couldn't find a copy of msgothic.ttc in either the home
     * directory nor in the game. Try to use the fallback: */
    NSString* font1 = @"/Library/Fonts/ヒラギノ角ゴ Pro W3.otf";
    NSFileManager *fm = [NSFileManager defaultManager];
    if([fm fileExistsAtPath:font1]) {
      *fontName = [font1 fileSystemRepresentation];
      return true;
    }
    else {
      NSAlert* alert = [[NSAlert alloc] init];
      [alert setMessageText:@"Couldn't find msgothic.ttc or a suitable fallback font."];
      [alert setInformativeText:@"Please place a copy of msgothic.ttc in either your home directory or in the game path."];
      [alert runModal];

      exit(-1);
    }
  }

  return false;
}
