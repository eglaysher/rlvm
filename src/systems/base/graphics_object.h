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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
//
// -----------------------------------------------------------------------

#ifndef SRC_SYSTEMS_BASE_GRAPHICS_OBJECT_H_
#define SRC_SYSTEMS_BASE_GRAPHICS_OBJECT_H_

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/version.hpp>

#include <string>
#include <vector>

#include "systems/base/colour.h"
#include "systems/base/rect.h"

class RLMachine;
class GraphicsObject;
class GraphicsObjectSlot;
class GraphicsObjectData;
class ObjectMutator;

// Describes an independent, movable graphical object on the
// screen. GraphicsObject, internally, references a copy-on-write
// datastructure, which in turn has optional components to save
// memory.
//
// TODO(erg): I want to put index checks on a lot of these accessors.
class GraphicsObject {
 public:
  GraphicsObject();
  GraphicsObject(const GraphicsObject& obj);
  ~GraphicsObject();
  GraphicsObject& operator=(const GraphicsObject& obj);

  // Object Position Accessors

  // This code, while a boolean, uses an int so that we can get rid
  // of one template parameter in one of the generic operation
  // functors.
  int visible() const { return impl_->visible_; }
  void SetVisible(const int in);

  int x() const { return impl_->x_; }
  void SetX(const int x);

  int y() const { return impl_->y_; }
  void SetY(const int y);

  int x_adjustment(int idx) const { return impl_->adjust_x_[idx]; }
  int GetXAdjustmentSum() const;
  void SetXAdjustment(int idx, int x);

  int y_adjustment(int idx) const { return impl_->adjust_y_[idx]; }
  int GetYAdjustmentSum() const;
  void SetYAdjustment(int idx, int y);

  int vert() const { return impl_->whatever_adjust_vert_operates_on_; }
  void SetVert(const int vert);

  int origin_x() const { return impl_->origin_x_; }
  void SetOriginX(const int x);

  int origin_y() const { return impl_->origin_y_; }
  void SetOriginY(const int y);

  int rep_origin_x() const { return impl_->rep_origin_x_; }
  void SetRepOriginX(const int x);

  int rep_origin_y() const { return impl_->rep_origin_y_; }
  void SetRepOriginY(const int y);

  // Note: width/height are object scale percentages.
  int width() const { return impl_->width_; }
  void SetWidth(const int in);
  int height() const { return impl_->height_; }
  void SetHeight(const int in);

  // Note: width/height are object scale factors out of 1000.
  int hq_width() const { return impl_->hq_width_; }
  void SetHqWidth(const int in);
  int hq_height() const { return impl_->hq_height_; }
  void SetHqHeight(const int in);

  float GetWidthScaleFactor() const;
  float GetHeightScaleFactor() const;

  int rotation() const { return impl_->rotation_; }
  void SetRotation(const int in);

  int PixelWidth() const;
  int PixelHeight() const;

  // Object attribute accessors
  int GetPattNo() const;
  void SetPattNo(const int in);

  int mono() const { return impl_->mono_; }
  void SetMono(const int in);

  int invert() const { return impl_->invert_; }
  void SetInvert(const int in);

  int light() const { return impl_->light_; }
  void SetLight(const int in);

  const RGBColour& tint() const { return impl_->tint_; }
  int tint_red() const { return impl_->tint_.r(); }
  int tint_green() const { return impl_->tint_.g(); }
  int tint_blue() const { return impl_->tint_.b(); }
  void SetTint(const RGBColour& colour);
  void SetTintRed(const int in);
  void SetTintGreen(const int in);
  void SetTintBlue(const int in);

  const RGBAColour& colour() const { return impl_->colour_; }
  int colour_red() const { return impl_->colour_.r(); }
  int colour_green() const { return impl_->colour_.g(); }
  int colour_blue() const { return impl_->colour_.b(); }
  int colour_level() const { return impl_->colour_.a(); }
  void SetColour(const RGBAColour& colour);
  void SetColourRed(const int in);
  void SetColourGreen(const int in);
  void SetColourBlue(const int in);
  void SetColourLevel(const int in);

  int composite_mode() const { return impl_->composite_mode_; }
  void SetCompositeMode(const int in);

  int scroll_rate_x() const { return impl_->scroll_rate_x_; }
  void SetScrollRateX(const int x);

  int scroll_rate_y() const { return impl_->scroll_rate_y_; }
  void SetScrollRateY(const int y);

  // Three level zorder.
  int z_order() const { return impl_->z_order_; }
  void SetZOrder(const int in);
  int z_layer() const { return impl_->z_layer_; }
  void SetZLayer(const int in);
  int z_depth() const { return impl_->z_depth_; }
  void SetZDepth(const int in);

  int GetComputedAlpha() const;
  int raw_alpha() const { return impl_->alpha_; }
  void SetAlpha(const int alpha);

