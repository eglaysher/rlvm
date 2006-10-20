#ifndef __TextWindow_hpp__
#define __TextWindow_hpp__

/**
 * Abstract representation of a TextWindow. Aggrigated by TextSystem,
 * and rendered in conjunction with GraphicsSystem.
 *
 * This class has all sorts of complex, rarely used text rendering
 * options, including several co-ordinate systems, which I'm sure was
 * done to give me a headache.
 */
class TextWindow
{
private:
  /**
   * Describes the origin point of the window
   * 
   * @{
   */
  int windowPositionOrigin;
  int windowPositionX;
  int windowPositionY;
  /// @}

  /** The text insertion point. These two numbers are relative to the
   * text window location. 
   *
   * @{
   */
  int textInsertionPointX;
  int textInsertionPointY;
  /// @}
  
  /// The current size of the font
  int fontSizeInPixels;

  /// The text color
  int r, g, b, alpha, filter;

public:

  virtual void setFontSizeInPixels(int i) const { fontSizeInPixels = i; }
  virtual int& fontSizeInPixels() { return fontSizeInPixels; }

  virtual void setR(int i) const { r = i; }
  virtual void setG(int i) const { g = i; }
  virtual void setB(int i) const { b = i; }
  virtual void setAlph(int i) const { alpha = i; }
  virtual void setFilter(int i) const { filter = i; }
  
  virtual int& r() { return r; }
  virtual int& g() { return g; }
  virtual int& b() { return b; }
  virtual int& alpha() { return alpha; }
  virtual int& filter() { return filter; }
};

#endif
