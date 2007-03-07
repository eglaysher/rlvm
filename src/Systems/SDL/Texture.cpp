// This file is part of RLVM, a RealLive virtual machine clone.
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

#include <boost/bind.hpp>
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>

#include <iostream>
#include <cstdio>

#include "Systems/Base/GraphicsObject.hpp"
#include "Systems/SDL/SDLGraphicsSystem.hpp"
#include "Systems/SDL/SDLSurface.hpp"
#include "Systems/SDL/Texture.hpp"
#include "Systems/SDL/SDLUtils.hpp"

#include "libReallive/defs.h"

using namespace std;
using namespace boost;
using namespace libReallive;

// -----------------------------------------------------------------------

Texture::Texture(SDL_Surface* surface)
  : m_logicalWidth(surface->w), m_logicalHeight(surface->h), m_isUpsideDown(false)
{
  glGenTextures(1, &m_textureID);
  glBindTexture(GL_TEXTURE_2D, m_textureID);
  ShowGLErrors();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  SDL_LockSurface(surface);

  Uint8 bytesPerPixel = surface->format->BytesPerPixel;
  GLint byteOrder = GL_RGBA;
  GLint byteType = GL_UNSIGNED_BYTE;

  // Determine the byte order of the surface
  SDL_PixelFormat* format = surface->format;
  if(bytesPerPixel == 4)
  {
    // If the order is RGBA...
    if(format->Rmask == 0xFF000000 && format->Amask == 0xFF)
      byteOrder = GL_RGBA;
    // OSX's crazy ARGB pixel format
    else if(format->Amask == 0xFF000000 && format->Rmask == 0xFF0000 &&
            format->Gmask == 0xFF00 && format->Bmask == 0xFF)
    {
      // This is an insane hack to get around OSX's crazy byte order
      // for alpha on PowerPC. Since there isn't a GL_ARGB type, we
      // need to specify BGRA and then tell the byte type to be
      // reversed order.
      //
      // 20070303: Whoah! Is this the internal format on all
      // platforms!?
      byteOrder = GL_BGRA;
      byteType = GL_UNSIGNED_INT_8_8_8_8_REV;
    }
    else 
    {
      ios_base::fmtflags f = cerr.flags(ios::hex | ios::uppercase);
      cerr << "Unknown mask: (" << format->Rmask << ", " << format->Gmask
           << ", " << format->Bmask << ", " << format->Amask << ")" << endl;
      cerr.flags(f);
    }
  }
  else if(bytesPerPixel == 3)
  {
    // For now, just assume RGB.
    byteOrder = GL_RGB;
    cerr << "Warning: Am I really an RGB Surface? Check Texture::Texture()!"
         << endl;
  }
  else
    throw Error("Error loading texture: bytesPerPixel != 3 or 4. Duuudee...");

  m_textureWidth = SafeSize(surface->w);
  m_textureHeight = SafeSize(surface->h);
  glTexImage2D(GL_TEXTURE_2D, 0, bytesPerPixel,
               m_textureWidth, m_textureHeight,
               0, 
               byteOrder,
               byteType, NULL);
  ShowGLErrors();

  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, surface->w, surface->h,
                  byteOrder, byteType, surface->pixels);            
  ShowGLErrors();

  SDL_UnlockSurface(surface);
}

// -----------------------------------------------------------------------

Texture::Texture(render_to_texture, int width, int height)
  : m_logicalWidth(width), m_logicalHeight(height), m_isUpsideDown(true)
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
//  cerr << "Deleteing texture with texid " << m_textureID << endl;
  ShowGLErrors();
}

// -----------------------------------------------------------------------

// This is really broken and brain dead.
void Texture::renderToScreen(int x1, int y1, int x2, int y2,
                             int dx1, int dy1, int dx2, int dy2,
                             int opacity)
{
  // For the time being, we are dumb and assume that it's one texture
  
  float thisx1 = float(x1) / m_textureWidth;
  float thisy1 = float(y1) / m_textureHeight;
  float thisx2 = float(x2) / m_textureWidth;
  float thisy2 = float(y2) / m_textureHeight;

   if(m_isUpsideDown)
   {
//     cerr << "is upside down" << endl;
     thisy1 = float(m_logicalHeight - y1) / m_textureHeight;
     thisy2 = float(m_logicalHeight - y2) / m_textureHeight;
   }

  glBindTexture(GL_TEXTURE_2D, m_textureID);

  // Blend when we have less opacity
//  if(opacity < 255)
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glBegin(GL_QUADS);
  {
    glColor4ub(255, 255, 255, opacity);
    glTexCoord2f(thisx1, thisy1);
    glVertex2i(dx1, dy1);
    glTexCoord2f(thisx2, thisy1);
    glVertex2i(dx2, dy1);
    glTexCoord2f(thisx2, thisy2);
    glVertex2i(dx2, dy2);        
    glTexCoord2f(thisx1, thisy2);
    glVertex2i(dx1, dy2);
  }
  glEnd();
  glBlendFunc(GL_ONE, GL_ZERO);
}

