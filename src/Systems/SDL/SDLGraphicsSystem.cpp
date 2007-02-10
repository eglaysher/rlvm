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
 * @file   SDLGraphicsSystem.cpp
 * @author Elliot Glaysher
 * @date   Fri Oct  6 13:34:08 2006
 * 
 * @brief  Exposed interface for the SDL Graphics system.
 * 
 * While there are other 
 * 
 * @todo Make the constructor take the Gameexe.ini, and read the
 * initial window size from it.
 */

#include "MachineBase/RLMachine.hpp"
#include "MachineBase/RLModule.hpp"
#include "Systems/SDL/SDLGraphicsSystem.hpp"
#include "Systems/Base/GraphicsObject.hpp"
#include "libReallive/defs.h"
#include "file.h"
#include "Utilities.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <cstdio>

#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#include <SDL/SDL_image.h>

#include <boost/bind.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/scoped_array.hpp>

using namespace boost;
using namespace std;
using namespace libReallive;

static void reportSDLError(const std::string& sdlName,
                           const std::string& functionName)
{
  stringstream ss;
  ss << "Error while calling SDL function '" << sdlName << "' in "
     << functionName << ": " << SDL_GetError();
  throw Error(ss.str());

}

void ShowGLErrors(void)
{
  GLenum error;
  const GLubyte* errStr;
  if ((error = glGetError()) != GL_NO_ERROR)
  {
    errStr = gluErrorString(error);
    fprintf(stderr, "OpenGL Error: %s\n", errStr);
    abort();
  }
}

static int SafeSize(int i) {
  static GLint maxTextureSize = 0;
  if(maxTextureSize == 0)
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
  int p;

  if (i > maxTextureSize) return maxTextureSize;

  for (p = 0; p < 24; p++)
    if (i <= (1<<p))
      return 1<<p;

  return maxTextureSize;
}

// -----------------------------------------------------------------------
// Texture
// -----------------------------------------------------------------------


class Texture
{
private:
  unsigned int m_logicalWidth;
  unsigned int m_logicalHeight;

  unsigned int m_textureWidth;
  unsigned int m_textureHeight;

  GLuint m_textureID;

public:
  Texture(SDL_Surface* surface);
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

// -----------------------------------------------------------------------

Texture::Texture(SDL_Surface* surface)
  : m_logicalWidth(surface->w), m_logicalHeight(surface->h)
{
  glGenTextures(1, &m_textureID);
  glBindTexture(GL_TEXTURE_2D, m_textureID);
  ShowGLErrors();
//  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);
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
      byteOrder = GL_BGRA;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
      byteType = GL_UNSIGNED_INT_8_8_8_8_REV;
#else
      byteType = GL_UNSIGNED_INT_8_8_8_8;
#endif
    }
    else
    {
      cerr << "Unknown mask! It's not Hakuro!" << endl;
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

  // I have no idea what I'm doing!
//    cerr << "MASK: [" << hex << surface->format->Rmask 
//         << ", " << surface->format->Gmask 
//         << ", " << surface->format->Bmask 
//         << ", " << surface->format->Amask 
//         << "]" << endl;

  m_textureWidth = SafeSize(surface->w);
  m_textureHeight = SafeSize(surface->h);
  glTexImage2D(GL_TEXTURE_2D, 0, bytesPerPixel,
               m_textureWidth, m_textureHeight,
               0, 
               byteOrder,
               byteType, NULL);
  ShowGLErrors();

  // Check the surface for the byte order of the surface:
//   cerr << "Size: {" << dec << surface->w << ", " << surface->h << "}"
//        << " First pixel: " << hex << ((int*)surface->pixels)[0] << endl;

  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, surface->w, surface->h,
                  byteOrder, byteType, surface->pixels);            
  ShowGLErrors();

  SDL_UnlockSurface(surface);
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
  int xPos2 = xPos1 + (xSrc2 - xSrc1) * (go.width() / 100.0f);
  int yPos2 = yPos1 + (ySrc2 - ySrc1) * (go.height() / 100.0f);

