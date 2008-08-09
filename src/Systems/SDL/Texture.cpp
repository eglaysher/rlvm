// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006, 2007 Elliot Glaysher
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

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

#include "glew.h"

#include <boost/bind.hpp>
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>

#include <iostream>
#include <cstdio>
#include <sstream>
#include <fstream>
#include <string>
#include <algorithm>
#include <cmath>

#include "Systems/Base/SystemError.hpp"
#include "Systems/Base/GraphicsObject.hpp"
#include "Systems/Base/GraphicsObjectData.hpp"
#include "Systems/SDL/SDLGraphicsSystem.hpp"
#include "Systems/SDL/SDLSurface.hpp"
#include "Systems/SDL/Texture.hpp"
#include "Systems/SDL/SDLUtils.hpp"

#include "Systems/Base/SystemError.hpp"
#include "Systems/Base/Colour.hpp"

#include "alphablit.h"

using namespace std;
using namespace boost;

GLuint Texture::m_shaderObjectID = 0;
GLuint Texture::m_programObjectID = 0;

unsigned int Texture::s_screenWidth = 0;
unsigned int Texture::s_screenHeight = 0;

unsigned int Texture::s_uploadBufferSize = 0;
boost::scoped_array<char> Texture::s_uploadBuffer;

// -----------------------------------------------------------------------

void Texture::SetScreenSize(const Size& s)
{
  s_screenWidth = s.width();
  s_screenHeight = s.height();
}

// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
// Texture
// -----------------------------------------------------------------------
Texture::Texture(SDL_Surface* surface, int x, int y, int w, int h,
                 unsigned int bytesPerPixel, int byteOrder, int byteType)
  : m_xOffset(x), m_yOffset(y), m_logicalWidth(w), m_logicalHeight(h),
    m_totalWidth(surface->w), m_totalHeight(surface->h),
    m_textureWidth(SafeSize(m_logicalWidth)),
    m_textureHeight(SafeSize(m_logicalHeight)),
    m_backTextureID(0), m_isUpsideDown(false)
{
  glGenTextures(1, &m_textureID);
  glBindTexture(GL_TEXTURE_2D, m_textureID);
  ShowGLErrors();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  if(w == m_totalWidth && h == m_totalHeight)
  {
    SDL_LockSurface(surface);
    glTexImage2D(GL_TEXTURE_2D, 0, bytesPerPixel,
                 m_textureWidth, m_textureHeight,
                 0,
                 byteOrder, byteType, NULL);
    ShowGLErrors();

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, surface->w, surface->h,
                    byteOrder, byteType, surface->pixels);
    ShowGLErrors();

    SDL_UnlockSurface(surface);
  }
  else
  {
    // Cut out the current piece
    char* pixelData = uploadBuffer(surface->format->BytesPerPixel * w * h);
    char* curDstPtr = pixelData;

    SDL_LockSurface(surface);
    {
      char* curSrcPtr = (char*) surface->pixels;
      curSrcPtr += surface->pitch * y;

      int rowStart = surface->format->BytesPerPixel * x;
      int subrowSize = surface->format->BytesPerPixel * w;
      for(int currentRow = 0; currentRow < h; ++currentRow)
      {
        memcpy(curDstPtr, curSrcPtr + rowStart, subrowSize);
        curDstPtr += subrowSize;
        curSrcPtr += surface->pitch;
      }
    }
    SDL_UnlockSurface(surface);

    glTexImage2D(GL_TEXTURE_2D, 0, bytesPerPixel,
                 m_textureWidth, m_textureHeight,
                 0,
                 byteOrder, byteType, NULL);
    ShowGLErrors();

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h,
                    byteOrder, byteType, pixelData);
    ShowGLErrors();
  }
}

// -----------------------------------------------------------------------

Texture::Texture(render_to_texture, int width, int height)
  : m_xOffset(0), m_yOffset(0),
    m_logicalWidth(width), m_logicalHeight(height),
    m_totalWidth(width), m_totalHeight(height),
    m_textureWidth(0), m_textureHeight(0), m_textureID(0),
    m_backTextureID(0), m_isUpsideDown(true)
{
  glGenTextures(1, &m_textureID);
  glBindTexture(GL_TEXTURE_2D, m_textureID);
  ShowGLErrors();
//  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  m_textureWidth = SafeSize(m_logicalWidth);
  m_textureHeight = SafeSize(m_logicalHeight);

  // This may fail.
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
               m_textureWidth, m_textureHeight,
               0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  ShowGLErrors();

  glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, m_logicalWidth,
                      m_logicalHeight);
  ShowGLErrors();
}

