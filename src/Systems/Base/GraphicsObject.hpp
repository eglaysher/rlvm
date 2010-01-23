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

#ifndef SRC_SYSTEMS_BASE_GRAPHICSOBJECT_HPP_
#define SRC_SYSTEMS_BASE_GRAPHICSOBJECT_HPP_

#include "Systems/Base/Colour.hpp"
#include "Systems/Base/Rect.hpp"

#include <string>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/version.hpp>

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
class GraphicsObject {
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
  int visible() const { return impl_->visible_; }
  void setVisible(const int in);

  int x() const { return impl_->x_; }
  void setX(const int x);

  int y() const { return impl_->y_; }
  void setY(const int y);

  int xAdjustment(int idx) const { return impl_->adjust_x_[idx]; }
  int xAdjustmentSum() const;
  void setXAdjustment(int idx, int x);

  int yAdjustment(int idx) const { return impl_->adjust_y_[idx]; }
  int yAdjustmentSum() const;
  void setYAdjustment(int idx, int y);

  int vert() const { return impl_->whatever_adjust_vert_operates_on_; }
  void setVert(const int vert);

  int xOrigin() const { return impl_->origin_x_; }
  void setXOrigin(const int x);

  int yOrigin() const { return impl_->origin_y_; }
  void setYOrigin(const int y);

  int width() const { return impl_->width_; }
  void setWidth(const int in);

  int height() const { return impl_->height_; }
  void setHeight(const int in);

  int rotation() const { return impl_->rotation_; }
  void setRotation(const int in);

  int pixelWidth() const;
  int pixelHeight() const;

  /// @}

  /**
   * @name Object attribute accessors
   *
   * @{
   */

  int pattNo() const { return impl_->patt_no_; }
  void setPattNo(const int in);

  int mono() const { return impl_->mono_; }
  void setMono(const int in);

  int invert() const { return impl_->invert_; }
  void setInvert(const int in);

  int light() const { return impl_->light_; }
  void setLight(const int in);

  const RGBColour& tint() const { return impl_->tint_; }
  void setTint(const RGBColour& colour);
  void setTintR(const int in);
  void setTintG(const int in);
  void setTintB(const int in);

  const RGBAColour& colour() const { return impl_->colour_; }
  void setColour(const RGBAColour& colour);
  void setColourR(const int in);
  void setColourG(const int in);
  void setColourB(const int in);
  void setColourLevel(const int in);

  int compositeMode() const { return impl_->composite_mode_; }
  void setCompositeMode(const int in);

  int scrollRateX() const { return impl_->scroll_rate_x_; }
  void setScrollRateX(const int x);

  int scrollRateY() const { return impl_->scroll_rate_y_; }
  void setScrollRateY(const int y);

  /// @}

  int alpha() const { return impl_->alpha_; }
  void setAlpha(const int alpha);

  bool hasClip() const {
    return impl_->clip_.width() >= 0 || impl_->clip_.height() >= 0;
  }
  void clearClip();
  void setClip(const Rect& rec);
  int clipX1() const { return impl_->clip_.x(); }
  int clipY1() const { return impl_->clip_.y(); }
  int clipX2() const { return impl_->clip_.x2(); }
  int clipY2() const { return impl_->clip_.y2(); }

  bool hasObjectData() const { return object_data_; }

  GraphicsObjectData& objectData();
  void setObjectData(GraphicsObjectData* obj);

  /// Render!
  void render(int objNum, std::ostream* tree);

  /**
   * Deletes the object data. Corresponds to the RLAPI command obj_delete.
   */
  void deleteObject();

  /**
   * Deletes the object data and resets all values in this
   * GraphicsObject. Corresponds to the RLAPI command obj_clear.
   */
  void clearObject();

  int wipeCopy() const { return impl_->wipe_copy_; }
  void setWipeCopy(const int wipe_copy);

  /**
   * Called each pass through the gameloop to see if this object needs
   * to force a redraw, or something.
   */
  void execute();

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

  // Drift object accessors
  void setDriftOpts(int count, int use_animation, int start_pattern,
                    int end_pattern, int total_animation_time_ms, int yspeed,
                    int period, int amplitude, int use_drift,
                    int unknown_drift_property, int driftspeed,
                    Rect driftarea);

