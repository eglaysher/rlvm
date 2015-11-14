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

// Contains definitions for object handling functions for the Modules 81
// "ObjFg", 82 "ObjBg", 90 "ObjRange", and 91 "ObjBgRange".

#include "modules/module_obj_fg_bg.h"

#include <cmath>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "libreallive/bytecode.h"
#include "libreallive/gameexe.h"
#include "machine/long_operation.h"
#include "machine/properties.h"
#include "machine/rlmachine.h"
#include "machine/rlmodule.h"
#include "machine/rloperation.h"
#include "machine/rloperation/default_value.h"
#include "machine/rloperation/rect_t.h"
#include "modules/module_obj.h"
#include "modules/object_mutator_operations.h"
#include "systems/base/colour_filter_object_data.h"
#include "systems/base/event_system.h"
#include "systems/base/graphics_object.h"
#include "systems/base/graphics_object_data.h"
#include "systems/base/graphics_system.h"
#include "systems/base/graphics_text_object.h"
#include "systems/base/object_mutator.h"
#include "systems/base/system.h"
#include "utilities/exception.h"
#include "utilities/graphics.h"
#include "utilities/string_utilities.h"

namespace {

struct dispArea_0 : public RLOpcode<IntConstant_T> {
  void operator()(RLMachine& machine, int buf) {
    GraphicsObject& obj = GetGraphicsObject(machine, this, buf);
    obj.ClearClipRect();
    machine.system().graphics().mark_object_state_as_dirty();
  }
};

struct dispArea_1 : RLOpcode<IntConstant_T,
                                IntConstant_T,
                                IntConstant_T,
                                IntConstant_T,
                                IntConstant_T> {
  void operator()(RLMachine& machine, int buf, int x1, int y1, int x2, int y2) {
    GraphicsObject& obj = GetGraphicsObject(machine, this, buf);
    obj.SetClipRect(Rect::GRP(x1, y1, x2, y2));
    machine.system().graphics().mark_object_state_as_dirty();
  }
};

struct dispRect_1 : RLOpcode<IntConstant_T,
                                IntConstant_T,
                                IntConstant_T,
                                IntConstant_T,
                                IntConstant_T> {
  void operator()(RLMachine& machine, int buf, int x, int y, int w, int h) {
    GraphicsObject& obj = GetGraphicsObject(machine, this, buf);
    obj.SetClipRect(Rect::REC(x, y, w, h));
    machine.system().graphics().mark_object_state_as_dirty();
  }
};

struct dispCorner_1 : RLOpcode<IntConstant_T, IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, int buf, int x, int y) {
    GraphicsObject& obj = GetGraphicsObject(machine, this, buf);
    obj.SetClipRect(Rect::GRP(0, 0, x, y));
    machine.system().graphics().mark_object_state_as_dirty();
  }
};

struct dispOwnArea_0 : public RLOpcode<IntConstant_T> {
  void operator()(RLMachine& machine, int buf) {
    GraphicsObject& obj = GetGraphicsObject(machine, this, buf);
    obj.ClearOwnClipRect();
    machine.system().graphics().mark_object_state_as_dirty();
  }
};

struct dispOwnArea_1 : RLOpcode<IntConstant_T,
                                   IntConstant_T,
                                   IntConstant_T,
                                   IntConstant_T,
                                   IntConstant_T> {
  void operator()(RLMachine& machine, int buf, int x1, int y1, int x2, int y2) {
    GraphicsObject& obj = GetGraphicsObject(machine, this, buf);
    obj.SetOwnClipRect(Rect::GRP(x1, y1, x2, y2));
    machine.system().graphics().mark_object_state_as_dirty();
  }
};

struct dispOwnRect_1 : RLOpcode<IntConstant_T,
                                   IntConstant_T,
                                   IntConstant_T,
                                   IntConstant_T,
                                   IntConstant_T> {
  void operator()(RLMachine& machine, int buf, int x, int y, int w, int h) {
    GraphicsObject& obj = GetGraphicsObject(machine, this, buf);
    obj.SetOwnClipRect(Rect::REC(x, y, w, h));
    machine.system().graphics().mark_object_state_as_dirty();
  }
};