// -----------------------------------------------------------------------

void Texture::renderToScreen(int x1, int y1, int x2, int y2,
                             int dx1, int dy1, int dx2, int dy2,
                             const int opacity[4])
{
    // For the time being, we are dumb and assume that it's one texture
  
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
    glVertex2i(dx1, dy1);
    glColor4ub(255, 255, 255, opacity[1]);
    glTexCoord2f(thisx2, thisy1);
    glVertex2i(dx2, dy1);
    glColor4ub(255, 255, 255, opacity[2]);
    glTexCoord2f(thisx2, thisy2);
    glVertex2i(dx2, dy2);        
    glColor4ub(255, 255, 255, opacity[3]);
    glTexCoord2f(thisx1, thisy2);
    glVertex2i(dx1, dy2);
  }
  glEnd();
  glBlendFunc(GL_ONE, GL_ZERO);
}

// -----------------------------------------------------------------------

void Texture::renderToScreenAsObject(const GraphicsObject& go, SDLSurface& surface)
{
  // Figure out the source to clip out of the image
  int pattNo = go.pattNo();
  int xSrc1 = surface.getPattern(pattNo).x1;
  int ySrc1 = surface.getPattern(pattNo).y1;
  int xSrc2 = surface.getPattern(pattNo).x2;
  int ySrc2 = surface.getPattern(pattNo).y2;

  // Figure out position to display on
  int xPos1 = go.x() + go.xAdjustmentSum();
  int yPos1 = go.y() + go.yAdjustmentSum();
  int xPos2 = int(xPos1 + (xSrc2 - xSrc1) * (go.width() / 100.0f));
  int yPos2 = int(yPos1 + (ySrc2 - ySrc1) * (go.height() / 100.0f));

  // If clipping is active for this object, take that into account too.
  if (go.hasClip()) {
    // Do nothing if object falls wholly outside clip area
    if (xPos2 < go.clipX1() || xPos1 > go.clipX2() ||
        yPos2 < go.clipY1() || yPos1 > go.clipY2()) {
      return;
    }
    // Otherwise, adjust coordinates to present only the visible area.
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
      xPos2 = go.clipX2() + 1; // Yeah, more inclusive ranges. Hurray!
    }
    if (yPos2 >= go.clipY2()) {
      ySrc2 -= yPos2 - go.clipY2();
      yPos2 = go.clipY2() + 1;
    }
  }
  
  // Convert the pixel coordinates into [0,1) texture coordinates
  float thisx1 = float(xSrc1) / m_textureWidth;
  float thisy1 = float(ySrc1) / m_textureHeight;
  float thisx2 = float(xSrc2) / m_textureWidth;
  float thisy2 = float(ySrc2) / m_textureHeight;

//    cerr << "patt: " << pattNo << ", texid: " << m_textureID << ", alpha: " << go.alpha()
//         << ", src:{" << xSrc1 << "," << ySrc1 << "," << xSrc2 << "," << ySrc2 << "}"
//         << ", dst:{" << xPos1 << "," << yPos1 << "," << xPos2 << "," << yPos2 << "}"
//         << endl;

  glBindTexture(GL_TEXTURE_2D, m_textureID);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glPushMatrix();
  {
    // Move the "origin" to the correct position.
    glTranslatef(go.xOrigin(), go.yOrigin(), 0);

    // Rotate here?
    glRotatef(float(go.rotation()) / 10, 0, 0, 1);

//     cerr << "Color: " << go.tintR() << ", " << go.tintG() << ", " << go.tintB()
//          << ", " << go.alpha() << endl;

    glBegin(GL_QUADS);
    {
      glColor4ub(go.tintR(), go.tintG(), go.tintB(), go.alpha());
      glTexCoord2f(thisx1, thisy1);
      glVertex2i(xPos1, yPos1);
      glTexCoord2f(thisx2, thisy1);
      glVertex2i(xPos2, yPos1);
      glTexCoord2f(thisx2, thisy2);
      glVertex2i(xPos2, yPos2);        
      glTexCoord2f(thisx1, thisy2);
      glVertex2i(xPos1, yPos2);
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
