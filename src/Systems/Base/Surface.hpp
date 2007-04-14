#ifndef __Surface_hpp__
#define __Surface_hpp__

/** 
 * Abstract concept of a surface. Used 
 * 
 * @return 
 */
class Surface
{
public:
  struct GrpRect {
    int x1, y1, x2, y2;
  };

public:
  virtual ~Surface() { }

  virtual int width() const = 0;
  virtual int height() const = 0;

  virtual void dump() {}

  /// Blits to another surface
  virtual void blitToSurface(Surface& surface, 
                             int srcX, int srcY, int srcWidth, int srcHeight,
                             int destX, int destY, int destWidth, int destHeight,
                             int alpha = 255, bool useSrcAlpha = true) { }

  virtual void renderToScreen(
                     int srcX, int srcY, int srcWidth, int srcHeight,
                     int destX, int destY, int destWidth, int destHeight,
                     int alpha = 255) { }

  virtual void renderToScreenAsColorMask(
                     int srcX1, int srcY1, int srcX2, int srcY2,
                     int destX1, int destY1, int destX2, int destY2,
                     int r, int g, int b, int alpha, int filter) { }

  virtual void renderToScreen(
    int srcX1, int srcY1, int srcX2, int srcY2,
    int destX1, int destY1, int destX2, int destY2,
    const int opacity[4]) { }

  virtual const GrpRect& getPattern(int pattNo) const 
  {
    static GrpRect rect;
    return rect;
  };

  virtual void rawRenderQuad(const int srcCoords[8], 
                             const int destCoords[8],
                             const int opacity[4]) { }

  virtual void fill(int r, int g, int b, int alpha) = 0;
  virtual void fill(int r, int g, int b, int alpha, int x, int y, 
                    int width, int height) = 0;

  virtual Surface* clone() const = 0;
};

#endif 