// -----------------------------------------------------------------------

Texture::~Texture()
{
  glDeleteTextures(1, &m_textureID);

  if(m_backTextureID)
    glDeleteTextures(1, &m_backTextureID);

  ShowGLErrors();
}

// -----------------------------------------------------------------------

char* Texture::uploadBuffer(unsigned int size)
{
  if(!s_uploadBuffer || size > s_uploadBufferSize) {
    s_uploadBuffer.reset(new char[size]);
    s_uploadBufferSize = size;
  }

  return s_uploadBuffer.get();
}

// -----------------------------------------------------------------------

void Texture::reupload(SDL_Surface* surface, int x, int y, int w, int h,
                       unsigned int bytesPerPixel, int byteOrder, int byteType)
{
  glBindTexture(GL_TEXTURE_2D, m_textureID);

  if(w == m_totalWidth && h == m_totalHeight)
  {
    SDL_LockSurface(surface);

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, surface->w, surface->h,
                    byteOrder, byteType, surface->pixels);
    ShowGLErrors();

    SDL_UnlockSurface(surface);
  }
  else
  {
    // Cut out the current piece
    char* pixelData = uploadBuffer(surface->format->BytesPerPixel * w * h);
    char* curDstPtr = pixelData;

    SDL_LockSurface(surface);
    {
      char* curSrcPtr = (char*) surface->pixels;
      curSrcPtr += surface->pitch * y;

      int rowStart = surface->format->BytesPerPixel * x;
      int subrowSize = surface->format->BytesPerPixel * w;
      for(int currentRow = 0; currentRow < h; ++currentRow)
      {
        memcpy(curDstPtr, curSrcPtr + rowStart, subrowSize);
        curDstPtr += subrowSize;
        curSrcPtr += surface->pitch;
      }
    }
    SDL_UnlockSurface(surface);

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h,
                    byteOrder, byteType, pixelData);
    ShowGLErrors();
  }
}

// -----------------------------------------------------------------------

std::string readTextFile(const std::string& file)
{
  ifstream ifs(file.c_str());
  if(!ifs)
  {
    ostringstream oss;
    oss << "Can't open text file: " << file;
    throw SystemError(oss.str());
  }

  string out, line;
  while(getline(ifs, line))
  {
    out += line;
    out += '\n';
  }

  return out;
}

// -----------------------------------------------------------------------

void printARBLog(GLhandleARB obj)
{
  char str[256];
  GLsizei size = 0;
  glGetInfoLogARB(obj, 256, &size, str);
  if(size != 0)
  {
    cerr << "Log: " << str << endl;
  }
}

// -----------------------------------------------------------------------

string Texture::getSubtractiveShaderString()
{
  string x =
    "uniform sampler2D currentValues, mask;"
    ""
    "void main()"
    "{"
    "vec4 bgColor = texture2D(currentValues, gl_TexCoord[0].st);"
    "vec4 maskVector = texture2D(mask, gl_TexCoord[1].st);"
    "float maskColor = clamp(maskVector.a * gl_Color.a, 0.0, 1.0);"
    "gl_FragColor = clamp(bgColor - maskColor + gl_Color * maskColor, 0.0, 1.0);"
    "}";

  return x;
}

// -----------------------------------------------------------------------

void Texture::buildShader()
{
  m_shaderObjectID = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
  ShowGLErrors();

//  string str = readTextFile("/Users/elliot/Projects/rlvm/src/Systems/SDL/subtractive.frag");
  string str = getSubtractiveShaderString();
  const char* file = str.c_str();

  glShaderSourceARB(m_shaderObjectID, 1, &file, NULL);
  ShowGLErrors();

  glCompileShaderARB(m_shaderObjectID);
  printARBLog(m_shaderObjectID);
  ShowGLErrors();

  // Check the log here

  m_programObjectID = glCreateProgramObjectARB();
  glAttachObjectARB(m_programObjectID, m_shaderObjectID);
  ShowGLErrors();

  glLinkProgramARB(m_programObjectID);
  printARBLog(m_programObjectID);
  ShowGLErrors();
}

