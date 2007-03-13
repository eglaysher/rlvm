// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006 El Riot
//  
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//  
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//  
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//  
// -----------------------------------------------------------------------

#ifndef __GraphicObject_hpp__
#define __GraphicObject_hpp__

#include <boost/scoped_ptr.hpp>

class RLMachine;
class GraphicsObject;
class GraphicsObjectSlot;

/**
 * Describes what is rendered in a graphics object;
 * 
 */
class GraphicsObjectData {
public:
  virtual ~GraphicsObjectData();
  virtual void render(RLMachine& machine, 
                      const GraphicsObject& renderingProperties) = 0;
  
  virtual int pixelWidth(const GraphicsObject& renderingProperties) = 0;
  virtual int pixelHeight(const GraphicsObject& renderingProperties) = 0;

  virtual GraphicsObjectData* clone() const = 0;
};

// -----------------------------------------------------------------------

/**
 * Describes an independent, movable graphical object on the
 * screen. GraphicsObject simply contains a set of properties and a
 * GraphicsObjectData object, which we dispatch render calls to if it
 * exists.
 *
 * @todo I want to put index checks on a lot of these accessors.
 */
class GraphicsObject
{
public:
  GraphicsObject();

  GraphicsObject(const GraphicsObject& obj);

  /** 
   * Copy operator.
   * 
   * @param obj 
   */
  GraphicsObject& operator=(const GraphicsObject& obj);

  /**
   * @name Object Position Accessors
   * 
   * @{
   */
  
  /// This code, while a boolean, uses an int so that we can get rid
  /// of one template parameter in one of the generic operation
  /// functors.
  int visible() const { return m_visible; }
  void setVisible(const int in) { m_visible = in; }

  int x() const { return m_x; }
  void setX(const int x) { m_x = x; }

  int y() const { return m_y; }
  void setY(const int y) { m_y = y; }
  
  int xAdjustment(int idx) const { return m_adjustX[idx]; }
  int xAdjustmentSum() const;
  void setXAdjustment(int idx, int x) { m_adjustX[idx] = x; }

  int yAdjustment(int idx) const { return m_adjustY[idx]; }
  int yAdjustmentSum() const;
  void setYAdjustment(int idx, int y) { m_adjustY[idx] = y; }

  int vert() const { return m_whateverAdjustVertOperatesOn; }
  void setVert(const int vert) { m_whateverAdjustVertOperatesOn = vert; }

  int xOrigin() const { return m_originX; }
  void setXOrigin(const int x) { m_originX = x; }

  int yOrigin() const { return m_originY; }
  void setYOrigin(const int y) { m_originY = y; }

  int width() const { return m_width; }
  void setWidth(const int in) { m_width = in; }

  int height() const { return m_height; }
  void setHeight(const int in) { m_height = in; }

  int pixelWidth() const;
  int pixelHeight() const;

  int rotation() const { return m_rotation; }
  void setRotation(const int in) { m_rotation = in; }

  /// @}

  /**
   * @name Object attribute accessors
   * 
   * @{
   */

  int pattNo() const { return m_pattNo; }
  void setPattNo(const int in) { m_pattNo = in; }

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
  void setAlpha(const int alpha);

  bool hasClip() const { return m_clipX2 >= 0 || m_clipY2 >= 0; }
  void clearClip() { m_clipX2 = -1; m_clipY2 = -1; }
  void setClip(const int x1, const int y1, const int x2, const int y2) {
    m_clipX1 = x1; m_clipY1 = y1; m_clipX2 = x2; m_clipY2 = y2;
  }
  int clipX1() const { return m_clipX1; }
  int clipY1() const { return m_clipY1; }
  int clipX2() const { return m_clipX2; }
  int clipY2() const { return m_clipY2; }
  
  bool hasObjectData() const { return m_objectData; }

  GraphicsObjectData& objectData() const;

  void setObjectData(GraphicsObjectData* obj) {
    m_objectData.reset(obj);
  }

  /// Render!
  void render(RLMachine& machine);

  /** 
   * Deletes the object data. Corresponds to the RLAPI command objDelete.
   */
  void deleteObject();

  /** 
   * Deletes the object data and resets all values in this
   * GraphicsObject. Corresponds to the RLAPI command objClear.
   */
  void clearObject();

  int wipeCopy() const { return m_wipeCopy; }
  void setWipeCopy(const int wipeCopy) { m_wipeCopy = wipeCopy; }
  
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

  /// The region ("pattern") in g00 bitmaps
  int m_pattNo;

  /// The source alpha for this image
  int m_alpha;

  /// The clipping region for this image
  int m_clipX1, m_clipY1, m_clipX2, m_clipY2;

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

  /// The wipeCopy bit
  int m_wipeCopy;

  /// The actual data used to render the object
  boost::scoped_ptr<GraphicsObjectData> m_objectData;
};


#endif 
