// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006, 2007 Elliot Glaysher
//  
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
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

#include "Systems/Base/Colour.hpp"

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/serialization/access.hpp>

class RLMachine;
class GraphicsObject;
class GraphicsObjectSlot;
class GraphicsObjectData;

/**
 * Describes an independent, movable graphical object on the
 * screen. GraphicsObject, internally, references a copy-on-write
 * datastructure, which in turn has optional components to save
 * memory.
 *
 * @todo I want to put index checks on a lot of these accessors.
 */
class GraphicsObject
{
public:
  GraphicsObject();
  GraphicsObject(const GraphicsObject& obj);
  ~GraphicsObject();

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
  int visible() const { return m_impl->m_visible; }
  void setVisible(const int in);

  int x() const { return m_impl->m_x; }
  void setX(const int x);

  int y() const { return m_impl->m_y; }
  void setY(const int y);
  
  int xAdjustment(int idx) const { return m_impl->m_adjustX[idx]; }
  int xAdjustmentSum() const;
  void setXAdjustment(int idx, int x);

  int yAdjustment(int idx) const { return m_impl->m_adjustY[idx]; }
  int yAdjustmentSum() const;
  void setYAdjustment(int idx, int y);

  int vert() const { return m_impl->m_whateverAdjustVertOperatesOn; }
  void setVert(const int vert);

  int xOrigin() const { return m_impl->m_originX; }
  void setXOrigin(const int x);

  int yOrigin() const { return m_impl->m_originY; }
  void setYOrigin(const int y);

  int width() const { return m_impl->m_width; }
  void setWidth(const int in);

  int height() const { return m_impl->m_height; }
  void setHeight(const int in);

  int rotation() const { return m_impl->m_rotation; }
  void setRotation(const int in);

  int pixelWidth(RLMachine& machine) const;
  int pixelHeight(RLMachine& machine) const;

  /// @}

  /**
   * @name Object attribute accessors
   * 
   * @{
   */

  int pattNo() const { return m_impl->m_pattNo; }
  void setPattNo(const int in);

  int mono() const { return m_impl->m_mono; }
  void setMono(const int in);

  int invert() const { return m_impl->m_invert; }
  void setInvert(const int in);

  int light() const { return m_impl->m_light; }
  void setLight(const int in);

  const RGBColour tint() const { return m_impl->m_tint; }
  int tintR() const { return m_impl->m_tint.r(); }
  void setTintR(const int in);
  int tintG() const { return m_impl->m_tint.g(); }
  void setTintG(const int in);
  int tintB() const { return m_impl->m_tint.b(); }
  void setTintB(const int in);

  int colourR() const { return m_impl->m_colour.r(); }
  void setColourR(const int in);
  int colourG() const { return m_impl->m_colour.g(); }
  void setColourG(const int in);
  int colourB() const { return m_impl->m_colour.b(); }
  void setColourB(const int in);
  int colourLevel() const { return m_impl->m_colour.a(); }
  void setColourLevel(const int in);

  int compositeMode() const { return m_impl->m_compositeMode; }
  void setCompositeMode(const int in);

  int scrollRateX() const { return m_impl->m_scrollRateX; }
  void setScrollRateX(const int x);

  int scrollRateY() const { return m_impl->m_scrollRateY; }
  void setScrollRateY(const int y);

  /// @}

  int alpha() const { return m_impl->m_alpha; }
  void setAlpha(const int alpha);

  bool hasClip() const { return m_impl->m_clipX2 >= 0 || m_impl->m_clipY2 >= 0; }
  void clearClip();
  void setClip(const int x1, const int y1, const int x2, const int y2);
  int clipX1() const { return m_impl->m_clipX1; }
  int clipY1() const { return m_impl->m_clipY1; }
  int clipX2() const { return m_impl->m_clipX2; }
  int clipY2() const { return m_impl->m_clipY2; }
  
  bool hasObjectData() const { return m_objectData; }

  GraphicsObjectData& objectData();
  void setObjectData(GraphicsObjectData* obj);

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

  int wipeCopy() const { return m_impl->m_wipeCopy; }
  void setWipeCopy(const int wipeCopy);

  /**
   * Called each pass through the gameloop to see if this object needs
   * to force a redraw, or something.
   */
  void execute(RLMachine& machine);

  /**
   * @name Text Object accessors
   * 
   * @{
   */
  void setTextText(const std::string& utf8str);
  const std::string& textText() const;
 
  void setTextOps(int size, int xspace, int yspace, int vertical, int colour, 
				  int shadow);
  int textSize() const;
  int textXSpace() const;
  int textYSpace() const;
  int textVertical() const;
  int textColour() const;
  int textShadowColour() const;
  // @}  

  /** 
   * Returns the number of GraphicsObject instances sharing the
   * internal copy-on-write object.
   */
  long referenceCount() const { return m_impl.use_count(); }

private:
  /** 
   * Makes the ineternal copy for our copy-on-write semantics. This
   * function checks to see if our Impl object has only one reference
   * to it. If it doesn't, a local copy is made.
   */
  void makeImplUnique();

  /** 
   * Implementation data structure. GraphicsObject::Impl is the
   * internal data store for GraphicsObjects' copy-on-write semantics. It is 
   * 
   */
  struct Impl
  {
    Impl();
    Impl(const Impl& rhs);
    ~Impl();

    Impl& operator=(const Impl& rhs);

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

    RGBColour m_tint;

    /// Applies a colour to the object by blending it directly at the
    /// alpha components opacity.
    RGBAColour m_colour;

    int m_compositeMode;
  
    int m_scrollRateX, m_scrollRateY;

    /// @}

    // ---------------------------------------------------------------------

    /**
     * @name Animation state
     * 
     * Certain pieces of state from Animated objects are cached on the
     * GraphicsObject to implement the delete-after-play semantics of
     * ganPlayOnce, et all.
     *
     * @{
     */

    /// @}



    // -----------------------------------------------------------------------

    /**
     * @name Text Object properties
     * 
     * @{
     */
    struct TextProperties
    {
      TextProperties();

      std::string value;

      int textSize, xspace, yspace;

      // Figure this out later.
      int vertical;
      int colour;
      int shadowColour;    

      /// boost::serialization support
      template<class Archive>
      void serialize(Archive& ar, unsigned int version);
    };

    void makeSureHaveTextProperties();
    boost::scoped_ptr<TextProperties> m_textProperties;

    /// @}

    /// The wipeCopy bit
    int m_wipeCopy;

    friend class boost::serialization::access;

    /// boost::serialization support
    template<class Archive>
    void serialize(Archive& ar, unsigned int version);
  };

  /** 
   * Default empty GraphicsObject::Impl. This variable is allocated
   * once, and then is used as the initial value of m_impl, where it
   * is cloned on write.
   */
  static const boost::shared_ptr<GraphicsObject::Impl> s_emptyImpl;

  /// Our actual implementation data
  boost::shared_ptr<GraphicsObject::Impl> m_impl;

  /// The actual data used to render the object
  boost::scoped_ptr<GraphicsObjectData> m_objectData;

  friend class boost::serialization::access;

  /// boost::serialization support
  template<class Archive>
  void serialize(Archive& ar, unsigned int version);
};

const static int OBJ_FG_LAYER = 0;
const static int OBJ_BG_LAYER = 1;

#endif 