// -----------------------------------------------------------------------

// This is really broken and brain dead.
void Texture::renderToScreen(const Rect& src, const Rect& dst, int opacity)
{
  int x1 = src.x(), y1 = src.y(), x2 = src.x2(), y2 = src.y2();
  float fdx1 = dst.x(), fdy1 = dst.y(), fdx2 = dst.x2(), fdy2 = dst.y2();
  if(!filterCoords(x1, y1, x2, y2, fdx1, fdy1, fdx2, fdy2))
    return;

  // For the time being, we are dumb and assume that it's one texture

  float thisx1 = float(x1) / m_textureWidth;
  float thisy1 = float(y1) / m_textureHeight;
  float thisx2 = float(x2) / m_textureWidth;
  float thisy2 = float(y2) / m_textureHeight;

  if(m_isUpsideDown)
  {
    thisy1 = float(m_logicalHeight - y1) / m_textureHeight;
    thisy2 = float(m_logicalHeight - y2) / m_textureHeight;
  }

  glBindTexture(GL_TEXTURE_2D, m_textureID);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glBegin(GL_QUADS);
  {
    glColor4ub(255, 255, 255, opacity);
    glTexCoord2f(thisx1, thisy1);
    glVertex2f(fdx1, fdy1);
    glTexCoord2f(thisx2, thisy1);
    glVertex2f(fdx2, fdy1);
    glTexCoord2f(thisx2, thisy2);
    glVertex2f(fdx2, fdy2);
    glTexCoord2f(thisx1, thisy2);
    glVertex2f(fdx1, fdy2);
  }
  glEnd();
  glBlendFunc(GL_ONE, GL_ZERO);
}

// -----------------------------------------------------------------------

/**
 * @todo A function of this hairiness needs super more amounts of
 *       documentation.
 * @todo When I merge back to trunk, make sure to change the throw
 *       cstrs over to the new exception class.
 */
void Texture::renderToScreenAsColorMask(
  const Rect& src, const Rect& dst, const RGBAColour& rgba, int filter)
{
  if(filter == 0)
  {
    if(GLEW_ARB_fragment_shader && GLEW_ARB_multitexture)
    {
      renderToScreenAsColorMask_subtractive_glsl(src, dst, rgba);
    }
    else
    {
      renderToScreenAsColorMask_subtractive_fallback(
        src, dst, rgba);
    }
  }
  else
  {
    renderToScreenAsColorMask_additive(
      src, dst, rgba);
  }
}

// -----------------------------------------------------------------------

