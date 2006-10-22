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

#include "Systems/SDL/SDLGraphicsSystem.hpp"
#include "libReallive/defs.h"
#include "file.h"

#include <iostream>
#include <sstream>
#include <cstdio>

using namespace std;
using namespace libReallive;

// -----------------------------------------------------------------------
// SDLSurface
// -----------------------------------------------------------------------

SDLSurface::SDLSurface(int width, int height, SDL_PixelFormat* pixelFormat)
{
  allocate(width, height, pixelFormat);
}

// -----------------------------------------------------------------------

SDLSurface::~SDLSurface()
{
  if(m_surface)
    SDL_FreeSurface(m_surface);
}


// -----------------------------------------------------------------------

void SDLSurface::allocate(int width, int height, SDL_PixelFormat* format)
{
  deallocate();

  SDL_Surface* tmp = 
    SDL_CreateRGBSurface(0, width, height, 
                         format->BitsPerPixel,
                         format->Rmask, format->Gmask,
                         format->Bmask, format->Amask);

  if(tmp == NULL)
  {
    stringstream ss;
    ss << "Couldn't allocate surface in SDLSurface::SDLSurface"
       << ": " << SDL_GetError();
    throw Error(ss.str());
  }  

  m_surface = SDL_DisplayFormat(tmp);

  if(m_surface == NULL)
  {
    stringstream ss;
    ss << "Couldn't allocate surface in SDLSurface::SDLSurface"
       << ": " << SDL_GetError();
    throw Error(ss.str());
  }
}

// -----------------------------------------------------------------------

void SDLSurface::deallocate()
{
  if(m_surface)
    SDL_FreeSurface(m_surface);
}

// -----------------------------------------------------------------------

Surface* SDLSurface::clone() const
{
  // Make a copy of the current surface
  SDL_Surface* tmpSurface = 
    SDL_CreateRGBSurface(m_surface->flags, m_surface->w, m_surface->h, 
                         m_surface->format->BitsPerPixel,
                         m_surface->format->Rmask, m_surface->format->Gmask,
                         m_surface->format->Bmask, m_surface->format->Amask);
  SDL_BlitSurface(m_surface, NULL, tmpSurface, NULL);

  SDL_Surface* output = SDL_DisplayFormat(tmpSurface);
  SDL_FreeSurface(tmpSurface);

  return new SDLSurface(output);
}

// -----------------------------------------------------------------------
// Private Interface
// -----------------------------------------------------------------------

void SDLGraphicsSystem::dc0writtenTo()
{
  switch(screenUpdateMode())
  {
  case SCREENUPDATEMODE_AUTOMATIC:
  case SCREENUPDATEMODE_SEMIAUTOMATIC:
  {
    // Perform a blit of DC0 to the screen, and update it.
    m_screenNeedsRefresh = true;
    break;
  }
  case SCREENUPDATEMODE_MANUAL:
  {
    // Simply mark that we are dirty
    m_screenDirty = true;
  }
  }
}

// -----------------------------------------------------------------------

void SDLGraphicsSystem::refresh() 
{
  // For now, we simply blit DC0 to the screen. This needs to be
  // improved later.
  SDL_BlitSurface(displayContexts[0], NULL, m_screen, NULL);
  
  // Update the screen
  SDL_UpdateRect(m_screen, 0, 0, 0, 0);
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
  m_width = 800;
  m_height = 600;
  int bpp = info->vfmt->BitsPerPixel;

  // Set the video mode
  if((m_screen =
      SDL_SetVideoMode( m_width, m_height, bpp, 
                        SDL_ANYFORMAT |
                        //SDL_FULLSCREEN |
                        //SDL_DOUBLEBUF |
                        SDL_HWSURFACE)) == 0 )
  {
    // This could happen for a variety of reasons,
    // including DISPLAY not being set, the specified
    // resolution not being available, etc.
    stringstream ss;
    ss << "Video mode set failed: " << SDL_GetError();
    throw Error(ss.str());
  }	

  SDL_PixelFormat* format = m_screen->format;

  // Now we allocate the first two display contexts with equal size to
  // the display
  for(int i = 0; i < 2; ++i) 
    displayContexts[i].allocate(m_width, m_height, format);
}

// -----------------------------------------------------------------------

