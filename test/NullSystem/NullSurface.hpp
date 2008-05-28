// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//

#ifndef __NullSurface_hpp__
#define __NullSurface_hpp__

#include "Systems/Base/Surface.hpp"
#include "NullSystem/MockLog.hpp"

#include <string>
#include <vector>

// -----------------------------------------------------------------------

class NullSurface : public Surface
{
public:
  NullSurface(const std::string& surface_name);
  NullSurface(const std::string& surface_name, int width, int height);

  void allocate(int width, int height);
  void deallocate();

  virtual int width() const;
  virtual int height() const;

  virtual void blitToSurface(Surface& surface, 
                             int srcX, int srcY, int srcWidth, int srcHeight,
                             int destX, int destY, int destWidth, int destHeight,
                             int alpha = 255, bool useSrcAlpha = true);

  virtual void renderToScreen(
                     int srcX, int srcY, int srcWidth, int srcHeight,
                     int destX, int destY, int destWidth, int destHeight,
                     int alpha = 255);

  virtual void renderToScreenAsColorMask(
                     int srcX1, int srcY1, int srcX2, int srcY2,
                     int destX1, int destY1, int destX2, int destY2,
                     int r, int g, int b, int alpha, int filter);

  virtual void renderToScreen(
    int srcX1, int srcY1, int srcX2, int srcY2,
    int destX1, int destY1, int destX2, int destY2,
    const int opacity[4]);

  virtual void renderToScreenAsObject(const GraphicsObject& rp);
  virtual void renderToScreenAsObject(const GraphicsObject& rp, 
                                      const GraphicsObjectOverride& override);

  virtual int numPatterns() const;
  virtual const GrpRect& getPattern(int pattNo) const;

  virtual void rawRenderQuad(const int srcCoords[8], 
                             const int destCoords[8],
                             const int opacity[4]);

  virtual void fill(int r, int g, int b, int alpha);
  virtual void fill(int r, int g, int b, int alpha, int x, int y, 
                    int width, int height);

  virtual void getDCPixel(int x, int y, int& r, int& g, int& b);

  virtual boost::shared_ptr<Surface> clipAsColorMask(
    int x, int y, int width, int height, 
    int r, int g, int b);

  virtual Surface* clone() const;

private:
  /// Unique name of this surface.
  std::string surface_name_;

  /// Whether we are allocated.
  bool allocated_;

  /// Supposed size of this surface.
  int width_, height_;

  /// The region table
  std::vector<GrpRect> m_regionTable;

  mutable MockLog surface_log_;
};


#endif