struct adjust
    : RLOpcode<IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, int buf, int idx, int x, int y) {
    GraphicsObject& obj = GetGraphicsObject(machine, this, buf);
    obj.SetXAdjustment(idx, x);
    obj.SetYAdjustment(idx, y);
    machine.system().graphics().mark_object_state_as_dirty();
  }
};

struct tint
    : RLOpcode<IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, int buf, int r, int g, int b) {
    GraphicsObject& obj = GetGraphicsObject(machine, this, buf);
    obj.SetTint(RGBColour(r, g, b));
    machine.system().graphics().mark_object_state_as_dirty();
  }
};

struct colour : RLOpcode<IntConstant_T,
                            IntConstant_T,
                            IntConstant_T,
                            IntConstant_T,
                            IntConstant_T> {
  void operator()(RLMachine& machine, int buf, int r, int g, int b, int level) {
    GraphicsObject& obj = GetGraphicsObject(machine, this, buf);
    obj.SetColour(RGBAColour(r, g, b, level));
    machine.system().graphics().mark_object_state_as_dirty();
  }
};

struct objSetRect_1
    : public RLOpcode<IntConstant_T, Rect_T<rect_impl::GRP>> {
  void operator()(RLMachine& machine, int buf, Rect rect) {
    GraphicsObject& obj = GetGraphicsObject(machine, this, buf);
    if (obj.has_object_data()) {
      ColourFilterObjectData* data =
          dynamic_cast<ColourFilterObjectData*>(&obj.GetObjectData());
      if (data) {
        data->set_rect(rect);
        machine.system().graphics().mark_object_state_as_dirty();
      }
    }
  }
};

struct objSetRect_0 : public RLOpcode<IntConstant_T> {
  void operator()(RLMachine& machine, int buf) {
    Rect rect(0, 0, GetScreenSize(machine.system().gameexe()));
    objSetRect_1()(machine, buf, rect);
  }
};

struct objSetText : public RLOpcode<IntConstant_T, DefaultStrValue_T> {
  void operator()(RLMachine& machine, int buf, string val) {
    GraphicsObject& obj = GetGraphicsObject(machine, this, buf);
    std::string utf8str = cp932toUTF8(val, machine.GetTextEncoding());
    obj.SetTextText(utf8str);
    machine.system().graphics().mark_object_state_as_dirty();
  }
};

// Note: It appears that the RealLive API changed sometime between when Haeleth
// was working on Kanon and RealLive Max. Previously, the zeroth overload took
// the shadow color. Now, the zeroth doesn't and a new first overload does. Use
// DefaultIntValue to try to handle both cases at once.
struct objTextOpts : public RLOpcode<IntConstant_T,
                                     IntConstant_T,
                                     IntConstant_T,
                                     IntConstant_T,
                                     IntConstant_T,
                                     IntConstant_T,
                                     DefaultIntValue_T<-1>> {
  void operator()(RLMachine& machine,
                  int buf,
                  int size,
                  int xspace,
                  int yspace,
                  int char_count,
                  int colour,
                  int shadow) {
    GraphicsObject& obj = GetGraphicsObject(machine, this, buf);
    obj.SetTextOps(size, xspace, yspace, char_count, colour, shadow);
    machine.system().graphics().mark_object_state_as_dirty();
  }
};

struct objDriftOpts : public RLOpcode<IntConstant_T,
                                     IntConstant_T,
                                     IntConstant_T,
                                     IntConstant_T,
                                     IntConstant_T,
                                     IntConstant_T,
                                     IntConstant_T,
                                     IntConstant_T,
                                     IntConstant_T,
                                     IntConstant_T,
                                     IntConstant_T,
                                     IntConstant_T,
                                     Rect_T<rect_impl::GRP>> {
  void operator()(RLMachine& machine,
                  int buf,
                  int count,
                  int use_animation,
                  int start_pattern,
                  int end_pattern,
                  int total_animaton_time_ms,
                  int yspeed,
                  int period,
                  int amplitude,
                  int use_drift,
                  int unknown,
                  int driftspeed,
                  Rect drift_area) {
    GraphicsObject& obj = GetGraphicsObject(machine, this, buf);
    obj.SetDriftOpts(count,
                     use_animation,
                     start_pattern,
                     end_pattern,
                     total_animaton_time_ms,
                     yspeed,
                     period,
                     amplitude,
                     use_drift,
                     unknown,
                     driftspeed,
                     drift_area);
    machine.system().graphics().mark_object_state_as_dirty();
  }
};

