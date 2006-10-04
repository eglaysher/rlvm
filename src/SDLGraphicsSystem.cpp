#include "SDLGraphicsSystem.hpp"

#include <sstream>

#include "defs.h"

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
  /* Let's get some video information. */
  const SDL_VideoInfo* info = SDL_GetVideoInfo( );
  
  if( !info ) {
    stringstream ss;
    ss << "Video query failed: " << SDL_GetError();
    throw Error(ss.str());
  }

  // Make this read these values from the Gameexe
  /*
   * Set our width/height to 640/480 (you would
   * of course let the user decide this in a normal
   * app). We get the bpp we will request from
   * the display. On X11, VidMode can't change
   * resolution, so this is probably being overly
   * safe. Under Win32, ChangeDisplaySettings
   * can change the bpp.
   */
  int width = 800;
  int height = 600;
  int bpp = info->vfmt->BitsPerPixel;

  /*
   * Set the video mode
   */
  if((displayContexts[0] =
      SDL_SetVideoMode( width, height, bpp, SDL_ANYFORMAT |
                        //SDL_FULLSCREEN |
                        //SDL_DOUBLEBUF |
                        SDL_HWPALETTE |
                        SDL_SWSURFACE)) == 0 )
  {
    /* 
     * This could happen for a variety of reasons,
     * including DISPLAY not being set, the specified
     * resolution not being available, etc.
     */
    stringstream ss;
    ss << "Video mode set failed: " << SDL_GetError();
    throw Error(ss.str());
  }	
}