  // Convert the pixel coordinates into [0,1) texture coordinates
  float thisx1 = float(xSrc1) / m_textureWidth;
  float thisy1 = float(ySrc1) / m_textureHeight;
  float thisx2 = float(xSrc2) / m_textureWidth;
  float thisy2 = float(ySrc2) / m_textureHeight;

//   cerr << "patt: " << pattNo << ", texid: " << m_textureID << ", alpha: " << go.alpha()
//        << ", src:{" << xSrc1 << "," << ySrc1 << "," << xSrc2 << "," << ySrc2 << "}"
//        << ", dst:{" << xPos1 << "," << yPos1 << "," << xPos2 << "," << yPos2 << "}"
//        << endl;

  glBindTexture(GL_TEXTURE_2D, m_textureID);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glPushMatrix();
  {
    // Move the "origin" to the correct position.
    glTranslatef(go.xOrigin(), go.yOrigin(), 0);

    // Rotate here?
    glRotatef(float(go.rotation()) / 10, 0, 0, 1);

//    cerr << "Color: " << go.tintR() << ", " << go.tintG() << ", " << go.tintB()
//         << ", " << go.alpha() << endl;

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

// -----------------------------------------------------------------------
// SDLSurface
// -----------------------------------------------------------------------

SDLSurface::SDLSurface()
  : m_surface(NULL), m_textureIsValid(false), m_graphicsSystem(NULL)
{}

/// Surface that takes ownership of an externally created surface.
SDLSurface::SDLSurface(SDL_Surface* surf, 
                       const vector<SDLSurface::GrpRect>& region_table)
  : m_surface(surf), m_regionTable(region_table),
    m_textureIsValid(false), m_graphicsSystem(NULL)
{}

SDLSurface::SDLSurface(int width, int height)
  : m_textureIsValid(false), m_graphicsSystem(NULL)
{
  allocate(width, height);

  // Build a region table with one entry the size of the surface (This
  // should never need to be used with objects created with this
  // constructor, but let's make sure everything is initialized since
  // it'll happen somehow.)
  SDLSurface::GrpRect rect;
  rect.x1 = 0;
  rect.y1 = 0;
  rect.x2 = width;
  rect.y2 = height;
  m_regionTable.push_back(rect);
}

// -----------------------------------------------------------------------

SDLSurface::~SDLSurface()
{
  deallocate();
}

void SDLSurface::dump()
{
  static int count = 0;
  stringstream ss;
  ss << "dump_" << count << ".bmp";
  count++;
  SDL_SaveBMP(m_surface, ss.str().c_str());
}

// -----------------------------------------------------------------------

void SDLSurface::allocate(int width, int height)
{
  deallocate();

  // Create an empty surface
  Uint32 rmask, gmask, bmask, amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif

  SDL_Surface* tmp = 
    SDL_CreateRGBSurface(SDL_HWSURFACE | SDL_SRCALPHA, width, height, 
                         32, 
                         rmask, gmask, bmask, amask);

  if(tmp == NULL)
  {
    stringstream ss;
    ss << "Couldn't allocate surface in SDLSurface::SDLSurface"
       << ": " << SDL_GetError();
    throw Error(ss.str());
  }  

  m_surface = tmp;

  fill(0, 0, 0, 255);
}

// -----------------------------------------------------------------------

void SDLSurface::allocate(int width, int height, SDLGraphicsSystem* sys)
{
  m_graphicsSystem = sys;
  allocate(width, height);
}

// -----------------------------------------------------------------------

void SDLSurface::deallocate()
{
  m_texture.reset();
  if(m_surface)
    SDL_FreeSurface(m_surface);
}

// -----------------------------------------------------------------------

/**
 * @todo This function doesn't ignore alpha blending when useSrcAlpha
 *       is false; thus, grpOpen and grpMaskOpen are really grpMaskOpen.
 */
void SDLSurface::blitToSurface(Surface& destSurface,
                               int srcX, int srcY, int srcWidth, int srcHeight,
                               int destX, int destY, int destWidth, int destHeight,
                               int alpha, bool useSrcAlpha)
{
  SDLSurface& dest = dynamic_cast<SDLSurface&>(destSurface);

  SDL_Rect srcRect, destRect;
  srcRect.x = srcX;
  srcRect.y = srcY;
  srcRect.w = srcWidth;
  srcRect.h = srcHeight;

  destRect.x = destX;
  destRect.y = destY;
  destRect.w = destWidth;
  destRect.h = destHeight;

  if(useSrcAlpha) 
  {
    if(SDL_SetAlpha(m_surface, SDL_SRCALPHA, alpha))
      reportSDLError("SDL_SetAlpha", "SDLGrpahicsSystem::blitSurfaceToDC()");
  }
  else
  {
    if(SDL_SetAlpha(m_surface, 0, 0))
      reportSDLError("SDL_SetAlpha", "SDLGrpahicsSystem::blitSurfaceToDC()");
  }

  if(SDL_BlitSurface(m_surface, &srcRect, dest.surface(), &destRect))
    reportSDLError("SDL_BlitSurface", "SDLGrpahicsSystem::blitSurfaceToDC()");

  dest.markWrittenTo();
}

// -----------------------------------------------------------------------

void SDLSurface::uploadTextureIfNeeded()
{
  if(!m_textureIsValid)
  {
//    cout << "Uploading texture!" << endl;
    m_texture.reset(new Texture(m_surface));
    m_textureIsValid = true;
  }
}

// -----------------------------------------------------------------------

Texture& SDLSurface::texture()
{
  uploadTextureIfNeeded();
  return *m_texture;
}

// -----------------------------------------------------------------------

void SDLSurface::renderToScreen(
                     int srcX1, int srcY1, int srcX2, int srcY2,
                     int destX1, int destY1, int destX2, int destY2,
                     int alpha)
{
  uploadTextureIfNeeded();

  m_texture->renderToScreen(srcX1, srcY1, srcX2, srcY2,
                            destX1, destY1, destX2, destY2,
                            alpha);
}

// -----------------------------------------------------------------------

void SDLSurface::renderToScreen(
                     int srcX1, int srcY1, int srcX2, int srcY2,
                     int destX1, int destY1, int destX2, int destY2,
                     const int opacity[4])
{
  uploadTextureIfNeeded();

  m_texture->renderToScreen(srcX1, srcY1, srcX2, srcY2,
                            destX1, destY1, destX2, destY2,
                            opacity);
}

// -----------------------------------------------------------------------

void SDLSurface::renderToScreenAsObject(const GraphicsObject& rp)
{
  uploadTextureIfNeeded();
  m_texture->renderToScreenAsObject(rp, *this);
}

// -----------------------------------------------------------------------

void SDLSurface::rawRenderQuad(const int srcCoords[8], 
                               const int destCoords[8],
                               const int opacity[4])
{
  uploadTextureIfNeeded();

  m_texture->rawRenderQuad(srcCoords, destCoords, opacity);
}

// -----------------------------------------------------------------------

void SDLSurface::fill(int r, int g, int b, int alpha)
{
  // Fill the entire surface with the incoming color
  Uint32 color = SDL_MapRGBA(m_surface->format, r, g, b, alpha);

  if(SDL_FillRect(m_surface, NULL, color))
    reportSDLError("SDL_FillRect", "SDLGrpahicsSystem::wipe()");

  // If we are the main screen, then we want to update the screen
  markWrittenTo();
}

// -----------------------------------------------------------------------

void SDLSurface::fill(int r, int g, int b, int alpha, int x, int y, 
                      int width, int height)
{
  // Fill the entire surface with the incoming color
  Uint32 color = SDL_MapRGBA(m_surface->format, r, g, b, alpha);

  SDL_Rect rect;
  rect.x = x;
  rect.y = y;
  rect.w = width;
  rect.h = height;

  if(SDL_FillRect(m_surface, &rect, color))
    reportSDLError("SDL_FillRect", "SDLGrpahicsSystem::wipe()");

  // If we are the main screen, then we want to update the screen
  markWrittenTo();
}

// -----------------------------------------------------------------------

void SDLSurface::markWrittenTo()
{
  // If we are marked as dc0, alert the SDLGraphicsSystem.
  if(m_graphicsSystem) {
    m_graphicsSystem->markScreenAsDirty();
  }

  // Mark that the texture needs reuploading
  m_textureIsValid = false;
}

// -----------------------------------------------------------------------

const SDLSurface::GrpRect& SDLSurface::getPattern(int pattNo) const
{
  return m_regionTable.at(pattNo);
}

// -----------------------------------------------------------------------

Surface* SDLSurface::clone() const
{
  SDL_Surface* tmpSurface = 
    SDL_CreateRGBSurface(m_surface->flags, m_surface->w, m_surface->h, 
                         m_surface->format->BitsPerPixel,
                         m_surface->format->Rmask, m_surface->format->Gmask,
                         m_surface->format->Bmask, m_surface->format->Amask);

  // Disable alpha blending because we're copying onto a blank (and
  // blank alpha!) surface
  if(SDL_SetAlpha(m_surface, 0, 0))
      reportSDLError("SDL_SetAlpha", "SDLGrpahicsSystem::blitSurfaceToDC()");

  if(SDL_BlitSurface(m_surface, NULL, tmpSurface, NULL))
    reportSDLError("SDL_BlitSurface", "SDLSurface::clone()");

  return new SDLSurface(tmpSurface, m_regionTable);
}

// -----------------------------------------------------------------------
// Private Interface
// -----------------------------------------------------------------------

void SDLGraphicsSystem::markScreenAsDirty()
{
  switch(screenUpdateMode())
  {
  case SCREENUPDATEMODE_AUTOMATIC:
  case SCREENUPDATEMODE_SEMIAUTOMATIC:
  {
    // Perform a blit of DC0 to the screen, and update it.
    m_screenDirty = true;
    break;
  }
  case SCREENUPDATEMODE_MANUAL:
  {
    // Don't really do anything.
    break;
  }
  default:
  {
    ostringstream oss;
    oss << "Invalid screen update mode value: " << screenUpdateMode();
    throw Error(oss.str());
  }
  }
}

// -----------------------------------------------------------------------

void SDLGraphicsSystem::markScreenForRefresh()
{
  m_screenNeedsRefresh = true;
}

// -----------------------------------------------------------------------

void SDLGraphicsSystem::beginFrame()
{
  glClearColor(0,0,0, 0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  ShowGLErrors();

  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glDisable(GL_LIGHTING);
  ShowGLErrors();
 	
  glMatrixMode(GL_PROJECTION);
//  glPushMatrix();
  glLoadIdentity();
  glOrtho(0.0, (GLdouble)m_width, (GLdouble)m_height, 0.0, 0.0, 1.0);
  ShowGLErrors();
 	
  glMatrixMode(GL_MODELVIEW);
//  glPushMatrix();
  glLoadIdentity();
  ShowGLErrors();
}

// -----------------------------------------------------------------------

void SDLGraphicsSystem::refresh(RLMachine& machine) 
{
  beginFrame();

  // Display DC0
  displayContexts[0].renderToScreen(0, 0, m_width, m_height, 
                                    0, 0, m_width, m_height, 255);

  // Render all visible foreground objects
  for_each(foregroundObjects, foregroundObjects + 256,
           bind(&GraphicsObject::render, _1, ref(machine)));

  endFrame();
}

// -----------------------------------------------------------------------

void SDLGraphicsSystem::endFrame()
{
  glFlush();
  
  // Swap the buffers
  SDL_GL_SwapBuffers();
  ShowGLErrors();
}

// -----------------------------------------------------------------------
// Public Interface
// -----------------------------------------------------------------------

/** 
 *
 * @pre SDL is initialized.
 */
SDLGraphicsSystem::SDLGraphicsSystem()
  : m_screenDirty(false), m_screenNeedsRefresh(false)
{
  for(int i = 0; i < 16; ++i)
    displayContexts.push_back(new SDLSurface);

  // Let's get some video information.
  const SDL_VideoInfo* info = SDL_GetVideoInfo( );
  
  if( !info ) {
    stringstream ss;
    ss << "Video query failed: " << SDL_GetError();
    throw Error(ss.str());
  }

  // Make this read these values from the Gameexe
  //
  // Set our width/height to 640/480 (you would
  // of course let the user decide this in a normal
  // app). We get the bpp we will request from
  // the display. On X11, VidMode can't change
  // resolution, so this is probably being overly
  // safe. Under Win32, ChangeDisplaySettings
  // can change the bpp.
  m_width = 640;
  m_height = 480;
  int bpp = info->vfmt->BitsPerPixel;

  /* the flags to pass to SDL_SetVideoMode */
  int videoFlags;
  videoFlags  = SDL_OPENGL;          /* Enable OpenGL in SDL */
  videoFlags |= SDL_GL_DOUBLEBUFFER; /* Enable double buffering */
  videoFlags |= SDL_HWPALETTE;       /* Store the palette in hardware */
  videoFlags |= SDL_RESIZABLE;       /* Enable window resizing */
  
  /* This checks to see if surfaces can be stored in memory */
  if ( info->hw_available )
	videoFlags |= SDL_HWSURFACE;
  else
	videoFlags |= SDL_SWSURFACE;

  /* This checks if hardware blits can be done */
  if ( info->blit_hw )
	videoFlags |= SDL_HWACCEL;

  /* Sets up OpenGL double buffering */
  SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
  SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
  SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
  SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

  // Set the video mode
  if((m_screen =
      SDL_SetVideoMode( m_width, m_height, bpp, videoFlags)) == 0 )
  {
    // This could happen for a variety of reasons,
    // including DISPLAY not being set, the specified
    // resolution not being available, etc.
    stringstream ss;
    ss << "Video mode set failed: " << SDL_GetError();
    throw Error(ss.str());
  }	

  glEnable(GL_TEXTURE_2D);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);


    /* Enable Texture Mapping ( NEW ) */
    glEnable( GL_TEXTURE_2D );

    /* Enable smooth shading */
    glShadeModel( GL_SMOOTH );

    /* Set the background black */
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );

    /* Depth buffer setup */
    glClearDepth( 1.0f );

    /* Enables Depth Testing */
    glEnable( GL_DEPTH_TEST );

    glEnable( GL_BLEND);

    /* The Type Of Depth Test To Do */
    glDepthFunc( GL_LEQUAL );

    /* Really Nice Perspective Calculations */
    glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );

