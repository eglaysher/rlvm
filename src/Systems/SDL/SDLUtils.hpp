// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006 Elliot Glaysher
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

#ifndef __SDLUtils_hpp__
#define __SDLUtils_hpp__

#include <string>
#include <SDL/SDL.h>

class Rect;
class RGBColour;

void ShowGLErrors(void);
void reportSDLError(const std::string& sdlName,
                    const std::string& functionName);
int SafeSize(int i);

struct SDL_Surface;
SDL_Surface* AlphaInvert(SDL_Surface* inSurface);

void RectToSDLRect(const Rect& rect, SDL_Rect* out);

void RGBColourToSDLColor(const RGBColour& in, SDL_Color* out);

#endif
