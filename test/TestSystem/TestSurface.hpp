// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//

#ifndef __TestSurface_hpp__
#define __TestSurface_hpp__

#include "Systems/Base/Surface.hpp"

#include <string>
#include <vector>

// -----------------------------------------------------------------------

class TestSurface : public Surface
{
public:
  TestSurface(const std::string& surface_name);
  TestSurface(const std::string& surface_name, const Size& size);

  void allocate(const Size& size);
  void deallocate();

  virtual Size size() const;

  virtual void blitToSurface(Surface& surface,
                             const Rect& src, const Rect& dst,
                             int alpha = 255, bool use_src_alpha = true);

  virtual void renderToScreen(
    const Rect& src, const Rect& dst,
    int alpha = 255);

  virtual void renderToScreenAsColorMask(
    const Rect& src, const Rect& dst, const RGBAColour& rgba, int filter);

  virtual void renderToScreen(
    const Rect& src, const Rect& dst, const int opacity[4]);

  virtual void renderToScreenAsObject(const GraphicsObject& rp,
                                      const Rect& src,
                                      const Rect& dst,
                                      int alpha);

  virtual int numPatterns() const;
  virtual const GrpRect& getPattern(int patt_no) const;

  virtual void rawRenderQuad(const int src_coords[8],
                             const int dest_coords[8],
                             const int opacity[4]);

  virtual void fill(const RGBAColour& colour);
  virtual void fill(const RGBAColour& colour, const Rect& rect);

  virtual void getDCPixel(const Point& pos, int& r, int& g, int& b);

  virtual boost::shared_ptr<Surface> clipAsColorMask(
    const Rect& rect, int r, int g, int b);

  virtual Surface* clone() const;

private:
  /// Unique name of this surface.
  std::string surface_name_;

  /// Whether we are allocated.
  bool allocated_;

  /// Supposed size of this surface.
  Size size_;

  /// The region table
  std::vector<GrpRect> region_table_;
};


#endif
