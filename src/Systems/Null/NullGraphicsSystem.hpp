#ifndef __NullGraphicsSystem_hpp__
#define __NullGraphicsSystem_hpp__

#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/Surface.hpp"

class NullSurface : public Surface
{
  virtual int width() const { return 0; }
  virtual int height() const { return 0; }

  virtual void fill(int r, int g, int b, int alpha) { }
  virtual void fill(int r, int g, int b, int alpha, int x, int y, 
                    int width, int height) { }

  virtual Surface* clone() const { return 0; }
};

class NullGraphicsSystem : public GraphicsSystem
{
public:

  virtual void refresh(RLMachine&) { }

  virtual void executeGraphicsSystem(RLMachine&) { };

  virtual int screenWidth() const { };
  virtual int screenHeight() const { };

  virtual void allocateDC(int dc, int width, int height) { }
  virtual void freeDC(int dc) { }

  virtual void promoteObjects() { }

  virtual GraphicsObjectData* buildObjOfFile(const std::string& filename)
  { return NULL; }

  virtual GraphicsObject& getObject(int layer, int objNumber) 
  { static GraphicsObject x; return x;}

  // Make a null Surface object?
  virtual Surface* loadSurfaceFromFile(const std::string& filename) { return NULL; }
  virtual boost::shared_ptr<Surface> getDC(int dc)
  { return boost::shared_ptr<Surface>(); }
  virtual void blitSurfaceToDC(Surface& sourceObj, int targetDC, 
                               int srcX, int srcY, int srcWidth, int srcHeight,
                               int destX, int destY, int destWidth, int destHeight,
                               int alpha = 255) { }
};

#endif