    /* Setup The Ambient Light */
//    glLightfv( GL_LIGHT1, GL_AMBIENT, LightAmbient );

    /* Setup The Diffuse Light */
//    glLightfv( GL_LIGHT1, GL_DIFFUSE, LightDiffuse );

    /* Position The Light */
//    glLightfv( GL_LIGHT1, GL_POSITION, LightPosition );

    /* Enable Light One */
//    glEnable( GL_LIGHT1 );

    /* Full Brightness, 50% Alpha ( NEW ) */
    glColor4f( 1.0f, 1.0f, 1.0f, 0.5f);

  // Now we allocate the first two display contexts with equal size to
  // the display
  displayContexts[0].allocate(m_width, m_height, this);
  displayContexts[1].allocate(m_width, m_height);
}

// -----------------------------------------------------------------------

void SDLGraphicsSystem::executeGraphicsSystem(RLMachine& machine)
{
//  cerr << "executeGraphicsSystm()" << endl;
  // For now, nothing, but later, we need to put all code each cycle
  // here.
  if(m_screenNeedsRefresh || 
     (screenUpdateMode() != SCREENUPDATEMODE_MANUAL && m_screenDirty))
  {
    refresh(machine);
    m_screenNeedsRefresh = false;
    m_screenDirty = false;
  }
    
  // For example, we should probably do something when the screen is
  // dirty.
}

