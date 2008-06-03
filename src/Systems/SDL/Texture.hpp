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
// POINT

// TODO: The entire Texture class's internals need to be transitioned
// to the Point and Rect classes.
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

  // TODO: Dead code?
  static unsigned int s_screenWidth;
  static unsigned int s_screenHeight;

  void renderToScreenAsColorMask_subtractive_glsl(
    const Rect& src, const Rect& dst, const RGBAColour& rgba);
  void renderToScreenAsColorMask_subtractive_fallback(
    const Rect& src, const Rect& dst, const RGBAColour& rgba);
  void renderToScreenAsColorMask_additive(
    const Rect& src, const Rect& dst, const RGBAColour& rgba);

  bool filterCoords(int& x1, int& y1, int& x2, int& y2, 
                    float& dx1, float& dy1, float& dx2, float& dy2);

public:
  static void SetScreenSize(const Size& s);

public:
  Texture(SDL_Surface* surface, int x, int y, int w, int h,
          unsigned int bytesPerPixel, int byteOrder, int byteType);
  Texture(render_to_texture, int screenWidth, int screenHeight);
  ~Texture();

  int width() { return m_logicalWidth; }
  int height() { return m_logicalHeight; }
  GLuint textureId() { return m_textureID; }

  void renderToScreenAsObject(
    const GraphicsObject& go, 
    SDLSurface& surface,
    const GraphicsObjectOverride& overrides);

  void renderToScreen(const Rect& src, const Rect& dst, int opacity);

  void renderToScreenAsColorMask(
    const Rect& src, const Rect& dst, const RGBAColour& rgba, int filter);

  void renderToScreen(const Rect& src, const Rect& dst,
                      const int opacity[4]);

  void rawRenderQuad(const int srcCoords[8], 
                     const int destCoords[8],
                     const int opacity[4]);

  void buildShader();
  std::string getSubtractiveShaderString();

};

#endif
