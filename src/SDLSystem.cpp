
#include <iostream>
#include <sstream>

#include "SDLSystem.hpp"
#include "SDLGraphicsSystem.hpp"

#include <SDL/SDL.h>

#include "defs.h"

using namespace std;
using namespace libReallive;

SDLSystem::SDLSystem()
{
  // First, initialize SDL's video subsystem.
  if( SDL_Init( SDL_INIT_VIDEO) < 0 ) {
    stringstream ss;
    ss << "Video initialization failed: " << SDL_GetError();
    throw Error(ss.str());
  } 
  else
    cerr << "SDL initialized!" << endl;

  graphicsSystem.reset(new SDLGraphicsSystem);
}
