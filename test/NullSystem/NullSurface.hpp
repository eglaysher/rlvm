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
  NullSurface(const std::string& surface_name, const Size& size);

  void allocate(const Size& size);
  void deallocate();

  virtual Size size() const;

  virtual void blitToSurface(Surface& surface, 
                             const Rect& src, const Rect& dst,
                             int alpha = 255, bool useSrcAlpha = true);

  virtual void renderToScreen(
    const Rect& src, const Rect& dst,
    int alpha = 255);

  virtual void renderToScreenAsColorMask(
    const Rect& src, const Rect& dst,
    int r, int g, int b, int alpha, int filter);

  virtual void renderToScreen(
    const Rect& src, const Rect& dst, const int opacity[4]);

  virtual void renderToScreenAsObject(const GraphicsObject& rp);
  virtual void renderToScreenAsObject(const GraphicsObject& rp, 
                                      const GraphicsObjectOverride& override);

  virtual int numPatterns() const;
  virtual const GrpRect& getPattern(int pattNo) const;

  virtual void rawRenderQuad(const int srcCoords[8], 
                             const int destCoords[8],
                             const int opacity[4]);

  virtual void fill(int r, int g, int b, int alpha);
  virtual void fill(int r, int g, int b, int alpha, const Rect& rect);

  virtual void getDCPixel(const Point& pos, int& r, int& g, int& b);

  virtual boost::shared_ptr<Surface> clipAsColorMask(
    const Rect& rect, int r, int g, int b);

  virtual Surface* clone() const;

  MockLog& log() { return surface_log_; }

private:
  /// Unique name of this surface.
  std::string surface_name_;

  /// Whether we are allocated.
  bool allocated_;

  /// Supposed size of this surface.
  Size size_;

  /// The region table
  std::vector<GrpRect> m_regionTable;

  mutable MockLog surface_log_;
};


#endif