struct objNumOpts : public RLOpcode<IntConstant_T,
                                       IntConstant_T,
                                       IntConstant_T,
                                       IntConstant_T,
                                       IntConstant_T,
                                       IntConstant_T> {
  void operator()(RLMachine& machine,
                  int buf,
                  int digits,
                  int zero,
                  int sign,
                  int pack,
                  int space) {
    GraphicsObject& obj = GetGraphicsObject(machine, this, buf);
    obj.SetDigitOpts(digits, zero, sign, pack, space);
    machine.system().graphics().mark_object_state_as_dirty();
  }
};

struct objAdjustAlpha
    : public RLOpcode<IntConstant_T, IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, int buf, int idx, int alpha) {
    GraphicsObject& obj = GetGraphicsObject(machine, this, buf);
    obj.SetAlphaAdjustment(idx, alpha);
    machine.system().graphics().mark_object_state_as_dirty();
  }
};

struct objButtonOpts : public RLOpcode<IntConstant_T,
                                          IntConstant_T,
                                          IntConstant_T,
                                          IntConstant_T,
                                          IntConstant_T> {
  void operator()(RLMachine& machine,
                  int buf,
                  int action,
                  int se,
                  int group,
                  int button_number) {
    GraphicsObject& obj = GetGraphicsObject(machine, this, buf);
    obj.SetButtonOpts(action, se, group, button_number);
    machine.system().graphics().mark_object_state_as_dirty();
  }
};

// -----------------------------------------------------------------------

class objEveAdjust : public RLOpcode<IntConstant_T,
                                        IntConstant_T,
                                        IntConstant_T,
                                        IntConstant_T,
                                        IntConstant_T,
                                        IntConstant_T,
                                        IntConstant_T> {
 public:
  virtual void operator()(RLMachine& machine,
                          int obj,
                          int repno,
                          int x,
                          int y,
                          int duration_time,
                          int delay,
                          int type) {
    unsigned int creation_time = machine.system().event().GetTicks();

    GraphicsObject& object = GetGraphicsObject(machine, this, obj);
    int start_x = object.x_adjustment(repno);
    int start_y = object.y_adjustment(repno);

    object.AddObjectMutator(std::unique_ptr<ObjectMutator>(
        new AdjustMutator(machine,
                          repno,
                          creation_time,
                          duration_time,
                          delay,
                          type,
                          start_x,
                          x,
                          start_y,
                          y)));
  }

 private:
  // We need a custom mutator here. One of the parameters isn't varying.
  class AdjustMutator : public ObjectMutator {
   public:
    AdjustMutator(RLMachine& machine,
                  int repno,
                  int creation_time,
                  int duration_time,
                  int delay,
                  int type,
                  int start_x,
                  int target_x,
                  int start_y,
                  int target_y)
        : ObjectMutator(repno,
                        "objEveAdjust",
                        creation_time,
                        duration_time,
                        delay,
                        type),
          repno_(repno),
          start_x_(start_x),
          end_x_(target_x),
          start_y_(start_y),
          end_y_(target_y) {}

   private:
    virtual void SetToEnd(RLMachine& machine,
                          GraphicsObject& object) override {
      object.SetXAdjustment(repno_, end_x_);
      object.SetYAdjustment(repno_, end_y_);
    }

    virtual ObjectMutator* Clone() const override {
      return new AdjustMutator(*this);
    }

    virtual void PerformSetting(RLMachine& machine,
                                GraphicsObject& object) override {
      int x = GetValueForTime(machine, start_x_, end_x_);
      object.SetXAdjustment(repno_, x);

      int y = GetValueForTime(machine, start_y_, end_y_);
      object.SetYAdjustment(repno_, y);
    }

    int repno_;
    int start_x_;
    int end_x_;
    int start_y_;
    int end_y_;
  };
};

