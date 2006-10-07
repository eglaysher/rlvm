#ifndef __GraphicsSystem_hpp__
#define __GraphicsSystem_hpp__

/** 
 * Abstract interface to a graphics system. Specialize this class for
 * each system you plan on running RLVM on. For now, there's only one
 * derived class; SDLGraphicsSystem.
 *
 * Two device contexts must be allocated during initialization; DC 0,
 * which should refer to the underlying surface for the screen, and DC
 * 1, which is simply guarenteed to be allocated, and is guarenteed to
 * not be smaller then the screen.
 */
class GraphicsSystem
{
public:
  virtual void allocateDC(int dc, int width, int height) = 0;
  virtual void freeDC(int dc) = 0;

  virtual void wipe(int dc, int r, int g, int b) = 0;

/*
  virtual void getDCPixel(int x, int y, int dc, int& r, int& g, int& b) = 0;
  virtual void wipe(int dc, int r, int g, int b) = 0;

  virtual void 
*/
};

#endif