  int alpha_adjustment(int idx) const { return impl_->adjust_alpha_[idx]; }
  void SetAlphaAdjustment(int idx, int alpha);

  const Rect& clip_rect() const { return impl_->clip_; }
  bool has_clip_rect() const {
    return impl_->clip_.width() >= 0 || impl_->clip_.height() >= 0;
  }
  void ClearClipRect();
  void SetClipRect(const Rect& rec);

  const Rect& own_clip_rect() const { return impl_->own_clip_; }
  bool has_own_clip_rect() const {
    return impl_->own_clip_.width() >= 0 || impl_->own_clip_.height() >= 0;
  }
  void ClearOwnClipRect();
  void SetOwnClipRect(const Rect& rec);

  bool has_object_data() const { return object_data_.get(); }

  GraphicsObjectData& GetObjectData();
  void SetObjectData(GraphicsObjectData* obj);

  // Render!
  void Render(int objNum, const GraphicsObject* parent, std::ostream* tree);

  // Frees the object data. Corresponds to objFree, but is also invoked by
  // other commands.
  void FreeObjectData();

  // Resets/reinitializes all the object parameters without deleting the loaded
  // graphics object data.
  void InitializeParams();

  // Both frees the object data and initializes parameters.
  void FreeDataAndInitializeParams();

  int wipe_copy() const { return impl_->wipe_copy_; }
  void SetWipeCopy(const int wipe_copy);

  // Called each pass through the gameloop to see if this object needs
  // to force a redraw, or something.
  void Execute(RLMachine& machine);

  // Text Object accessors
  void SetTextText(const std::string& utf8str);
  const std::string& GetTextText() const;

  void SetTextOps(int size,
                  int xspace,
                  int yspace,
                  int char_count,
                  int colour,
                  int shadow);
  int GetTextSize() const;
  int GetTextXSpace() const;
  int GetTextYSpace() const;
  int GetTextCharCount() const;
  int GetTextColour() const;
  int GetTextShadowColour() const;

  // Drift object accessors
  void SetDriftOpts(int count,
                    int use_animation,
                    int start_pattern,
                    int end_pattern,
                    int total_animation_time_ms,
                    int yspeed,
                    int period,
                    int amplitude,
                    int use_drift,
                    int unknown_drift_property,
                    int driftspeed,
                    Rect driftarea);

  int GetDriftParticleCount() const;
  int GetDriftUseAnimation() const;
  int GetDriftStartPattern() const;
  int GetDriftEndPattern() const;
  int GetDriftAnimationTime() const;
  int GetDriftYSpeed() const;
  int GetDriftPeriod() const;
  int GetDriftAmplitude() const;
  int GetDriftUseDrift() const;
  int GetDriftUnknown() const;
  int GetDriftDriftSpeed() const;
  Rect GetDriftArea() const;

  // Digit object accessors
  void SetDigitValue(int value);
  void SetDigitOpts(int digits, int zero, int sign, int pack, int space);

  int GetDigitValue() const;
  int GetDigitDigits() const;
  int GetDigitZero() const;
  int GetDigitSign() const;
  int GetDigitPack() const;
  int GetDigitSpace() const;

  // Button object accessors
  void SetButtonOpts(int action, int se, int group, int button_number);
  void SetButtonState(int state);

  int IsButton() const;
  int GetButtonAction() const;
  int GetButtonSe() const;
  int GetButtonGroup() const;
  int GetButtonNumber() const;
  int GetButtonState() const;

  // Called only from ButtonObjectSelectLongOperation. Sets override
  // properties.
  void SetButtonOverrides(int override_pattern,
                          int override_x_offset,
                          int override_y_offset);
  void ClearButtonOverrides();

  bool GetButtonUsingOverides() const;
  int GetButtonPatternOverride() const;
  int GetButtonXOffsetOverride() const;
  int GetButtonYOffsetOverride() const;

  // Adds a mutator to the list of active mutators. GraphicsSystem takes
  // ownership of the passed in object.
  void AddObjectMutator(std::unique_ptr<ObjectMutator> mutator);

  // Returns true if a mutator matching the following parameters is currently
  // running.
  bool IsMutatorRunningMatching(int repno, const std::string& name);

  // Ends all mutators that match the given parameters.
  void EndObjectMutatorMatching(RLMachine& machine,
                                int repno,
                                const std::string& name,
                                int speedup);

  // Returns a string for each mutator.
  std::vector<std::string> GetMutatorNames() const;

  // Returns the number of GraphicsObject instances sharing the
  // internal copy-on-write object. Only used in unit testing.
  int32_t reference_count() const { return impl_.use_count(); }

  // Whether we have the default shared data. Only used in unit testing.
  bool is_cleared() const { return impl_ == s_empty_impl; }

 private:
  // Makes the internal copy for our copy-on-write semantics. This function
  // checks to see if our Impl object has only one reference to it. If it
  // doesn't, a local copy is made.
  void MakeImplUnique();