void SDLGraphicsSystem::executeGraphicsSystem()
{
  // For now, nothing, but later, we need to put all code each cycle
  // here.
  if(m_screenNeedsRefresh)
  {
    refresh();
    m_screenNeedsRefresh = false;
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
  displayContexts[dc].allocate(width, height, m_screen->format);
}

// -----------------------------------------------------------------------

void SDLGraphicsSystem::freeDC(int dc)
{
  if(dc == 0)
    throw Error("Attempt to deallocate DC[0]");
  else if(dc == 1)
  {
    // DC[1] never gets freed; it only gets blanked
    wipe(dc, 0, 0, 0);
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

void SDLGraphicsSystem::reportSDLError(const std::string& sdlName,
                                       const std::string& functionName)
{
  stringstream ss;
  ss << "Error while calling SDL function '" << sdlName << "' in "
     << functionName << ": " << SDL_GetError();
  throw Error(ss.str());
}

// -----------------------------------------------------------------------

void SDLGraphicsSystem::wipe(int dc, int r, int g, int b)
{
  cerr << "wipe(" << dc << ", " << r << ", " << g << ", " << b << ")" 
       << endl;

  verifySurfaceExists(dc, "SDLGraphicsSystem::wipe");

  SDL_Surface* surface = displayContexts[dc];

  // Fill the entire surface with the incoming color
  Uint32 color = SDL_MapRGB(surface->format, r, g, b);

  if(SDL_FillRect(surface, NULL, color))
    reportSDLError("SDL_FillRect", "SDLGrpahicsSystem::wipe()");

  // If we are the main screen, then we want to update the screen
  if(dc == 0)
    dc0writtenTo();
}

// -----------------------------------------------------------------------

void SDLGraphicsSystem::blitSurfaceToDC(
  Surface& sourceObj, int targetDC, 
  int srcX, int srcY, int srcWidth, int srcHeight,
  int destX, int destY, int destWidth, int destHeight,
  int alpha)
{
  // Drawing an empty image is a noop
  if(alpha == 0)
    return;

  verifySurfaceExists(targetDC, "SDLGraphicsSystem::blitSurfaceToDC");
  SDL_Surface* dest = displayContexts[targetDC];
  SDL_Surface* src = dynamic_cast<SDLSurface&>(sourceObj).surface();

  SDL_Rect srcRect, destRect;
  srcRect.x = srcX;
  srcRect.y = srcY;
  srcRect.w = srcWidth;
  srcRect.h = srcHeight;

  destRect.x = destX;
  destRect.y = destY;
  destRect.w = destWidth;
  destRect.h = destHeight;

  if(alpha != 255) 
  {
    if(SDL_SetAlpha(src, SDL_SRCALPHA, alpha))
      reportSDLError("SDL_SetAlpha", "SDLGrpahicsSystem::blitSurfaceToDC()");
  }

  if(SDL_BlitSurface(src, &srcRect, dest, &destRect))
    reportSDLError("SDL_BlitSurface", "SDLGrpahicsSystem::blitSurfaceToDC()");

  if(targetDC == 0)
    dc0writtenTo();
}

// -----------------------------------------------------------------------
typedef enum { NO_MASK, ALPHA_MASK, COLOR_MASK} MaskType;

#define DefaultRmask 0xff0000
#define DefaultGmask 0xff00
#define DefaultBmask 0xff
#define DefaultAmask 0xff000000
#define DefaultBpp 32

static SDL_Surface* newSurfaceFromRGBAData(int w, int h, char* data, 
                                           MaskType with_mask)
{
  int amask = (with_mask == ALPHA_MASK) ? DefaultAmask : 0;
  SDL_Surface* tmp = SDL_CreateRGBSurfaceFrom(
    data, w, h, DefaultBpp, w*4, DefaultRmask, DefaultGmask, 
    DefaultBmask, amask);

  // This is the perfect example of why I need to come back and
  // really understand this part of the code I'm stealing. WTF is
  // this!?
  tmp->flags &= ~SDL_PREALLOC;

  SDL_Surface* s = SDL_DisplayFormat(tmp);
  SDL_FreeSurface(tmp);

  return s;
};

/** 
 * @author Jagarl
 * @source Xclannad
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
  fseek(file, 0, SEEK_END);
  int size = ftell(file);
  char* d = new char[size];
  fseek(file, 0, SEEK_SET);
  fread(d, size, 1, file);
  fclose(file);

  // For the time being, and against my better judgement, we simply
  // call the image loading methods stolen from xclannad. The
  // following code is stolen verbatim from picture.cc in xclannad.
  GRPCONV* conv = GRPCONV::AssignConverter(d, size, "???");
  if (conv == 0) { return 0;}
  char* mem = (char*)malloc(conv->Width() * conv->Height() * 4 + 1024);
  SDL_Surface* s = 0;
  if (conv->Read(mem)) {
    MaskType is_mask = conv->IsMask() ? ALPHA_MASK : NO_MASK;
    if (is_mask == ALPHA_MASK) { // alpha がすべて 0xff ならマスク無しとする
      int len = conv->Width()*conv->Height();
      unsigned int* d = (unsigned int*)mem;
      int i; for (i=0; i<len; i++) {
        if ( (*d&0xff000000) != 0xff000000) break;
        d++;
      }
      if (i == len) {
        is_mask = NO_MASK;
      }
    }
    s = newSurfaceFromRGBAData(conv->Width(), conv->Height(), mem, is_mask);
  }

  // @todo Right about here, we might want to deal with stealing the
  // type 2 information out of Jagarl's grpconv object.

  delete conv;  // delete data;
  delete d;

  return new SDLSurface(s);
}

// -----------------------------------------------------------------------

Surface& SDLGraphicsSystem::getDC(int dc)
{
  verifySurfaceExists(dc, "SDLGraphicsSystem::getDC");
  return displayContexts[dc];
}
                                
int SDLSurface::width() const { return m_surface->w; }
int SDLSurface::height() const { return m_surface->h; }

