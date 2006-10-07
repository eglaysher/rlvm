#ifndef __SDLGraphicsSystem_hpp_
#define __SDLGraphicsSystem_hpp_

#include <SDL/SDL.h>

#include <vector>

#include "GraphicsSystem.hpp"

class SDLGraphicsSystem : public GraphicsSystem
{
private:
  // 
  std::vector<SDL_Surface*> displayContexts;

public:
  SDLGraphicsSystem(); 

  virtual void allocateDC(int dc, int width, int height);
  virtual void freeDC(int dc);

  virtual void wipe(int dc, int r, int g, int b);
};

#endif