  // Immediately delete all mutators; doesn't run their SetToEnd() method.
  void DeleteObjectMutators();

  // Implementation data structure. GraphicsObject::Impl is the internal data
  // store for GraphicsObjects' copy-on-write semantics.
  struct Impl {
    Impl();
    Impl(const Impl& rhs);
    ~Impl();

    Impl& operator=(const Impl& rhs);

    // Visibility. Different from whether an object is in the bg or fg layer
    bool visible_;

    // The positional coordinates of the object
    int x_, y_;

    // Eight additional parameters that are added to x and y during
    // rendering.
    int adjust_x_[8], adjust_y_[8];

    // Whatever obj_adjust_vert operates on; what's this used for?
    int whatever_adjust_vert_operates_on_;

    // The origin
    int origin_x_, origin_y_;

    // "Rep" origin. This second origin is added to the normal origin
    // only in cases of rotating and scaling.
    int rep_origin_x_, rep_origin_y_;

    // The size of the object, given in integer percentages of [0,
    // 100]. Used for scaling.
    int width_, height_;

    // A second scaling factor, given between [0, 1000].
    int hq_width_, hq_height_;

    // The rotation degree / 10
    int rotation_;

    // Object attributes.

    // The region ("pattern") in g00 bitmaps
    int patt_no_;

    // The source alpha for this image
    int alpha_;

    // Eight additional alphas that are averaged during rendering.
    int adjust_alpha_[8];

    // The clipping region for this image
    Rect clip_;

    // A second clipping region in the object's own space.
    Rect own_clip_;

    // The monochrome transformation
    int mono_;

    // The invert transformation
    int invert_;

    int light_;

    RGBColour tint_;

    // Applies a colour to the object by blending it directly at the
    // alpha components opacity.
    RGBAColour colour_;

    int composite_mode_;

    int scroll_rate_x_, scroll_rate_y_;

    // Three deep zordering.
    int z_order_, z_layer_, z_depth_;

    // Text Object properties
    struct TextProperties {
      TextProperties();

      std::string value;

      int text_size, xspace, yspace;

      int char_count;
      int colour;
      int shadow_colour;

      // boost::serialization support
      template <class Archive>
      void serialize(Archive& ar, unsigned int version);
    };

    void MakeSureHaveTextProperties();
    boost::scoped_ptr<TextProperties> text_properties_;

    // Drift Object properties
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

      // boost::serialization support
      template <class Archive>
      void serialize(Archive& ar, unsigned int version);
    };

    void MakeSureHaveDriftProperties();
    boost::scoped_ptr<DriftProperties> drift_properties_;

    // Digit Object properties
    struct DigitProperties {
      DigitProperties();

      int value;

      int digits;
      int zero;
      int sign;
      int pack;
      int space;

      // boost::serialization support
      template <class Archive>
      void serialize(Archive& ar, unsigned int version);
    };

    void MakeSureHaveDigitProperties();
    boost::scoped_ptr<DigitProperties> digit_properties_;

    // Button Object properties
    struct ButtonProperties {
      ButtonProperties();

      int is_button;

      int action;
      int se;
      int group;
      int button_number;

      int state;

      bool using_overides;
      int pattern_override;
      int x_offset_override;
      int y_offset_override;

      // boost::serialization support
      template <class Archive>
      void serialize(Archive& ar, unsigned int version);
    };

    void MakeSureHaveButtonProperties();
    boost::scoped_ptr<ButtonProperties> button_properties_;

    // The wipe_copy bit
    int wipe_copy_;

    friend class boost::serialization::access;

    // boost::serialization support
    template <class Archive>
    void serialize(Archive& ar, unsigned int version);
  };

  // Default empty GraphicsObject::Impl. This variable is allocated
  // once, and then is used as the initial value of impl_, where it
  // is cloned on write.
  static const boost::shared_ptr<GraphicsObject::Impl> s_empty_impl;

  // Our actual implementation data
  boost::shared_ptr<GraphicsObject::Impl> impl_;

  // The actual data used to render the object
  boost::scoped_ptr<GraphicsObjectData> object_data_;

  // Tasks that run every tick. Used to mutate object parameters over time (and
  // how we check from a blocking LongOperation if the mutation is ongoing).
  //
  // I think R23 mentioned that these were called "Parameter Events" in the
  // RLMAX SDK.
  std::vector<std::unique_ptr<ObjectMutator>> object_mutators_;

  friend class boost::serialization::access;

  // boost::serialization support
  template <class Archive>
  void serialize(Archive& ar, unsigned int version);
};

BOOST_CLASS_VERSION(GraphicsObject::Impl, 7)

static const int OBJ_FG = 0;
static const int OBJ_BG = 1;

#endif  // SRC_SYSTEMS_BASE_GRAPHICS_OBJECT_H_