void Texture::renderToScreenAsColorMask_subtractive_glsl(
  const Rect& src, const Rect& dst, const RGBAColour& rgba)
{
  int x1 = src.x(), y1 = src.y(), x2 = src.x2(), y2 = src.y2();
  float fdx1 = dst.x(), fdy1 = dst.y(), fdx2 = dst.x2(), fdy2 = dst.y2();
  if(!filterCoords(x1, y1, x2, y2, fdx1, fdy1, fdx2, fdy2))
    return;

  if(m_shaderObjectID == 0)
    buildShader();

  float thisx1 = float(x1) / m_textureWidth;
  float thisy1 = float(y1) / m_textureHeight;
  float thisx2 = float(x2) / m_textureWidth;
  float thisy2 = float(y2) / m_textureHeight;

  if(m_isUpsideDown)
  {
    thisy1 = float(m_logicalHeight - y1) / m_textureHeight;
    thisy2 = float(m_logicalHeight - y2) / m_textureHeight;
  }

  // If we haven't already, allocate video memory for the back
  // texture.
  //
  // NOTE: Does this code deal with changing the dimensions of the
  // text box? Does it matter?
  if(m_backTextureID == 0)
  {
    glGenTextures(1, &m_backTextureID);
    glBindTexture(GL_TEXTURE_2D, m_backTextureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Generate this texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 m_textureWidth, m_textureHeight,
                 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    ShowGLErrors();
  }

  // Copy the current value of the region where we're going to render
  // to a texture for input to the shader
  glBindTexture(GL_TEXTURE_2D, m_backTextureID);
  int ystart = int(s_screenHeight - fdy1 - (fdy2 - fdy1));
  int idx1 = int(fdx1);
  glCopyTexSubImage2D(GL_TEXTURE_2D,
                      0,
                      0, 0,
                      idx1, ystart, m_textureWidth, m_textureHeight);
  ShowGLErrors();

  glUseProgramObjectARB(m_programObjectID);

  // Put the backTexture in texture slot zero and set this to be the
  // texture "currentValues" in the above shader program.
  glActiveTextureARB(GL_TEXTURE0_ARB);
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, m_backTextureID);
  GLint currentValuesLoc = glGetUniformLocationARB(m_programObjectID,
                                                   "currentValues");
  if(currentValuesLoc == -1)
    throw SystemError("Bad uniform value");
  glUniform1iARB(currentValuesLoc, 0);

  // Put the mask in texture slot one and set this to be the
  // texture "mask" in the above shader program.
  glActiveTextureARB(GL_TEXTURE1_ARB);
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, m_textureID);
  GLint maskLoc = glGetUniformLocationARB(m_programObjectID, "mask");
  if(maskLoc == -1)
    throw SystemError("Bad uniform value");
  glUniform1iARB(maskLoc, 1);

  glDisable(GL_BLEND);

  glBegin(GL_QUADS);
  {
    glColorRGBA(rgba);
    glMultiTexCoord2fARB(GL_TEXTURE0_ARB, thisx1, thisy2);
    glMultiTexCoord2fARB(GL_TEXTURE1_ARB, thisx1, thisy1);
    glVertex2f(fdx1, fdy1);
    glMultiTexCoord2fARB(GL_TEXTURE0_ARB, thisx2, thisy2);
    glMultiTexCoord2fARB(GL_TEXTURE1_ARB, thisx2, thisy1);
    glVertex2f(fdx2, fdy1);
    glMultiTexCoord2fARB(GL_TEXTURE0_ARB, thisx2, thisy1);
    glMultiTexCoord2fARB(GL_TEXTURE1_ARB, thisx2, thisy2);
    glVertex2f(fdx2, fdy2);
    glMultiTexCoord2fARB(GL_TEXTURE0_ARB, thisx1, thisy1);
    glMultiTexCoord2fARB(GL_TEXTURE1_ARB, thisx1, thisy2);
    glVertex2f(fdx1, fdy2);
  }
  glEnd();

  glActiveTextureARB(GL_TEXTURE1_ARB);
  glDisable(GL_TEXTURE_2D);
  glActiveTextureARB(GL_TEXTURE0_ARB);

  glUseProgramObjectARB(0);
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ZERO);
}

// -----------------------------------------------------------------------

/**
 * This fallback does not accurately render the scene according to
 * standard RealLive. This only negatively shades according to the
 * alpha value, ignoring the rest of the \#WINDOW_ATTR color.
 *
 * This will probably only occur with mesa software and people with
 * graphics cards > 5 years old.
 */
void Texture::renderToScreenAsColorMask_subtractive_fallback(
  const Rect& src, const Rect& dst, const RGBAColour& rgba)
{
  int x1 = src.x(), y1 = src.y(), x2 = src.x2(), y2 = src.y2();
  float fdx1 = dst.x(), fdy1 = dst.y(), fdx2 = dst.x2(), fdy2 = dst.y2();
  if(!filterCoords(x1, y1, x2, y2, fdx1, fdy1, fdx2, fdy2))
    return;

  float thisx1 = float(x1) / m_textureWidth;
  float thisy1 = float(y1) / m_textureHeight;
  float thisx2 = float(x2) / m_textureWidth;
  float thisy2 = float(y2) / m_textureHeight;

  if(m_isUpsideDown)
  {
    thisy1 = float(m_logicalHeight - y1) / m_textureHeight;
    thisy2 = float(m_logicalHeight - y2) / m_textureHeight;
  }

  // First draw the mask
  glBindTexture(GL_TEXTURE_2D, m_textureID);

  /// SERIOUS WTF: glBlendFuncSeparate causes a segmentation fault
  /// under the current i810 driver for linux.
//  glBlendFuncSeparate(GL_SRC_ALPHA_SATURATE, GL_ONE_MINUS_SRC_ALPHA,
//                      GL_SRC_COLOR, GL_ONE_MINUS_SRC_ALPHA);
  glBlendFunc(GL_SRC_ALPHA_SATURATE, GL_ONE_MINUS_SRC_ALPHA);

  glBegin(GL_QUADS);
  {
    glColorRGBA(rgba);
    glTexCoord2f(thisx1, thisy1);
    glVertex2f(fdx1, fdy1);
    glTexCoord2f(thisx2, thisy1);
    glVertex2f(fdx2, fdy1);
    glTexCoord2f(thisx2, thisy2);
    glVertex2f(fdx2, fdy2);
    glTexCoord2f(thisx1, thisy2);
    glVertex2f(fdx1, fdy2);
  }
  glEnd();

  glBlendFunc(GL_ONE, GL_ZERO);
}

