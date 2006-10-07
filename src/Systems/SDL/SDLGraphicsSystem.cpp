/**
 * @file   SDLGraphicsSystem.cpp
 * @author Elliot Glaysher
 * @date   Fri Oct  6 13:34:08 2006
 * 
 * @brief  
 * 
 * 
 * @todo Make the constructor take the Gameexe.ini, and read the
 * initial window size from it.
 *
 *
 */

#include "Systems/SDL/SDLGraphicsSystem.hpp"
#include "libReallive/defs.h"

#include <sstream>

using namespace std;
using namespace libReallive;

/** 
 * 
 *
 * @pre SDL is initialized.
 */
SDLGraphicsSystem::SDLGraphicsSystem()
  // 0 is the screen, and then there are fifteen offscreen contexts
  : displayContexts(16)
{
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
  int width = 800;
  int height = 600;
  int bpp = info->vfmt->BitsPerPixel;

  // Set the video mode
  if((displayContexts[0] =
      SDL_SetVideoMode( width, height, bpp, SDL_ANYFORMAT |
                        //SDL_FULLSCREEN |
                        //SDL_DOUBLEBUF |
                        SDL_HWPALETTE |
                        SDL_SWSURFACE)) == 0 )
  {
    // This could happen for a variety of reasons,
    // including DISPLAY not being set, the specified
    // resolution not being available, etc.
    stringstream ss;
    ss << "Video mode set failed: " << SDL_GetError();
    throw Error(ss.str());
  }	
  else 
  {
    // We successfully created the main display. Now we allocate
    // displayContexts[1] to the same size as the main screen
    SDL_PixelFormat* format = displayContexts[0]->format;
    displayContexts[1] = SDL_CreateRGBSurface(0, width, height, 
                                              format->BitsPerPixel,
                                              format->Rmask, format->Gmask,
                                              format->Bmask, format->Amask);
    if(displayContexts[1] == NULL) 
    {
      stringstream ss;
      ss << "Couldn't create DC[1]: " << SDL_GetError();
      throw Error(ss.str());
    }
  }
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

  // Free the current display context if it exists.
  if(displayContexts[dc] != NULL)
  {
    SDL_FreeSurface(displayContexts[dc]);
    displayContexts[dc] = NULL;
  }

  // Allocate the new surface based off of the pixel format
  // information in the display
  SDL_PixelFormat* format = displayContexts[0]->format;
  SDL_Surface* newSurface = SDL_CreateRGBSurface(
    0, width, height, format->BitsPerPixel,
    format->Rmask, format->Gmask,
    format->Bmask, format->Amask);

  if(newSurface == NULL)
  {
    stringstream ss;
    ss << "Couldn't create DC[" << dc << "]: " << SDL_GetError();
    throw Error(ss.str());
  }

  // Assign
  displayContexts[dc] = newSurface;
}

// -----------------------------------------------------------------------

void SDLGraphicsSystem::freeDC(int dc)
{
  if(dc == 0)
    throw Error("Attempt to deallocate DC[0] (the screen)");
  else if(dc == 1)
  {
    // DC[1] never gets freed; it only gets blanked
    wipe(dc, 0, 0, 0);
  }
  else
  {
    SDL_FreeSurface(displayContexts[dc]);
    displayContexts[dc] = 0;
  }
}

// -----------------------------------------------------------------------

void SDLGraphicsSystem::wipe(int dc, int r, int g, int b)
{
  if(dc >= displayContexts.size())
    throw Error("Invalid DC number in SDLGrpahicsSystem::wipe");

  SDL_Surface* surface = displayContexts[dc];
  if(surface == NULL)
    throw Error("Device Context not allocated in SDLGraphicsSystem::wipe");

  // Fill the entire surface with the incoming color
  Uint32 color = SDL_MapRGB(surface->format, r, g, b);
  SDL_FillRect(surface, NULL, color);

  // If we are the main screen, then we want to update the screen
  if(dc == 0)
    SDL_UpdateRect(surface, 0, 0, 0, 0);
}

// -----------------------------------------------------------------------