class DisplayMutator : public ObjectMutator {
 public:
  DisplayMutator(RLMachine& machine,
                 GraphicsObject& object,
                 int creation_time,
                 int duration_time,
                 int delay,
                 int display,
                 int dip_event_mod,  // ignored
                 int tr_mod,
                 int move_mod,
                 int move_len_x,
                 int move_len_y,
                 int rotate_mod,
                 int rotate_count,
                 int scale_x_mod,
                 int scale_x_percent,
                 int scale_y_mod,
                 int scale_y_percent,
                 int sin_mod,
                 int sin_len,
                 int sin_count)
      : ObjectMutator(-1,
                      "objEveDisplay",
                      creation_time,
                      duration_time,
                      delay,
                      0),
        display_(display),
        tr_mod_(tr_mod),
        tr_start_(0),
        tr_end_(0),
        move_mod_(move_mod),
        move_start_x_(0),
        move_end_x_(0),
        move_start_y_(0),
        move_end_y_(0),
        rotate_mod_(rotate_mod),
        scale_x_mod_(scale_x_mod),
        scale_y_mod_(scale_y_mod) {
    if (tr_mod_) {
      tr_start_ = display ? 0 : 255;
      tr_end_ = display ? 255 : 0;
    }

    if (move_mod_) {
      // I suspect this isn't right here. If I start at object.x() and end at
      // (object.x() + move_len), then the "Episode" text ends up off the right
      // hand of the screen. However, with this, the downward scrolling text
      // jumps downward when it starts scrolling back up (that didn't used to
      // happen.)
      if (display) {
        move_start_x_ = object.x() - move_len_x;
        move_end_x_ = object.x();
        move_start_y_ = object.y() - move_len_y;
        move_end_y_ = object.y();
      } else {
        move_start_x_ = object.x();
        move_end_x_ = object.x() + move_len_x;
        move_start_y_ = object.y();
        move_end_y_ = object.y() + move_len_y;
      }
    }

    if (rotate_mod_) {
      static bool printed = false;
      if (!printed) {
        std::cerr << "We don't support rotate mod yet." << std::endl;
        printed = true;
      }
    }

    if (scale_x_mod_) {
      static bool printed = false;
      if (!printed) {
        std::cerr << "We don't support scale Y mod yet." << std::endl;
        printed = true;
      }
    }

    if (scale_y_mod_) {
      static bool printed = false;
      if (!printed) {
        std::cerr << "We don't support scale X mod yet." << std::endl;
        printed = true;
      }
    }

    if (sin_mod) {
      static bool printed = false;
      if (!printed) {
        std::cerr << "  We don't support \"sin\" yet." << std::endl;
        printed = true;
      }
    }
  }

 private:
  virtual void SetToEnd(RLMachine& machine, GraphicsObject& object) override {
    object.SetVisible(display_);

    if (tr_mod_)
      object.SetAlpha(tr_end_);

    if (move_mod_) {
      object.SetX(move_end_x_);
      object.SetY(move_end_y_);
    }
  }

  virtual ObjectMutator* Clone() const override {
    return new DisplayMutator(*this);
  }

  virtual void PerformSetting(RLMachine& machine,
                              GraphicsObject& object) override {
    // While performing whatever visual transition, the object should be
    // displayed.
    object.SetVisible(true);

    if (tr_mod_) {
      int alpha = GetValueForTime(machine, tr_start_, tr_end_);
      object.SetAlpha(alpha);
    }

    if (move_mod_) {
      int x = GetValueForTime(machine, move_start_x_, move_end_x_);
      object.SetX(x);

      int y = GetValueForTime(machine, move_start_y_, move_end_y_);
      object.SetY(y);
    }
  }

  bool display_;

  bool tr_mod_;
  int tr_start_;
  int tr_end_;

  bool move_mod_;
  int move_start_x_;
  int move_end_x_;
  int move_start_y_;
  int move_end_y_;

  bool rotate_mod_;
  bool scale_x_mod_;
  bool scale_y_mod_;
};