// -----------------------------------------------------------------------

void Texture::renderToScreenAsColorMask_additive(
  const Rect& src, const Rect& dst, const RGBAColour& rgba)
{
  int x1 = src.x(), y1 = src.y(), x2 = src.x2(), y2 = src.y2();
  float fdx1 = dst.x(), fdy1 = dst.y(), fdx2 = dst.x2(), fdy2 = dst.y2();
  if(!filterCoords(x1, y1, x2, y2, fdx1, fdy1, fdx2, fdy2))
    return;

  float thisx1 = float(x1) / m_textureWidth;
  float thisy1 = float(y1) / m_textureHeight;
  float thisx2 = float(x2) / m_textureWidth;
  float thisy2 = float(y2) / m_textureHeight;

  if(m_isUpsideDown)
  {
    thisy1 = float(m_logicalHeight - y1) / m_textureHeight;
    thisy2 = float(m_logicalHeight - y2) / m_textureHeight;
  }

  // First draw the mask
  glBindTexture(GL_TEXTURE_2D, m_textureID);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glBegin(GL_QUADS);
  {
    glColorRGBA(rgba);
    glTexCoord2f(thisx1, thisy1);
    glVertex2f(fdx1, fdy1);
    glTexCoord2f(thisx2, thisy1);
    glVertex2f(fdx2, fdy1);
    glTexCoord2f(thisx2, thisy2);
    glVertex2f(fdx2, fdy2);
    glTexCoord2f(thisx1, thisy2);
    glVertex2f(fdx1, fdy2);
  }
  glEnd();

  glBlendFunc(GL_ONE, GL_ZERO);
}

// -----------------------------------------------------------------------

void Texture::renderToScreen(const Rect& src, const Rect& dst,
                             const int opacity[4])
{
  // For the time being, we are dumb and assume that it's one texture
  int x1 = src.x(), y1 = src.y(), x2 = src.x2(), y2 = src.y2();
  float fdx1 = dst.x(), fdy1 = dst.y(), fdx2 = dst.x2(), fdy2 = dst.y2();
  if(!filterCoords(x1, y1, x2, y2, fdx1, fdy1, fdx2, fdy2))
    return;

  float thisx1 = float(x1) / m_textureWidth;
  float thisy1 = float(y1) / m_textureHeight;
  float thisx2 = float(x2) / m_textureWidth;
  float thisy2 = float(y2) / m_textureHeight;

  glBindTexture(GL_TEXTURE_2D, m_textureID);

  // Blend when we have less opacity
  if(find_if(opacity, opacity + 4, bind(std::less<int>(), _1, 255))
     != opacity + 4)
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glBegin(GL_QUADS);
  {
    glColor4ub(255, 255, 255, opacity[0]);
    glTexCoord2f(thisx1, thisy1);
    glVertex2f(fdx1, fdy1);
    glColor4ub(255, 255, 255, opacity[1]);
    glTexCoord2f(thisx2, thisy1);
    glVertex2f(fdx2, fdy1);
    glColor4ub(255, 255, 255, opacity[2]);
    glTexCoord2f(thisx2, thisy2);
    glVertex2f(fdx2, fdy2);
    glColor4ub(255, 255, 255, opacity[3]);
    glTexCoord2f(thisx1, thisy2);
    glVertex2f(fdx1, fdy2);
  }
  glEnd();
  glBlendFunc(GL_ONE, GL_ZERO);
}

