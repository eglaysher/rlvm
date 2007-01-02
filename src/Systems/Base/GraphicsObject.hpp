#ifndef __GraphicObject_hpp__
#define __GraphicObject_hpp__

#include <boost/scoped_ptr.hpp>

class RLMachine;
class GraphicsObjectSlot;

/**
 * Describes what is rendered in a graphics object;
 * 
 */
class GraphicsObjectData {
  virtual void render(RLMachine& machine, 
                      const GraphicsObject& renderingProperties) = 0;
};

// -----------------------------------------------------------------------

/**
 * Describes an independent, movable graphical object on the
 * screen. GraphicsObject simply contains a set of properties and a
 * GraphicsObjectData object, which we dispatch render calls to if it
 * exists.
 */
class GraphicsObject
{
public:
  GraphicsObject();

  /**
   * @name Object Position Accessors
   * 
   * @{
   */

  bool visible() const { return m_visible; }
  void setVisible(const bool in) { m_visible = in; }

  int x() const { return m_x; }
  void setX(const int x) { m_x = x; }

  int y() const { return m_y; }
  void setY(const int y) { m_y = y; }

  int xAdjustment(int idx) const { return m_adjustX[idx]; }
  void setXAdjustment(int idx, int x) { m_adjustX[idx] = x; }

  int yAdjustment(int idx) const { return m_adjustY[idx]; }
  void setYAdjustment(int idx, int y) { m_adjustY[idx] = y; }

  int vert() const { return m_whateverAdjustVertOperatesOn; }
  void setVert(const int vert) { m_whateverAdjustVertOperatesOn = vert; }

  int xOrigin() const { return m_originX; }
  void setXOrigin(const int x) { m_originX = x; }

  int yOrigin() const { return m_originY; }
  void setYOrigin(const int y) { m_originY = y; }

  /// @}

  /**
   * @name Object attribute accessors
   * 
   * @{
   */

  int mono() const { return m_mono; }
  void setMono(const int in) { m_mono = in; }

  int invert() const { return m_invert; }
  void setInvert(const int in) { m_invert = in; }

  int light() const { return m_light; }
  void setLight(const int in) { m_light = in; }

  int tintR() const { return m_tintR; }
  void setTintR(const int in) { m_tintR = in; }
  int tintG() const { return m_tintG; }
  void setTintG(const int in) { m_tintG = in; }
  int tintB() const { return m_tintB; }
  void setTintB(const int in) { m_tintB = in; }

  int colourR() const { return m_colourR; }
  void setColourR(const int in) { m_colourR = in; }
  int colourG() const { return m_colourG; }
  void setColourG(const int in) { m_colourG = in; }
  int colourB() const { return m_colourB; }
  void setColourB(const int in) { m_colourB = in; }
  int colourLevel() const { return m_colourLevel; }
  void setColourLevel(const int in) { m_colourLevel = in; }

  int compositeMode() const { return m_compositeMode; }
  void setCompositeMode(const int in) { m_compositeMode = in; }

  int scrollRateX() const { return m_scrollRateX; }
  void setScrollRateX(const int x) { m_scrollRateX = x; }

  int scrollRateY() const { return m_scrollRateY; }
  void setScrollRateY(const int y) { m_scrollRateY = y; }

  /// @}

  int alpha() const { return m_alpha; }
  void setAlpha(const int alpha) { m_alpha = alpha; }

  /// Render!
  void render(RLMachine& machine);
  
private:

  /**
   * @name Object Position Variables
   * 
   * Describes various properties as defined in section 5.12.3 of the
   * RLDev manual.
   * 
   * @{
   */

  /// Visiblitiy. Different from whether an object is in the bg or fg layer
  bool m_visible;

  /// The positional coordinates of the object
  int m_x, m_y;

  /// Eight additional parameters that are added to x and y during
  /// rendering. (WTF?!)
  int m_adjustX[8], m_adjustY[8];

  /// Whatever objAdjustVert operates on; what's this used for?
  int m_whateverAdjustVertOperatesOn;

  /// The origin
  int m_originX, m_originY;

  /// "Rep" origin. This second origin is added to the normal origin
  /// only in cases of rotating and scaling.
  int m_repOriginX, m_repOriginY;

  /// The size of the object, given in integer percentages of [0,
  /// 100]. Used for scaling.
  int m_width, m_height;

  /// The rotation degree / 10
  int m_rotation;

  /// @}

  // -----------------------------------------------------------------------

  /**
   * @name Object attributes.
   * 
   * @{
   */

  /// The region in g00 bitmaps
  int m_type2g00region;

  /// The source alpha for this image
  int m_alpha;

  /// The monochrome transformation
  int m_mono;

  /// The invert transformation
  int m_invert;

  int m_light;

  int m_tintR, m_tintG, m_tintB;

  int m_colourR, m_colourG, m_colourB, m_colourLevel;

  int m_compositeMode;

  int m_scrollRateX, m_scrollRateY;

  /// @}

  /// The actual data used to render the object
  boost::scoped_ptr<GraphicsObjectData> m_objectData;
};


#endif 