struct objEveDisplay_1 : public RLOpcode<IntConstant_T,
                                            IntConstant_T,
                                            IntConstant_T,
                                            IntConstant_T,
                                            IntConstant_T> {
  void operator()(RLMachine& machine,
                  int obj,
                  int display,
                  int duration_time,
                  int delay,
                  int param) {
    Gameexe& gameexe = machine.system().gameexe();
    const std::vector<int>& disp = gameexe("OBJDISP", param).ToIntVector();

    GraphicsObject& object = GetGraphicsObject(machine, this, obj);
    unsigned int creation_time = machine.system().event().GetTicks();
    object.AddObjectMutator(std::unique_ptr<ObjectMutator>(
        new DisplayMutator(machine,
                           object,
                           creation_time,
                           duration_time,
                           delay,
                           display,
                           disp.at(0),
                           disp.at(1),
                           disp.at(2),
                           disp.at(3),
                           disp.at(4),
                           disp.at(5),
                           disp.at(6),
                           disp.at(7),
                           disp.at(8),
                           disp.at(9),
                           disp.at(10),
                           disp.at(11),
                           disp.at(12),
                           disp.at(13))));
  }
};

struct objEveDisplay_2 : public RLOpcode<IntConstant_T,
                                            IntConstant_T,
                                            IntConstant_T,
                                            IntConstant_T,
                                            IntConstant_T,
                                            IntConstant_T,
                                            IntConstant_T,
                                            IntConstant_T,
                                            IntConstant_T> {
  void operator()(RLMachine& machine,
                  int obj,
                  int display,
                  int duration_time,
                  int delay,
                  int disp_event_mod,
                  int tr_mod,
                  int move_mod,
                  int move_len_x,
                  int move_len_y) {
    GraphicsObject& object = GetGraphicsObject(machine, this, obj);
    unsigned int creation_time = machine.system().event().GetTicks();
    object.AddObjectMutator(std::unique_ptr<ObjectMutator>(
        new DisplayMutator(machine,
                           object,
                           creation_time,
                           duration_time,
                           delay,
                           display,
                           disp_event_mod,
                           tr_mod,
                           move_mod,
                           move_len_x,
                           move_len_y,
                           0,
                           0,
                           0,
                           0,
                           0,
                           0,
                           0,
                           0,
                           0)));
  }
};

struct objEveDisplay_3 : public RLOpcode<IntConstant_T,
                                        IntConstant_T,
                                        IntConstant_T,
                                        IntConstant_T,
                                        IntConstant_T,
                                        IntConstant_T,
                                        IntConstant_T,
                                        IntConstant_T,
                                        IntConstant_T,
                                        IntConstant_T,
                                        IntConstant_T,
                                        IntConstant_T,
                                        IntConstant_T,
                                        IntConstant_T,
                                        IntConstant_T,
                                        IntConstant_T,
                                        IntConstant_T,
                                        IntConstant_T> {
  void operator()(RLMachine& machine,
                  int obj,
                  int display,
                  int duration_time,
                  int delay,
                  int disp_event_mod,
                  int tr_mod,
                  int move_mod,
                  int move_len_x,
                  int move_len_y,
                  int rotate_mod,
                  int rotate_count,
                  int scale_x_mod,
                  int scale_x_percent,
                  int scale_y_mod,
                  int scale_y_percent,
                  int sin_mod,
                  int sin_len,
                  int sin_count) {
    GraphicsObject& object = GetGraphicsObject(machine, this, obj);
    unsigned int creation_time = machine.system().event().GetTicks();
    object.AddObjectMutator(std::unique_ptr<ObjectMutator>(
        new DisplayMutator(machine,
                           object,
                           creation_time,
                           duration_time,
                           delay,
                           display,
                           disp_event_mod,
                           tr_mod,
                           move_mod,
                           move_len_x,
                           move_len_y,
                           rotate_mod,
                           rotate_count,
                           scale_x_mod,
                           scale_x_percent,
                           scale_y_mod,
                           scale_y_percent,
                           sin_mod,
                           sin_len,
                           sin_count)));
  }
};

// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------