// -----------------------------------------------------------------------

int SDLGraphicsSystem::screenWidth() const
{
  return m_width;
}

// -----------------------------------------------------------------------

int SDLGraphicsSystem::screenHeight() const 
{
  return m_height;
}

// -----------------------------------------------------------------------

void SDLGraphicsSystem::allocateDC(int dc, int width, int height)
{
  if(dc >= displayContexts.size())
    throw Error("Invalid DC number in SDLGrpahicsSystem::allocateDC");

  // We can't reallocate the screen!
  if(dc == 0)
    throw Error("Attempting to reallocate DC 0!");

  // DC 1 is a special case and must always be at least the size of
  // the screen.
  if(dc == 1)
  {
    SDL_Surface* dc0 = displayContexts[0];
    if(width < dc0->w)
      width = dc0->w;
    if(height < dc0->h)
      height = dc0->h;
  }

  // Allocate a new obj.
  displayContexts[dc].allocate(width, height);
}

// -----------------------------------------------------------------------

void SDLGraphicsSystem::freeDC(int dc)
{
  if(dc == 0)
    throw Error("Attempt to deallocate DC[0]");
  else if(dc == 1)
  {
    // DC[1] never gets freed; it only gets blanked
    getDC(1).fill(0, 0, 0, 255);
  }
  else
    displayContexts[dc].deallocate();
}