  int driftParticleCount() const;
  int driftUseAnimation() const;
  int driftStartPattern() const;
  int driftEndPattern() const;
  int driftAnimationTime() const;
  int driftYSpeed() const;
  int driftPeriod() const;
  int driftAmplitude() const;
  int driftUseDrift() const;
  int driftUnknown() const;
  int driftDriftSpeed() const;
  Rect driftArea() const;

  // Returns the number of GraphicsObject instances sharing the
  // internal copy-on-write object. Only used in unit testing.
  int32_t referenceCount() const { return impl_.use_count(); }

  // Whether we have the default shared data. Only used in unit testing.
  bool isCleared() const { return impl_ == s_empty_impl; }

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
  struct Impl {
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
    bool visible_;

    /// The positional coordinates of the object
    int x_, y_;

    /// Eight additional parameters that are added to x and y during
    /// rendering. (WTF?!)
    int adjust_x_[8], adjust_y_[8];

    /// Whatever obj_adjust_vert operates on; what's this used for?
    int whatever_adjust_vert_operates_on_;

    /// The origin
    int origin_x_, origin_y_;

    /// "Rep" origin. This second origin is added to the normal origin
    /// only in cases of rotating and scaling.
    int rep_origin_x_, rep_origin_y_;

    /// The size of the object, given in integer percentages of [0,
    /// 100]. Used for scaling.
    int width_, height_;

    /// The rotation degree / 10
    int rotation_;

    /// @}

    // -----------------------------------------------------------------------

    /**
     * @name Object attributes.
     *
     * @{
     */

    /// The region ("pattern") in g00 bitmaps
    int patt_no_;

    /// The source alpha for this image
    int alpha_;

    /// The clipping region for this image
    Rect clip_;

    /// The monochrome transformation
    int mono_;

    /// The invert transformation
    int invert_;

    int light_;

    RGBColour tint_;

    /// Applies a colour to the object by blending it directly at the
    /// alpha components opacity.
    RGBAColour colour_;

    int composite_mode_;

    int scroll_rate_x_, scroll_rate_y_;

    /// @}

    // ---------------------------------------------------------------------

    /**
     * @name Animation state
     *
     * Certain pieces of state from Animated objects are cached on the
     * GraphicsObject to implement the delete-after-play semantics of
     * gan_play_once, et all.
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
    struct TextProperties {
      TextProperties();

      std::string value;

      int text_size, xspace, yspace;

      // Figure this out later.
      int vertical;
      int colour;
      int shadow_colour;

      /// boost::serialization support
      template<class Archive>
      void serialize(Archive& ar, unsigned int version);
    };

    void makeSureHaveTextProperties();
    boost::scoped_ptr<TextProperties> text_properties_;

    /// @}

    struct DriftProperties {
      DriftProperties();

      int count;

      int use_animation;
      int start_pattern;
      int end_pattern;
      int total_animation_time_ms;

      int yspeed;

      int period;
      int amplitude;

      int use_drift;
      int unknown_drift_property;
      int driftspeed;

      Rect drift_area;

      /// boost::serialization support
      template<class Archive>
      void serialize(Archive& ar, unsigned int version);
    };

    void makeSureHaveDriftProperties();
    boost::scoped_ptr<DriftProperties> drift_properties_;

    /// The wipe_copy bit
    int wipe_copy_;

    friend class boost::serialization::access;

    /// boost::serialization support
    template<class Archive>
    void serialize(Archive& ar, unsigned int version);
  };

  /**
   * Default empty GraphicsObject::Impl. This variable is allocated
   * once, and then is used as the initial value of impl_, where it
   * is cloned on write.
   */
  static const boost::shared_ptr<GraphicsObject::Impl> s_empty_impl;

  /// Our actual implementation data
  boost::shared_ptr<GraphicsObject::Impl> impl_;

  /// The actual data used to render the object
  boost::scoped_ptr<GraphicsObjectData> object_data_;

  friend class boost::serialization::access;

  /// boost::serialization support
  template<class Archive>
  void serialize(Archive& ar, unsigned int version);
};

BOOST_CLASS_VERSION(GraphicsObject::Impl, 1)

static const int OBJ_FG = 0;
static const int OBJ_BG = 1;

#endif  // SRC_SYSTEMS_BASE_GRAPHICSOBJECT_HPP_