void addUnifiedFunctions(ObjectModule& h) {
  h.AddDoubleObjectCommands(0, "Move",
                            &GraphicsObject::x,
                            &GraphicsObject::SetX,
                            &GraphicsObject::y,
                            &GraphicsObject::SetY);
  h.AddSingleObjectCommands(1, "Left",
                            &GraphicsObject::x,
                            &GraphicsObject::SetX);
  h.AddSingleObjectCommands(2, "Top",
                            &GraphicsObject::y,
                            &GraphicsObject::SetY);
  h.AddSingleObjectCommands(3, "Alpha",
                            &GraphicsObject::raw_alpha,
                            &GraphicsObject::SetAlpha);

  // ----

  h.AddCustomRepno<adjust, objEveAdjust>(6, "Adjust");
  h.AddRepnoObjectCommands(7, "AdjustX",
                           &GraphicsObject::x_adjustment,
                           &GraphicsObject::SetXAdjustment);
  h.AddRepnoObjectCommands(8, "AdjustY",
                           &GraphicsObject::y_adjustment,
                           &GraphicsObject::SetYAdjustment);
  h.AddSingleObjectCommands(9, "Mono",
                            &GraphicsObject::mono,
                            &GraphicsObject::SetMono);
  h.AddSingleObjectCommands(10, "Invert",
                            &GraphicsObject::invert,
                            &GraphicsObject::SetInvert);
  h.AddSingleObjectCommands(11, "Light",
                            &GraphicsObject::light,
                            &GraphicsObject::SetLight);

  // ---

  h.AddSingleObjectCommands(13, "TintR",
                            &GraphicsObject::tint_red,
                            &GraphicsObject::SetTintRed);
  h.AddSingleObjectCommands(14, "TintG",
                            &GraphicsObject::tint_green,
                            &GraphicsObject::SetTintGreen);
  h.AddSingleObjectCommands(15, "TintB",
                            &GraphicsObject::tint_blue,
                            &GraphicsObject::SetTintBlue);

  // ---

  h.AddSingleObjectCommands(17, "ColR",
                            &GraphicsObject::colour_red,
                            &GraphicsObject::SetColourRed);
  h.AddSingleObjectCommands(18, "ColG",
                            &GraphicsObject::colour_green,
                            &GraphicsObject::SetColourGreen);
  h.AddSingleObjectCommands(19, "ColB",
                            &GraphicsObject::colour_blue,
                            &GraphicsObject::SetColourBlue);
  h.AddSingleObjectCommands(20, "ColLevel",
                            &GraphicsObject::colour_level,
                            &GraphicsObject::SetColourLevel);

  // ---

  h.AddSingleObjectCommands(36, "AdjustVert",
                            &GraphicsObject::vert,
                            &GraphicsObject::SetVert);

  h.AddRepnoObjectCommands(40, "AdjustAlpha",
                           &GraphicsObject::alpha_adjustment,
                           &GraphicsObject::SetAlphaAdjustment);

  // --
  h.AddDoubleObjectCommands(46, "Scale",
                            &GraphicsObject::width,
                            &GraphicsObject::SetWidth,
                            &GraphicsObject::height,
                            &GraphicsObject::SetHeight);
  h.AddSingleObjectCommands(47, "Width",
                            &GraphicsObject::width,
                            &GraphicsObject::SetWidth);
  h.AddSingleObjectCommands(48, "Height",
                            &GraphicsObject::height,
                            &GraphicsObject::SetHeight);
  h.AddSingleObjectCommands(49, "Rotate",
                            &GraphicsObject::rotation,
                            &GraphicsObject::SetRotation);
  h.AddDoubleObjectCommands(50, "RepOrigin",
                            &GraphicsObject::rep_origin_x,
                            &GraphicsObject::SetRepOriginX,
                            &GraphicsObject::rep_origin_y,
                            &GraphicsObject::SetRepOriginY);
  h.AddSingleObjectCommands(51, "RepOriginX",
                            &GraphicsObject::rep_origin_x,
                            &GraphicsObject::SetRepOriginX);
  h.AddSingleObjectCommands(52, "RepOriginY",
                            &GraphicsObject::rep_origin_y,
                            &GraphicsObject::SetRepOriginY);
  h.AddDoubleObjectCommands(53, "Origin",
                            &GraphicsObject::origin_x,
                            &GraphicsObject::SetOriginX,
                            &GraphicsObject::origin_y,
                            &GraphicsObject::SetOriginY);
  h.AddSingleObjectCommands(54, "OriginX",
                            &GraphicsObject::origin_x,
                            &GraphicsObject::SetOriginX);
  h.AddSingleObjectCommands(55, "OriginY",
                            &GraphicsObject::origin_y,
                            &GraphicsObject::SetOriginY);

  // ---

  h.AddDoubleObjectCommands(61, "HqScale",
                            &GraphicsObject::hq_width,
                            &GraphicsObject::SetHqWidth,
                            &GraphicsObject::hq_height,
                            &GraphicsObject::SetHqHeight);
  h.AddSingleObjectCommands(62, "HqWidth",
                            &GraphicsObject::hq_width,
                            &GraphicsObject::SetHqWidth);
  h.AddSingleObjectCommands(63, "HqHeight",
                            &GraphicsObject::hq_height,
                            &GraphicsObject::SetHqHeight);
}