// -----------------------------------------------------------------------

void SDLGraphicsSystem::verifySurfaceExists(int dc, const std::string& caller)
{
  if(dc >= displayContexts.size())
  {
    stringstream ss;
    ss << "Invalid DC number (" << dc << ") in " << caller;
    throw Error(ss.str());
  }

  if(displayContexts[dc] == NULL)
  {
    stringstream ss;
    ss << "Parameter DC[" << dc << "] not allocated in " << caller;
    throw Error(ss.str());
  }
}

// -----------------------------------------------------------------------

void SDLGraphicsSystem::verifyDCAllocation(int dc, const std::string& caller)
{
  if(displayContexts[dc] == NULL)
  {
    stringstream ss;
    ss << "Couldn't allocate DC[" << dc << "] in " << caller 
       << ": " << SDL_GetError();
    throw Error(ss.str());
  }
}

// -----------------------------------------------------------------------

GraphicsObject& SDLGraphicsSystem::getObject(int layer, int objNumber)
{
  if(layer < 0 || layer > 1)
    throw Error("Invalid layer number");

  if(objNumber < 0 || objNumber > 512)
    throw Error("Out of rnage object number");

  return foregroundObjects[objNumber];
}

// -----------------------------------------------------------------------

void SDLGraphicsSystem::setObject(int layer, int objNumber, GraphicsObject& obj)
{
  if(layer < 0 || layer > 1)
    throw Error("Invalid layer number");

  if(objNumber < 0 || objNumber > 512)
    throw Error("Out of rnage object number");

  foregroundObjects[objNumber] = obj;
}