// -----------------------------------------------------------------------

void Texture::renderToScreenAsObject(
  const GraphicsObject& go,
  SDLSurface& surface,
  const GraphicsObjectOverride& overrides)
{
  // Figure out the source to clip out of the image
  int pattNo = go.pattNo();
  int xSrc1 = surface.getPattern(pattNo).rect.x();
  int ySrc1 = surface.getPattern(pattNo).rect.y();
  int xSrc2 = surface.getPattern(pattNo).rect.x2();
  int ySrc2 = surface.getPattern(pattNo).rect.y2();
  int xOrigin = surface.getPattern(pattNo).originX;
  int yOrigin = surface.getPattern(pattNo).originY;

  if(overrides.overrideSource)
  {
    // POINT
    xSrc1 = overrides.srcX1;
    ySrc1 = overrides.srcY1;
    xSrc2 = overrides.srcX2;
    ySrc2 = overrides.srcY2;
  }

  // Figure out position to display on the screen
  int xPos1, yPos1, xPos2, yPos2;
  if(overrides.overrideDest)
  {
    xPos1 = overrides.dstX1; yPos1 = overrides.dstY1;
    xPos2 = overrides.dstX2; yPos2 = overrides.dstY2;
  }
  else
  {
    xPos1 = go.x() + go.xAdjustmentSum() - xOrigin;
    yPos1 = go.y() + go.yAdjustmentSum() - yOrigin;

    if(overrides.hasDestOffset)
    {
      xPos1 += overrides.dstX;
      yPos1 += overrides.dstY;
    }

    xPos2 = int(xPos1 + (xSrc2 - xSrc1) * (go.width() / 100.0f));
    yPos2 = int(yPos1 + (ySrc2 - ySrc1) * (go.height() / 100.0f));
  }

  // If clipping is active for this object, take that into account too.
  if (go.hasClip()) {
    // Do nothing if object falls wholly outside clip area
    if (xPos2 < go.clipX1() || xPos1 > go.clipX2() ||
        yPos2 < go.clipY1() || yPos1 > go.clipY2()) {
      return;
    }
    // Otherwise, adjust coordinates to present only the visible area.
    // POINT
    // TODO: Move this logic into an intersection of rectangles.
    if (xPos1 < go.clipX1()) {
      xSrc1 += go.clipX1() - xPos1;
      xPos1 = go.clipX1();
    }
    if (yPos1 < go.clipY1()) {
      ySrc1 += go.clipY1() - yPos1;
      yPos1 = go.clipY1();
    }
    if (xPos2 >= go.clipX2()) {
      xSrc2 -= xPos2 - go.clipX2();
      xPos2 = go.clipX2();
    }
    if (yPos2 >= go.clipY2()) {
      ySrc2 -= yPos2 - go.clipY2();
      yPos2 = go.clipY2();
    }
  }

  float fdx1 = xPos1, fdy1 = yPos1, fdx2 = xPos2, fdy2 = yPos2;
  if(!filterCoords(xSrc1, ySrc1, xSrc2, ySrc2,
                   fdx1, fdy1, fdx2, fdy2))
    return;

  // Convert the pixel coordinates into [0,1) texture coordinates
  float thisx1 = float(xSrc1) / m_textureWidth;
  float thisy1 = float(ySrc1) / m_textureHeight;
  float thisx2 = float(xSrc2) / m_textureWidth;
  float thisy2 = float(ySrc2) / m_textureHeight;

  glBindTexture(GL_TEXTURE_2D, m_textureID);

  // Make this so that when we have composite 1, we're doing a pure
  // additive blend, (ignoring the alpha channel?)
  switch(go.compositeMode())
  {
  case 0:
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    break;
  case 1:
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    break;
  case 2:
    cerr << "Compisite mode 2 (unwritten!)" << endl;
    break;
  default:
  {
    ostringstream oss;
    oss << "Invalid compositeMode in render: " << go.compositeMode();
    throw SystemError(oss.str());
  }
  }

  // @todo Double and triple check to see that the light it being
  // added in against the gold standard when I get home.
  int alpha = go.alpha();
  if(overrides.hasAlphaOverride)
  {
    alpha = overrides.alpha;
  }

  glPushMatrix();
  {
    // Move the "origin" to the correct position.
    glTranslatef(go.xOrigin(), go.yOrigin(), 0);

    // Rotate here?
    glRotatef(float(go.rotation()) / 10, 0, 0, 1);

    glBegin(GL_QUADS);
    {
      glColorRGBA(RGBAColour(go.tint(), alpha));
      glTexCoord2f(thisx1, thisy1);
      glVertex2f(fdx1, fdy1);
      glTexCoord2f(thisx2, thisy1);
      glVertex2f(fdx2, fdy1);
      glTexCoord2f(thisx2, thisy2);
      glVertex2f(fdx2, fdy2);
      glTexCoord2f(thisx1, thisy2);
      glVertex2f(fdx1, fdy2);
    }
    glEnd();

    glBlendFunc(GL_ONE, GL_ZERO);
  }
  glPopMatrix();

  ShowGLErrors();
}

