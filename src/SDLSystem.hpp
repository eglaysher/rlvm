#ifndef __SDLSystem_hpp__
#define __SDLSystem_hpp__

#include <boost/scoped_ptr.hpp>

#include "System.hpp"

class SDLGraphicsSystem;

class SDLSystem : public System
{
private:
  boost::scoped_ptr<SDLGraphicsSystem> graphicsSystem;

public:
  SDLSystem();

};

#endif
