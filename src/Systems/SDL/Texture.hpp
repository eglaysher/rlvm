// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006 Elliot Glaysher
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

/**
 * Contains one or more OpenGL textures, representing a single image,
 * and provides a logical interface to working with them.
 */
class Texture
{
private:
  int m_xOffset;
  int m_yOffset;

  int m_logicalWidth;
  int m_logicalHeight;

  int m_totalWidth;
  int m_totalHeight;

  unsigned int m_textureWidth;
  unsigned int m_textureHeight;

  GLuint m_textureID;

  GLuint m_backTextureID;

  static GLuint m_shaderObjectID;
  static GLuint m_programObjectID;

  /// Is this texture upside down? (Because it's a screenshot, et cetera.)
  bool m_isUpsideDown;

  static unsigned int s_screenWidth;
  static unsigned int s_screenHeight;

  void renderToScreenAsColorMask_subtractive_glsl(
    int x1, int y1, int x2, int y2,
    int dx1, int dy1, int dx2, int dy2,
    int r, int g, int b, int alpha);
  void renderToScreenAsColorMask_subtractive_fallback(
    int x1, int y1, int x2, int y2,
    int dx1, int dy1, int dx2, int dy2,
    int r, int g, int b, int alpha);
  void renderToScreenAsColorMask_additive(
    int x1, int y1, int x2, int y2,
    int dx1, int dy1, int dx2, int dy2,
    int r, int g, int b, int alpha);

  bool filterCoords(int& x1, int& y1, int& x2, int& y2, int& dx1, 
                    int& dy1, int& dx2, int& dy2);

public:
  static void SetScreenSize(unsigned int screenWidth, unsigned int screenHeight);

public:
  Texture(SDL_Surface* surface, int x, int y, int w, int h,
          GLenum bytesPerPixel, GLint byteOrder, GLint byteType);
  Texture(render_to_texture, int screenWidth, int screenHeight);
  ~Texture();

  int width() { return m_logicalWidth; }
  int height() { return m_logicalHeight; }
  GLuint textureId() { return m_textureID; }

  void renderToScreenAsObject(const GraphicsObject& go, SDLSurface& surface);

  void renderToScreen(int x1, int y1, int x2, int y2,
                      int dx1, int dy1, int dx2, int dy2,
                      int opacity);

  void renderToScreenAsColorMask(
    int x1, int y1, int x2, int y2,
    int dx1, int dy1, int dx2, int dy2,
    int r, int g, int b, int alpha, int filter);

  void renderToScreen(int x1, int y1, int x2, int y2,
                      int dx1, int dy1, int dx2, int dy2,
                      const int opacity[4]);

  void rawRenderQuad(const int srcCoords[8], 
                     const int destCoords[8],
                     const int opacity[4]);

  void buildShader();
  std::string getSubtractiveShaderString();

};

#endif