// -----------------------------------------------------------------------

void Texture::rawRenderQuad(const int srcCoords[8],
                            const int destCoords[8],
                            const int opacity[4])
{
  /// @bug FIXME!
//  if(!filterCoords(x1, y1, x2, y2, dx1, dy1, dx2, dy2))
//    return;

  // For the time being, we are dumb and assume that it's one texture
  float textureCoords[8];
  for(int i = 0; i < 8; i += 2)
  {
    textureCoords[i] = float(srcCoords[i]) / m_textureWidth;
    textureCoords[i + 1] = float(srcCoords[i + 1]) / m_textureHeight;
  }

  glBindTexture(GL_TEXTURE_2D, m_textureID);

  // Blend when we have less opacity
  if(find_if(opacity, opacity + 4, bind(std::less<int>(), _1, 255))
     != opacity + 4)
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glBegin(GL_QUADS);
  {
    for(int i = 0; i < 4; i++)
    {
      glColor4ub(255, 255, 255, opacity[i]);

      int firstIndex = i * 2;
      int secondIndex = firstIndex + 1;
      glTexCoord2f(srcCoords[firstIndex], srcCoords[secondIndex]);
      glVertex2i(destCoords[firstIndex], destCoords[secondIndex]);
    }
  }
  glEnd();
  glBlendFunc(GL_ONE, GL_ZERO);
}

// -----------------------------------------------------------------------

bool Texture::filterCoords(int& x1, int& y1, int& x2, int& y2,
                           float& dx1, float& dy1, float& dx2, float& dy2)
{
  // POINT
  using std::max;
  using std::min;

  // Input: raw image coordinates
  // Output: false if this doesn't intersect with the texture piece we hold.
  //         true otherwise, and set the local coordinates
  int w1 = x2 - x1;
  int h1 = y2 - y1;

  // First thing we do is an intersection test to see if this input
  // range intersects the virtual range this Texture object holds.
  //
  /// @bug s/>/>=/?
  if (x1 + w1 > m_xOffset && x1 < m_xOffset + m_logicalWidth &&
      y1 + h1 > m_yOffset && y1 < m_yOffset + m_logicalHeight)
  {
    // Do an intersection test in terms of the virtual coordinates
    int virX = max(x1, m_xOffset);
    int virY = max(y1, m_yOffset);
    int w = min(x1+w1, m_xOffset + m_logicalWidth) - max(x1, m_xOffset);
    int h = min(y1+h1, m_yOffset + m_logicalHeight) - max(y1, m_yOffset);

    // Adjust the destination coordinates
    float dxWidth = dx2 - dx1;
    float dyHeight = dy2 - dy1;
    float dx1Off = (virX - x1) / float(w1);
    dx1 = dx1 + (dxWidth * dx1Off);
    float dx2Off = w / float(w1);
    dx2 = dx1 + (dxWidth * dx2Off);
    float dy1Off = (virY - y1) / float(h1);
    dy1 = dy1 + (dyHeight * dy1Off);
    float dy2Off = h / float(h1);
    dy2 = dy1 + (dyHeight * dy2Off);

    // Output the source intersection in real (instead of
    // virtual) coordinates
    x1 = virX - m_xOffset;
    x2 = x1 + w;
    y1 = virY - m_yOffset;
    y2 = y1 + h;

    return true;
  }

  return false;
}