void addObjectFunctions(RLModule& m) {
  m.AddOpcode(
      1004, 0, "objShow", new Obj_SetOneIntOnObj(&GraphicsObject::SetVisible));
  m.AddOpcode(1005, 0, "objDispArea", new dispArea_0);
  m.AddOpcode(1005, 1, "objDispArea", new dispArea_1);

  m.AddOpcode(1012, 0, "objTint", new tint);

  m.AddOpcode(1016, 0, "objColour", new colour);

  m.AddOpcode(1021,
              0,
              "objComposite",
              new Obj_SetOneIntOnObj(&GraphicsObject::SetCompositeMode));

  m.AddOpcode(1022, 0, "objSetRect", new objSetRect_0);
  m.AddOpcode(1022, 1, "objSetRect", new objSetRect_1);

  m.AddOpcode(1024, 0, "objSetText", new objSetText);
  m.AddOpcode(1024, 1, "objSetText", new objSetText);
  m.AddOpcode(1025, 0, "objTextOpts", new objTextOpts);
  m.AddOpcode(1025, 1, "objTextOpts", new objTextOpts);

  m.AddOpcode(
      1026, 0, "objLayer", new Obj_SetOneIntOnObj(&GraphicsObject::SetZLayer));
  m.AddOpcode(
      1027, 0, "objDepth", new Obj_SetOneIntOnObj(&GraphicsObject::SetZDepth));
  m.AddUnsupportedOpcode(1028, 0, "objScrollRate");
  m.AddOpcode(1029,
              0,
              "objScrollRateX",
              new Obj_SetOneIntOnObj(&GraphicsObject::SetScrollRateX));
  m.AddOpcode(1030,
              0,
              "objScrollRateY",
              new Obj_SetOneIntOnObj(&GraphicsObject::SetScrollRateY));
  m.AddOpcode(1031, 0, "objDriftOpts", new objDriftOpts);
  m.AddOpcode(
      1032, 0, "objOrder", new Obj_SetOneIntOnObj(&GraphicsObject::SetZOrder));
  m.AddUnsupportedOpcode(1033, 0, "objQuarterView");

  m.AddOpcode(1034, 0, "objDispRect", new dispArea_0);
  m.AddOpcode(1034, 1, "objDispRect", new dispRect_1);
  m.AddOpcode(1035, 0, "objDispCorner", new dispArea_0);
  m.AddOpcode(1035, 1, "objDispCorner", new dispArea_1);
  m.AddOpcode(1035, 2, "objDispCorner", new dispCorner_1);

  m.AddOpcode(1037,
              0,
              "objSetDigits",
              new Obj_SetOneIntOnObj(&GraphicsObject::SetDigitValue));
  m.AddOpcode(1038, 0, "objNumOpts", new objNumOpts);
  m.AddOpcode(
      1039, 0, "objPattNo", new Obj_SetOneIntOnObj(&GraphicsObject::SetPattNo));

  m.AddUnsupportedOpcode(1041, 0, "objAdjustAll");
  m.AddUnsupportedOpcode(1042, 0, "objAdjustAllX");
  m.AddUnsupportedOpcode(1043, 0, "objAdjustAllY");

  m.AddUnsupportedOpcode(1056, 0, "objFadeOpts");

  m.AddOpcode(1064, 2, "objButtonOpts", new objButtonOpts);
  m.AddOpcode(1066,
              0,
              "objBtnState",
              new Obj_SetOneIntOnObj(&GraphicsObject::SetButtonState));

  m.AddOpcode(1070, 0, "objOwnDispArea", new dispOwnArea_0);
  m.AddOpcode(1070, 1, "objOwnDispArea", new dispOwnArea_1);
  m.AddOpcode(1071, 0, "objOwnDispRect", new dispOwnArea_0);
  m.AddOpcode(1071, 1, "objOwnDispRect", new dispOwnRect_1);
}

