#ifndef __System_hpp__
#define __System_hpp__

//#include "ScriptModifyable.hpp"

/** 
 * Abstract base class that provides a generalized interface to the
 * text windows.
 *
 */
/* class TextSystem : public ScriptModifyable
{
private:
  int xpos;
  int ypos;

  // The current size of the font
  int fontSizeInPixels;

  /// The text color
  int r, g, b, alpha, filter;

  /// Fast text mode
  bool fastTextMode;

  /// Internal 'no wait' flag
  bool messageNoWait;

  /// Message speed; range from 0 to 255
  char messageSpeed;

public:
  TextSystem() 
    : xpos(0), ypos(0), fontSizeInPixels(0), r(0), g(0), b(0), 
      alpha(0), filter(0), fastTextMode(0)
  {}

  virtual int& xpos() { return xpos; }
  virtual int& ypos() { return ypos; }

  virtual int& fontSizeInPixels() { return fontSizeInPixels; }
  
  virtual int& r() { return r; }
  virtual int& g() { return g; }
  virtual int& b() { return b; }
  virtual int& alpha() { return alpha; }
  virtual int& filter() { return filter; }

  virtual int& fastTextMode() { return fastTextMode; }
  virtual int& messageNoWait() { return messageNoWait; }
  virtual int& messageSpeed() { return messageSpeed; }
*/
  /** 
   * Give a default implementation for alertModified(). This should be
   * overridden by subclasses that need to be alerted when one of the
   * values were modified.
   */
//  virtual void alertModified() { }
//};

class GraphicsSystem;

/**
 * The system class provides a generalized interface to all the
 * components that make up a local system that may need to be
 * implemented differently on different systems, i.e., sound,
 * graphics, et cetera.
 *
 * The base System class is an abstract base class that 
 */
class System
{
public:
  virtual GraphicsSystem& graphics() = 0;
//  virtual TextSystem& textSystem() = 0;
//  virtual SoundSystem& soundSystem() = 0;
};

#endif
