#ifndef __NullGraphicsSystem_hpp__
#define __NullGraphicsSystem_hpp__

#include "Systems/Base/GraphicsSystem.hpp"

class NullSurface : public Surface
{
  virtual int width() const { return 0; }
  virtual int height() const { return 0; }
};

class NullGraphicsSystem : public GraphicsSystem
{
public:

  virtual void refresh() { }

  virtual void executeGraphicsSystem() { };

  virtual int screenWidth() const { };
  virtual int screenHeight() const { };

  virtual void allocateDC(int dc, int width, int height) { }
  virtual void freeDC(int dc) { }

  virtual void wipe(int dc, int r, int g, int b) { }

  // Make a null Surface object?
  virtual Surface* loadSurfaceFromFile(const std::string& filename) { return NULL; }
  virtual Surface& getDC(int dc) { static NullSurface s; return s; };
  virtual void blitSurfaceToDC(Surface& sourceObj, int targetDC, 
                               int srcX, int srcY, int srcWidth, int srcHeight,
                               int destX, int destY, int destWidth, int destHeight) { }
};

#endif