void addEveObjectFunctions(RLModule& m) {
  m.AddOpcode(2004,
              0,
              "objEveDisplay",
              new Obj_SetOneIntOnObj(&GraphicsObject::SetVisible));
  m.AddOpcode(2004, 1, "objEveDisplay", new objEveDisplay_1);
  m.AddOpcode(2004, 2, "objEveDisplay", new objEveDisplay_2);
  m.AddOpcode(2004, 3, "objEveDisplay", new objEveDisplay_3);

  m.AddOpcode(
      3004, 0, "objEveDisplayCheck", new Op_MutatorCheck("objEveDisplay"));

  m.AddOpcode(
      4004, 0, "objEveDisplayWait", new Op_MutatorWaitNormal("objEveDisplay"));

  m.AddOpcode(5004,
              0,
              "objEveDisplayWaitC",
              new Op_MutatorWaitCNormal("objEveDisplay"));

  m.AddOpcode(6004,
              0,
              "objEveDisplayEnd",
              new Op_EndObjectMutation_Normal("objEveDisplay"));
}

}  // namespace

// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------

ObjFgModule::ObjFgModule()
    : RLModule("ObjFg", 1, 81),
      helper_("obj", this) {
  addUnifiedFunctions(helper_);

  addObjectFunctions(*this);
  addEveObjectFunctions(*this);
  SetProperty(P_FGBG, OBJ_FG);
}

// -----------------------------------------------------------------------

ObjBgModule::ObjBgModule()
    : RLModule("ObjBg", 1, 82),
      helper_("objBg", this) {
  addUnifiedFunctions(helper_);

  addObjectFunctions(*this);
  addEveObjectFunctions(*this);
  SetProperty(P_FGBG, OBJ_BG);
}

// -----------------------------------------------------------------------

ChildObjFgModule::ChildObjFgModule()
    : MappedRLModule(ChildObjMappingFun, "ChildObjFg", 2, 81),
      helper_("objChild", this) {
  addUnifiedFunctions(helper_);

  addObjectFunctions(*this);
  addEveObjectFunctions(*this);
  SetProperty(P_FGBG, OBJ_FG);
}

// -----------------------------------------------------------------------

ChildObjBgModule::ChildObjBgModule()
    : MappedRLModule(ChildObjMappingFun, "ChildObjBg", 2, 82),
      helper_("objChildBg", this) {
  addUnifiedFunctions(helper_);

  addObjectFunctions(*this);
  addEveObjectFunctions(*this);
  SetProperty(P_FGBG, OBJ_BG);
}

// -----------------------------------------------------------------------

ObjRangeFgModule::ObjRangeFgModule()
    : MappedRLModule(RangeMappingFun, "ObjRangeFg", 1, 90),
      helper_("objRange", this) {
  addUnifiedFunctions(helper_);

  addObjectFunctions(*this);
  SetProperty(P_FGBG, OBJ_FG);
}

// -----------------------------------------------------------------------

ObjRangeBgModule::ObjRangeBgModule()
    : MappedRLModule(RangeMappingFun, "ObjRangeBg", 1, 91),
      helper_("objRangeBg", this) {
  addUnifiedFunctions(helper_);

  addObjectFunctions(*this);
  SetProperty(P_FGBG, OBJ_BG);
}

// -----------------------------------------------------------------------

ChildObjRangeFgModule::ChildObjRangeFgModule()
    : MappedRLModule(ChildRangeMappingFun, "ObjChildRangeFg", 2, 90),
      helper_("objChildRange", this) {
  addUnifiedFunctions(helper_);

  addObjectFunctions(*this);
  SetProperty(P_FGBG, OBJ_FG);
}

// -----------------------------------------------------------------------

ChildObjRangeBgModule::ChildObjRangeBgModule()
    : MappedRLModule(ChildRangeMappingFun, "ObjChildRangeBg", 2, 91),
      helper_("objChildRangeBg", this) {
  addUnifiedFunctions(helper_);

  addObjectFunctions(*this);
  SetProperty(P_FGBG, OBJ_BG);
}