// -----------------------------------------------------------------------

typedef enum { NO_MASK, ALPHA_MASK, COLOR_MASK} MaskType;

// Note to self: These describe the byte order IN THE RAW G00 DATA!
// These should NOT be switched to native byte order.
#define DefaultRmask 0xff0000
#define DefaultGmask 0xff00
#define DefaultBmask 0xff
#define DefaultAmask 0xff000000
#define DefaultBpp 32

static SDL_Surface* newSurfaceFromRGBAData(int w, int h, char* data, 
                                           MaskType with_mask)
{
//  int amask = (with_mask == ALPHA_MASK) ? DefaultAmask : 0;
  int amask = DefaultAmask;
  SDL_Surface* tmp = SDL_CreateRGBSurfaceFrom(
    data, w, h, DefaultBpp, w*4, DefaultRmask, DefaultGmask, 
    DefaultBmask, amask);

  // This is the perfect example of why I need to come back and
  // really understand this part of the code I'm stealing. WTF is
  // this!?
  tmp->flags &= ~SDL_PREALLOC;

  // Convert to the proper type of alpha channel
//  SDL_Surface* s;
//  if(with_mask == ALPHA_MASK)
//  {
//    s = SDL_DisplayFormatAlpha(tmp);
//    SDL_FreeSurface(tmp);
//   }
//   else
//   {
//     // Check to see:
//     // * bpp ? Might be an issue
//     // * What g00 type are we trying to load in?
//     // * 

//     s = tmp;
// //    s = SDL_DisplayFormat(tmp);
// //    if(SDL_SetAlpha(s, 0, 255))
// //      reportSDLError("SDL_SetAlpha", "newSurfaceFromRGBAData");
//   }

  return tmp;
};

// -----------------------------------------------------------------------

/** 
 * Helper function for loadSurfaceFromFile; invoked in a stl loop.
 */
SDLSurface::GrpRect xclannadRegionToGrpRect(const GRPCONV::REGION& region)
{
  SDLSurface::GrpRect rect;
  rect.x1 = region.x1;
  rect.y1 = region.y1;
  rect.x2 = region.x2 + 1;
  rect.y2 = region.y2 + 1;
  return rect;
}

