#ifndef __NullGraphicsSystem_hpp__
#define __NullGraphicsSystem_hpp__

#include "Systems/Base/GraphicsSystem.hpp"

class NullGraphicsSystem : public GraphicsSystem
{
public:
  virtual void allocateDC(int dc, int width, int height) { }
  virtual void freeDC(int dc) { }

  virtual void wipe(int dc, int r, int g, int b) { }
};

#endif
