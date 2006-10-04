#ifndef __SDLGraphicsSystem_hpp_
#define __SDLGraphicsSystem_hpp_

#include <SDL/SDL.h>

#include <vector>

class SDLGraphicsSystem
{
private:
  // 
  std::vector<SDL_Surface*> displayContexts;

public:
  SDLGraphicsSystem(); 
};

#endif