/** 
 * @author Jagarl
 *
 * Loads a file from disk into a Surface object. The file loaded
 * should be a type 0 or type 1 g00 bitmap. We don't handle PDT files
 * (yet).
 * 
 * @param filename File to load (a full filename, not the basename)
 * @return A Surface object with the data from this file
 *
 * @todo Considering that I really don't understand this whole section
 * of the code, I'll probably *NEED* to come back and rewrite this,
 * simply because I'll find out that this doesn't do what I think it does.
 *
 * @warning This function probably isn't basic exception safe.
 */
Surface* SDLGraphicsSystem::loadSurfaceFromFile(const std::string& filename)
{
  // Glue code to allow my stuff to work with Jagarl's loader
  FILE* file = fopen(filename.c_str(), "rb");
  if(!file)
  {
    ostringstream oss;
    oss << "Could not open file: " << filename;
    throw Error(oss.str());
  }

  fseek(file, 0, SEEK_END);
  int size = ftell(file);
  scoped_array<char> d(new char[size]);
  fseek(file, 0, SEEK_SET);
  fread(d.get(), size, 1, file);
  fclose(file);

  // For the time being, and against my better judgement, we simply
  // call the image loading methods stolen from xclannad. The
  // following code is stolen verbatim from picture.cc in xclannad.
  scoped_ptr<GRPCONV> conv(GRPCONV::AssignConverter(d.get(), size, "???"));
  if (conv == 0) { return 0;}
  char* mem = (char*)malloc(conv->Width() * conv->Height() * 4 + 1024);
  SDL_Surface* s = 0;
  if (conv->Read(mem)) {
    MaskType is_mask = conv->IsMask() ? ALPHA_MASK : NO_MASK;
    if (is_mask == ALPHA_MASK) { // alpha がすべて 0xff ならマスク無しとする
//      cerr << "Loading alpha mask..." << endl;
      int len = conv->Width()*conv->Height();
      unsigned int* d = (unsigned int*)mem;
      int i; for (i=0; i<len; i++) {
        if ( (*d&0xff000000) != 0xff000000) break;
        d++;
      }
      if (i == len) {
//        cerr << "No mask found!" << endl;
        is_mask = NO_MASK;
      }
    }

    s = newSurfaceFromRGBAData(conv->Width(), conv->Height(), mem, is_mask);
  }

//  cerr << "Converter table size: " << conv->region_table.size() << endl;

  // Grab the Type-2 information out of the converter or create one
  // default region if none exist
  vector<SDLSurface::GrpRect> region_table;
  if(conv->region_table.size())
  {
    transform(conv->region_table.begin(), conv->region_table.end(),
              back_inserter(region_table),
              xclannadRegionToGrpRect);
  }
  else
  {
    SDLSurface::GrpRect rect;
    rect.x1 = 0;
    rect.y1 = 0;
    rect.x2 = conv->Width();
    rect.y2 = conv->Height();
    region_table.push_back(rect);
  }

  return new SDLSurface(s, region_table);
}

// -----------------------------------------------------------------------

Surface& SDLGraphicsSystem::getDC(int dc)
{
  verifySurfaceExists(dc, "SDLGraphicsSystem::getDC");
  return displayContexts[dc];
}
                                
int SDLSurface::width() const { return m_surface->w; }
int SDLSurface::height() const { return m_surface->h; }

// -----------------------------------------------------------------------

class SDLGraphicsObjectOfFile : public GraphicsObjectData
{
private:
  scoped_ptr<SDLSurface> surface;
  // A texture is fine too?

public:
  SDLGraphicsObjectOfFile(SDLGraphicsSystem& graphics, 
                          const std::string& filename)
    : surface(static_cast<SDLSurface*>(graphics.loadSurfaceFromFile(filename)))
  {  }

  SDLGraphicsObjectOfFile(SDLSurface* inSurface)
    : surface(inSurface)
  {  }

  void render(RLMachine& machine, const GraphicsObject& rp)
  {
//    cerr << "Rendering object!" << endl;
//    surface->dump();
    surface->renderToScreenAsObject(rp);
  }

  GraphicsObjectData* clone() const 
  {
    return new SDLGraphicsObjectOfFile(static_cast<SDLSurface*>(surface->clone()));
  }
};

// -----------------------------------------------------------------------

GraphicsObjectData* SDLGraphicsSystem::buildObjOfFile(const std::string& filename)
{
  return new SDLGraphicsObjectOfFile(*this, findFile(filename));
}
