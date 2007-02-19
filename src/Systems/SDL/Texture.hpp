// This file is part of RLVM, a RealLive virutal machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006 El Riot
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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//  
// -----------------------------------------------------------------------

/**
 * @file   Texture.hpp
 * @author Elliot Glaysher
 * @date   Sat Feb 17 20:21:59 2007
 * 
 * @brief  Contains an OpenGL texture and some metadata.
 */

#ifndef __Texture_hpp__
#define __Texture_hpp__

#include <SDL/SDL_opengl.h>

struct SDL_Surface;
class SDLSurface;
class GraphicsObject;

struct render_to_texture { };

class Texture
{
private:
  unsigned int m_logicalWidth;
  unsigned int m_logicalHeight;

  unsigned int m_textureWidth;
  unsigned int m_textureHeight;

  GLuint m_textureID;

  bool m_isUpsideDown;

public:
  Texture(SDL_Surface* surface);
  Texture(render_to_texture, int screenWidth, int screenHeight);
  ~Texture();

  int width() { return m_logicalWidth; }
  int height() { return m_logicalHeight; }
  GLuint textureId() { return m_textureID; }

  void renderToScreenAsObject(const GraphicsObject& go, SDLSurface& surface);

  void renderToScreen(int x1, int y1, int x2, int y2,
                      int dx1, int dy1, int dx2, int dy2,
                      int opacity);
  void renderToScreen(int x1, int y1, int x2, int y2,
                      int dx1, int dy1, int dx2, int dy2,
                      const int opacity[4]);

  void rawRenderQuad(const int srcCoords[8], 
                     const int destCoords[8],
                     const int opacity[4]);

};

#endif
