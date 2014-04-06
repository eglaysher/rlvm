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

#include <boost/shared_ptr.hpp>
#include <cmath>
#include <functional>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "libreallive/bytecode.h"
#include "libreallive/gameexe.h"
#include "long_operations/wait_long_operation.h"
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

struct dispArea_0 : public RLOp_Void_1<IntConstant_T> {
  void operator()(RLMachine& machine, int buf) {
    GraphicsObject& obj = getGraphicsObject(machine, this, buf);
    obj.clearClip();
    machine.system().graphics().markObjectStateAsDirty();
  }
};

struct dispArea_1 : RLOp_Void_5<IntConstant_T,
                                IntConstant_T,
                                IntConstant_T,
                                IntConstant_T,
                                IntConstant_T> {
  void operator()(RLMachine& machine, int buf, int x1, int y1, int x2, int y2) {
    GraphicsObject& obj = getGraphicsObject(machine, this, buf);
    obj.setClip(Rect::GRP(x1, y1, x2, y2));
    machine.system().graphics().markObjectStateAsDirty();
  }
};

struct dispRect_1 : RLOp_Void_5<IntConstant_T,
                                IntConstant_T,
                                IntConstant_T,
                                IntConstant_T,
                                IntConstant_T> {
  void operator()(RLMachine& machine, int buf, int x, int y, int w, int h) {
    GraphicsObject& obj = getGraphicsObject(machine, this, buf);
    obj.setClip(Rect::REC(x, y, w, h));
    machine.system().graphics().markObjectStateAsDirty();
  }
};

struct dispCorner_1 : RLOp_Void_3<IntConstant_T, IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, int buf, int x, int y) {
    GraphicsObject& obj = getGraphicsObject(machine, this, buf);
    obj.setClip(Rect::GRP(0, 0, x, y));
    machine.system().graphics().markObjectStateAsDirty();
  }
};

struct dispOwnArea_0 : public RLOp_Void_1<IntConstant_T> {
  void operator()(RLMachine& machine, int buf) {
    GraphicsObject& obj = getGraphicsObject(machine, this, buf);
    obj.clearOwnClip();
    machine.system().graphics().markObjectStateAsDirty();
  }
};

struct dispOwnArea_1 : RLOp_Void_5<IntConstant_T,
                                   IntConstant_T,
                                   IntConstant_T,
                                   IntConstant_T,
                                   IntConstant_T> {
  void operator()(RLMachine& machine, int buf, int x1, int y1, int x2, int y2) {
    GraphicsObject& obj = getGraphicsObject(machine, this, buf);
    obj.setOwnClip(Rect::GRP(x1, y1, x2, y2));
    machine.system().graphics().markObjectStateAsDirty();
  }
};

struct dispOwnRect_1 : RLOp_Void_5<IntConstant_T,
                                   IntConstant_T,
                                   IntConstant_T,
                                   IntConstant_T,
                                   IntConstant_T> {
  void operator()(RLMachine& machine, int buf, int x, int y, int w, int h) {
    GraphicsObject& obj = getGraphicsObject(machine, this, buf);
    obj.setOwnClip(Rect::REC(x, y, w, h));
    machine.system().graphics().markObjectStateAsDirty();
  }
};

struct adjust
    : RLOp_Void_4<IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, int buf, int idx, int x, int y) {
    GraphicsObject& obj = getGraphicsObject(machine, this, buf);
    obj.setXAdjustment(idx, x);
    obj.setYAdjustment(idx, y);
    machine.system().graphics().markObjectStateAsDirty();
  }
};

struct adjustX : RLOp_Void_3<IntConstant_T, IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, int buf, int idx, int x) {
    GraphicsObject& obj = getGraphicsObject(machine, this, buf);
    obj.setXAdjustment(idx, x);
    machine.system().graphics().markObjectStateAsDirty();
  }
};

struct adjustY : RLOp_Void_3<IntConstant_T, IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, int buf, int idx, int y) {
    GraphicsObject& obj = getGraphicsObject(machine, this, buf);
    obj.setYAdjustment(idx, y);
    machine.system().graphics().markObjectStateAsDirty();
  }
};

struct tint
    : RLOp_Void_4<IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, int buf, int r, int g, int b) {
    GraphicsObject& obj = getGraphicsObject(machine, this, buf);
    obj.setTint(RGBColour(r, g, b));
    machine.system().graphics().markObjectStateAsDirty();
  }
};

struct colour : RLOp_Void_5<IntConstant_T,
                            IntConstant_T,
                            IntConstant_T,
                            IntConstant_T,
                            IntConstant_T> {
  void operator()(RLMachine& machine, int buf, int r, int g, int b, int level) {
    GraphicsObject& obj = getGraphicsObject(machine, this, buf);
    obj.setColour(RGBAColour(r, g, b, level));
    machine.system().graphics().markObjectStateAsDirty();
  }
};

struct objSetRect_1
    : public RLOp_Void_2<IntConstant_T, Rect_T<rect_impl::GRP>> {
  void operator()(RLMachine& machine, int buf, Rect rect) {
    GraphicsObject& obj = getGraphicsObject(machine, this, buf);
    if (obj.hasObjectData()) {
      ColourFilterObjectData* data =
          dynamic_cast<ColourFilterObjectData*>(&obj.objectData());
      if (data) {
        data->setRect(rect);
        machine.system().graphics().markObjectStateAsDirty();
      }
    }
  }
};

struct objSetRect_0 : public RLOp_Void_1<IntConstant_T> {
  void operator()(RLMachine& machine, int buf) {
    Rect rect(0, 0, getScreenSize(machine.system().gameexe()));
    objSetRect_1()(machine, buf, rect);
  }
};

struct objSetText : public RLOp_Void_2<IntConstant_T, DefaultStrValue_T> {
  void operator()(RLMachine& machine, int buf, string val) {
    GraphicsObject& obj = getGraphicsObject(machine, this, buf);
    std::string utf8str = cp932toUTF8(val, machine.GetTextEncoding());
    obj.setTextText(utf8str);
    machine.system().graphics().markObjectStateAsDirty();
  }
};

struct objTextOpts : public RLOp_Void_7<IntConstant_T,
                                        IntConstant_T,
                                        IntConstant_T,
                                        IntConstant_T,
                                        IntConstant_T,
                                        IntConstant_T,
                                        IntConstant_T> {
  void operator()(RLMachine& machine,
                  int buf,
                  int size,
                  int xspace,
                  int yspace,
                  int char_count,
                  int colour,
                  int shadow) {
    GraphicsObject& obj = getGraphicsObject(machine, this, buf);
    obj.setTextOps(size, xspace, yspace, char_count, colour, shadow);
    machine.system().graphics().markObjectStateAsDirty();
  }
};

struct objDriftOpts : public RLOp_Void_13<IntConstant_T,
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
    GraphicsObject& obj = getGraphicsObject(machine, this, buf);
    obj.setDriftOpts(count,
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
    machine.system().graphics().markObjectStateAsDirty();
  }
};

struct objNumOpts : public RLOp_Void_6<IntConstant_T,
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
    GraphicsObject& obj = getGraphicsObject(machine, this, buf);
    obj.setDigitOpts(digits, zero, sign, pack, space);
    machine.system().graphics().markObjectStateAsDirty();
  }
};

struct objAdjustAlpha
    : public RLOp_Void_3<IntConstant_T, IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, int buf, int idx, int alpha) {
    GraphicsObject& obj = getGraphicsObject(machine, this, buf);
    obj.setAlphaAdjustment(idx, alpha);
    machine.system().graphics().markObjectStateAsDirty();
  }
};

struct objButtonOpts : public RLOp_Void_5<IntConstant_T,
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
    GraphicsObject& obj = getGraphicsObject(machine, this, buf);
    obj.setButtonOpts(action, se, group, button_number);
    machine.system().graphics().markObjectStateAsDirty();
  }
};

// -----------------------------------------------------------------------

class objEveAdjust : public RLOp_Void_7<IntConstant_T,
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
    unsigned int creation_time = machine.system().event().getTicks();

    GraphicsObject& object = getGraphicsObject(machine, this, obj);
    int start_x = object.xAdjustment(repno);
    int start_y = object.yAdjustment(repno);

    object.AddObjectMutator(new AdjustMutator(machine,
                                              repno,
                                              creation_time,
                                              duration_time,
                                              delay,
                                              type,
                                              start_x,
                                              x,
                                              start_y,
                                              y));
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
    virtual void SetToEnd(RLMachine& machine, GraphicsObject& object) {
      object.setXAdjustment(repno_, end_x_);
      object.setYAdjustment(repno_, end_y_);
    }

    virtual void PerformSetting(RLMachine& machine, GraphicsObject& object) {
      int x = GetValueForTime(machine, start_x_, end_x_);
      object.setXAdjustment(repno_, x);

      int y = GetValueForTime(machine, start_y_, end_y_);
      object.setYAdjustment(repno_, y);
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
  virtual void SetToEnd(RLMachine& machine, GraphicsObject& object) {
    object.setVisible(display_);

    if (tr_mod_)
      object.setAlpha(tr_end_);

    if (move_mod_) {
      object.setX(move_end_x_);
      object.setY(move_end_y_);
    }
  }

  virtual void PerformSetting(RLMachine& machine, GraphicsObject& object) {
    // While performing whatever visual transition, the object should be
    // displayed.
    object.setVisible(true);

    if (tr_mod_) {
      int alpha = GetValueForTime(machine, tr_start_, tr_end_);
      object.setAlpha(alpha);
    }

    if (move_mod_) {
      int x = GetValueForTime(machine, move_start_x_, move_end_x_);
      object.setX(x);

      int y = GetValueForTime(machine, move_start_y_, move_end_y_);
      object.setY(y);
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

struct objEveDisplay_1 : public RLOp_Void_5<IntConstant_T,
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

    GraphicsObject& object = getGraphicsObject(machine, this, obj);
    unsigned int creation_time = machine.system().event().getTicks();
    object.AddObjectMutator(new DisplayMutator(machine,
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
                                               disp.at(13)));
  }
};

struct objEveDisplay_2 : public RLOp_Void_9<IntConstant_T,
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
    GraphicsObject& object = getGraphicsObject(machine, this, obj);
    unsigned int creation_time = machine.system().event().getTicks();
    object.AddObjectMutator(new DisplayMutator(machine,
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
                                               0));
  }
};

struct objEveDisplay_3 : public RLOp_Void_18<IntConstant_T,
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
    GraphicsObject& object = getGraphicsObject(machine, this, obj);
    unsigned int creation_time = machine.system().event().getTicks();
    object.AddObjectMutator(new DisplayMutator(machine,
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
                                               sin_count));
  }
};

bool MutatorIsDone(RLMachine& machine,
                   RLOperation* op,
                   int obj,
                   int repno,
                   const char* name) {
  return getGraphicsObject(machine, op, obj)
             .IsMutatorRunningMatching(repno, name) == false;
}

class Op_MutatorWaitNormal : public RLOp_Void_1<IntConstant_T> {
 public:
  explicit Op_MutatorWaitNormal(const char* name) : name_(name) {}

  virtual void operator()(RLMachine& machine, int obj) {
    WaitLongOperation* wait_op = new WaitLongOperation(machine);
    wait_op->BreakOnEvent(
        std::bind(MutatorIsDone, std::ref(machine), this, obj, -1, name_));
    machine.PushLongOperation(wait_op);
  }

 private:
  const char* name_;
};

class Op_MutatorWaitRepNo : public RLOp_Void_2<IntConstant_T, IntConstant_T> {
 public:
  explicit Op_MutatorWaitRepNo(const char* name) : name_(name) {}

  virtual void operator()(RLMachine& machine, int obj, int repno) {
    WaitLongOperation* wait_op = new WaitLongOperation(machine);
    wait_op->BreakOnEvent(
        std::bind(MutatorIsDone, std::ref(machine), this, obj, repno, name_));
    machine.PushLongOperation(wait_op);
  }

 private:
  const char* name_;
};

bool objectMutatorIsWorking(RLMachine& machine,
                            RLOperation* op,
                            int obj,
                            int repno,
                            const char* name) {
  return getGraphicsObject(machine, op, obj)
             .IsMutatorRunningMatching(repno, name) == false;
}

class Op_MutatorWaitCNormal : public RLOp_Void_1<IntConstant_T> {
 public:
  explicit Op_MutatorWaitCNormal(const char* name) : name_(name) {}

  virtual void operator()(RLMachine& machine, int obj) {
    WaitLongOperation* wait_op = new WaitLongOperation(machine);
    wait_op->BreakOnClicks();
    wait_op->BreakOnEvent(std::bind(
        objectMutatorIsWorking, std::ref(machine), this, obj, -1, name_));
    machine.PushLongOperation(wait_op);
  }

 private:
  const char* name_;
};

class Op_MutatorWaitCRepNo : public RLOp_Void_2<IntConstant_T, IntConstant_T> {
 public:
  explicit Op_MutatorWaitCRepNo(const char* name) : name_(name) {}

  virtual void operator()(RLMachine& machine, int obj, int repno) {
    WaitLongOperation* wait_op = new WaitLongOperation(machine);
    wait_op->BreakOnClicks();
    wait_op->BreakOnEvent(std::bind(
        objectMutatorIsWorking, std::ref(machine), this, obj, repno, name_));
    machine.PushLongOperation(wait_op);
  }

 private:
  const char* name_;
};

// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------

void addObjectFunctions(RLModule& m) {
  m.AddOpcode(
      1000,
      0,
      "objMove",
      new Obj_SetTwoIntOnObj(&GraphicsObject::setX, &GraphicsObject::setY));
  m.AddOpcode(
      1001, 0, "objLeft", new Obj_SetOneIntOnObj(&GraphicsObject::setX));
  m.AddOpcode(1002, 0, "objTop", new Obj_SetOneIntOnObj(&GraphicsObject::setY));
  m.AddOpcode(
      1003, 0, "objAlpha", new Obj_SetOneIntOnObj(&GraphicsObject::setAlpha));
  m.AddOpcode(
      1004, 0, "objShow", new Obj_SetOneIntOnObj(&GraphicsObject::setVisible));
  m.AddOpcode(1005, 0, "objDispArea", new dispArea_0);
  m.AddOpcode(1005, 1, "objDispArea", new dispArea_1);
  m.AddOpcode(1006, 0, "objAdjust", new adjust);
  m.AddOpcode(1007, 0, "objAdjustX", new adjustX);
  m.AddOpcode(1008, 0, "objAdjustY", new adjustY);
  m.AddOpcode(
      1009, 0, "objMono", new Obj_SetOneIntOnObj(&GraphicsObject::setMono));
  m.AddOpcode(
      1010, 0, "objInvert", new Obj_SetOneIntOnObj(&GraphicsObject::setInvert));
  m.AddOpcode(
      1011, 0, "objLight", new Obj_SetOneIntOnObj(&GraphicsObject::setLight));
  m.AddOpcode(1012, 0, "objTint", new tint);
  m.AddOpcode(
      1013, 0, "objTintR", new Obj_SetOneIntOnObj(&GraphicsObject::setTintR));
  m.AddOpcode(
      1014, 0, "objTintG", new Obj_SetOneIntOnObj(&GraphicsObject::setTintG));
  m.AddOpcode(
      1015, 0, "objTintB", new Obj_SetOneIntOnObj(&GraphicsObject::setTintB));
  m.AddOpcode(1016, 0, "objColour", new colour);
  m.AddOpcode(
      1017, 0, "objColR", new Obj_SetOneIntOnObj(&GraphicsObject::setColourR));
  m.AddOpcode(
      1018, 0, "objColG", new Obj_SetOneIntOnObj(&GraphicsObject::setColourG));
  m.AddOpcode(
      1019, 0, "objColB", new Obj_SetOneIntOnObj(&GraphicsObject::setColourB));
  m.AddOpcode(1020,
              0,
              "objColLevel",
              new Obj_SetOneIntOnObj(&GraphicsObject::setColourLevel));
  m.AddOpcode(1021,
              0,
              "objComposite",
              new Obj_SetOneIntOnObj(&GraphicsObject::setCompositeMode));

  m.AddOpcode(1022, 0, "objSetRect", new objSetRect_0);
  m.AddOpcode(1022, 1, "objSetRect", new objSetRect_1);

  m.AddOpcode(1024, 0, "objSetText", new objSetText);
  m.AddOpcode(1024, 1, "objSetText", new objSetText);
  m.AddOpcode(1025, 0, "objTextOpts", new objTextOpts);

  m.AddOpcode(
      1026, 0, "objLayer", new Obj_SetOneIntOnObj(&GraphicsObject::setZLayer));
  m.AddOpcode(
      1027, 0, "objDepth", new Obj_SetOneIntOnObj(&GraphicsObject::setZDepth));
  m.AddUnsupportedOpcode(1028, 0, "objScrollRate");
  m.AddOpcode(1029,
              0,
              "objScrollRateX",
              new Obj_SetOneIntOnObj(&GraphicsObject::setScrollRateX));
  m.AddOpcode(1030,
              0,
              "objScrollRateY",
              new Obj_SetOneIntOnObj(&GraphicsObject::setScrollRateY));
  m.AddOpcode(1031, 0, "objDriftOpts", new objDriftOpts);
  m.AddOpcode(
      1032, 0, "objOrder", new Obj_SetOneIntOnObj(&GraphicsObject::setZOrder));
  m.AddUnsupportedOpcode(1033, 0, "objQuarterView");

  m.AddOpcode(1034, 0, "objDispRect", new dispArea_0);
  m.AddOpcode(1034, 1, "objDispRect", new dispRect_1);
  m.AddOpcode(1035, 0, "objDispCorner", new dispArea_0);
  m.AddOpcode(1035, 1, "objDispCorner", new dispArea_1);
  m.AddOpcode(1035, 2, "objDispCorner", new dispCorner_1);
  m.AddOpcode(1036,
              0,
              "objAdjustVert",
              new Obj_SetOneIntOnObj(&GraphicsObject::setVert));
  m.AddOpcode(1037,
              0,
              "objSetDigits",
              new Obj_SetOneIntOnObj(&GraphicsObject::setDigitValue));
  m.AddOpcode(1038, 0, "objNumOpts", new objNumOpts);
  m.AddOpcode(
      1039, 0, "objPattNo", new Obj_SetOneIntOnObj(&GraphicsObject::setPattNo));

  m.AddOpcode(1040, 0, "objAdjustAlpha", new objAdjustAlpha);
  m.AddUnsupportedOpcode(1041, 0, "objAdjustAll");
  m.AddUnsupportedOpcode(1042, 0, "objAdjustAllX");
  m.AddUnsupportedOpcode(1043, 0, "objAdjustAllY");

  m.AddOpcode(1046,
              0,
              "objScale",
              new Obj_SetTwoIntOnObj(&GraphicsObject::setWidth,
                                     &GraphicsObject::setHeight));
  m.AddOpcode(
      1047, 0, "objWidth", new Obj_SetOneIntOnObj(&GraphicsObject::setWidth));
  m.AddOpcode(
      1048, 0, "objHeight", new Obj_SetOneIntOnObj(&GraphicsObject::setHeight));
  m.AddOpcode(1049,
              0,
              "objRotate",
              new Obj_SetOneIntOnObj(&GraphicsObject::setRotation));

  m.AddOpcode(1050,
              0,
              "objRepOrigin",
              new Obj_SetTwoIntOnObj(&GraphicsObject::setXRepOrigin,
                                     &GraphicsObject::setYRepOrigin));
  m.AddOpcode(1051,
              0,
              "objRepOriginX",
              new Obj_SetOneIntOnObj(&GraphicsObject::setXRepOrigin));
  m.AddOpcode(1052,
              0,
              "objRepOriginY",
              new Obj_SetOneIntOnObj(&GraphicsObject::setYRepOrigin));
  m.AddOpcode(1053,
              0,
              "objOrigin",
              new Obj_SetTwoIntOnObj(&GraphicsObject::setXOrigin,
                                     &GraphicsObject::setYOrigin));
  m.AddOpcode(1054,
              0,
              "objOriginX",
              new Obj_SetOneIntOnObj(&GraphicsObject::setXOrigin));
  m.AddOpcode(1055,
              0,
              "objOriginY",
              new Obj_SetOneIntOnObj(&GraphicsObject::setYOrigin));
  m.AddUnsupportedOpcode(1056, 0, "objFadeOpts");

  m.AddOpcode(1061,
              0,
              "objHqScale",
              new Obj_SetTwoIntOnObj(&GraphicsObject::setHqWidth,
                                     &GraphicsObject::setHqHeight));
  m.AddOpcode(1062,
              0,
              "objHqWidth",
              new Obj_SetOneIntOnObj(&GraphicsObject::setHqWidth));
  m.AddOpcode(1063,
              0,
              "objHqHeight",
              new Obj_SetOneIntOnObj(&GraphicsObject::setHqHeight));

  m.AddOpcode(1064, 2, "objButtonOpts", new objButtonOpts);
  m.AddOpcode(1066,
              0,
              "objBtnState",
              new Obj_SetOneIntOnObj(&GraphicsObject::setButtonState));

  m.AddOpcode(1070, 0, "objOwnDispArea", new dispOwnArea_0);
  m.AddOpcode(1070, 1, "objOwnDispArea", new dispOwnArea_1);
  m.AddOpcode(1071, 0, "objOwnDispRect", new dispOwnArea_0);
  m.AddOpcode(1071, 1, "objOwnDispRect", new dispOwnRect_1);
}

void addEveObjectFunctions(RLModule& m) {
  m.AddOpcode(
      2000,
      0,
      "objEveMove",
      new Obj_SetTwoIntOnObj(&GraphicsObject::setX, &GraphicsObject::setY));
  m.AddOpcode(2000,
              1,
              "objEveMove",
              new Op_ObjectMutatorIntInt(&GraphicsObject::x,
                                         &GraphicsObject::setX,
                                         &GraphicsObject::y,
                                         &GraphicsObject::setY,
                                         "objEveMove"));

  m.AddOpcode(
      2001, 0, "objEveLeft", new Obj_SetOneIntOnObj(&GraphicsObject::setX));
  m.AddOpcode(2001,
              1,
              "objEveLeft",
              new Op_ObjectMutatorInt(
                  &GraphicsObject::x, &GraphicsObject::setX, "objEveLeft"));

  m.AddOpcode(
      2002, 0, "objEveTop", new Obj_SetOneIntOnObj(&GraphicsObject::setY));
  m.AddOpcode(2002,
              1,
              "objEveTop",
              new Op_ObjectMutatorInt(
                  &GraphicsObject::y, &GraphicsObject::setY, "objEveTop"));

  m.AddOpcode(2003,
              0,
              "objEveAlpha",
              new Obj_SetOneIntOnObj(&GraphicsObject::setAlpha));
  m.AddOpcode(
      2003,
      1,
      "objEveAlpha",
      new Op_ObjectMutatorInt(
          &GraphicsObject::rawAlpha, &GraphicsObject::setAlpha, "objEveAlpha"));

  m.AddOpcode(2004,
              0,
              "objEveDisplay",
              new Obj_SetOneIntOnObj(&GraphicsObject::setVisible));
  m.AddOpcode(2004, 1, "objEveDisplay", new objEveDisplay_1);
  m.AddOpcode(2004, 2, "objEveDisplay", new objEveDisplay_2);
  m.AddOpcode(2004, 3, "objEveDisplay", new objEveDisplay_3);

  m.AddOpcode(2006, 0, "objEveAdjust", new adjust);
  m.AddOpcode(2006, 1, "objEveAdjust", new objEveAdjust);

  m.AddOpcode(2007, 0, "objEveAdjustX", new adjustX);
  m.AddOpcode(2007,
              1,
              "objEveAdjustX",
              new Op_ObjectMutatorRepnoInt(&GraphicsObject::xAdjustment,
                                           &GraphicsObject::setXAdjustment,
                                           "objEveAdjustX"));

  m.AddOpcode(2008, 0, "objEveAdjustY", new adjustY);
  m.AddOpcode(2008,
              1,
              "objEveAdjustY",
              new Op_ObjectMutatorRepnoInt(&GraphicsObject::yAdjustment,
                                           &GraphicsObject::setYAdjustment,
                                           "objEveAdjustY"));

  m.AddOpcode(2040, 0, "objEveAdjustAlpha", new objAdjustAlpha);
  m.AddOpcode(2040,
              1,
              "objEveAdjustAlpha",
              new Op_ObjectMutatorRepnoInt(&GraphicsObject::alphaAdjustment,
                                           &GraphicsObject::setAlphaAdjustment,
                                           "objEveAdjustAlpha"));

  m.AddOpcode(2046,
              0,
              "objEveScale",
              new Obj_SetTwoIntOnObj(&GraphicsObject::setWidth,
                                     &GraphicsObject::setHeight));
  m.AddOpcode(2046,
              1,
              "objEveScale",
              new Op_ObjectMutatorIntInt(&GraphicsObject::width,
                                         &GraphicsObject::setWidth,
                                         &GraphicsObject::height,
                                         &GraphicsObject::setHeight,
                                         "objEveScale"));

  m.AddOpcode(
      4000, 0, "objEveMoveWait", new Op_MutatorWaitNormal("objEveMove"));
  m.AddOpcode(
      4001, 0, "objEveLeftWait", new Op_MutatorWaitNormal("objEveLeft"));
  m.AddOpcode(4002, 0, "objEveTopWait", new Op_MutatorWaitNormal("objEveTop"));
  m.AddOpcode(
      4003, 0, "objEveAlphaWait", new Op_MutatorWaitNormal("objEveAlpha"));
  m.AddOpcode(
      4004, 0, "objEveDisplayWait", new Op_MutatorWaitNormal("objEveDisplay"));
  m.AddOpcode(
      4006, 0, "objEveAdjustEnd", new Op_MutatorWaitRepNo("objEveAdjust"));
  m.AddOpcode(4040,
              0,
              "objEveAdjustAlpha",
              new Op_MutatorWaitRepNo("objEveAdjustAlpha"));

  m.AddOpcode(
      5000, 0, "objEveMoveWaitC", new Op_MutatorWaitCNormal("objEveMove"));
  m.AddOpcode(
      5001, 0, "objEveLeftWaitC", new Op_MutatorWaitCNormal("objEveLeft"));
  m.AddOpcode(
      5002, 0, "objEveTopWaitC", new Op_MutatorWaitCNormal("objEveTop"));
  m.AddOpcode(
      5003, 0, "objEveAlphaWaitC", new Op_MutatorWaitCNormal("objEveAlpha"));
  m.AddOpcode(5004,
              0,
              "objEveDisplayWaitC",
              new Op_MutatorWaitCNormal("objEveDisplay"));
  m.AddOpcode(
      5006, 0, "objEveAdjustWaitC", new Op_MutatorWaitCRepNo("objEveAdjust"));
  m.AddOpcode(5040,
              0,
              "objEveAdjustAlphaWaitC",
              new Op_MutatorWaitCRepNo("objEveAdjustAlpha"));

  m.AddOpcode(
      6000, 0, "objEveMoveEnd", new Op_EndObjectMutation_Normal("objEveMove"));
  m.AddOpcode(
      6001, 0, "objEveLeftEnd", new Op_EndObjectMutation_Normal("objEveLeft"));
  m.AddOpcode(
      6002, 0, "objEveTopEnd", new Op_EndObjectMutation_Normal("objEveTop"));
  m.AddOpcode(6003,
              0,
              "objEveAlphaEnd",
              new Op_EndObjectMutation_Normal("objEveAlpha"));
  m.AddOpcode(6004,
              0,
              "objEveDisplayEnd",
              new Op_EndObjectMutation_Normal("objEveDisplay"));
  m.AddOpcode(6006,
              0,
              "objEveAdjustEnd",
              new Op_EndObjectMutation_RepNo("objEveAdjust"));
  m.AddOpcode(6040,
              0,
              "objEveAdjustAlphaEnd",
              new Op_EndObjectMutation_RepNo("objEveAdjustAlpha"));
}

}  // namespace

// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------

ObjFgModule::ObjFgModule() : RLModule("ObjFg", 1, 81) {
  addObjectFunctions(*this);
  addEveObjectFunctions(*this);
  SetProperty(P_FGBG, OBJ_FG);
}

// -----------------------------------------------------------------------

ObjBgModule::ObjBgModule() : RLModule("ObjBg", 1, 82) {
  addObjectFunctions(*this);
  addEveObjectFunctions(*this);
  SetProperty(P_FGBG, OBJ_BG);
}

// -----------------------------------------------------------------------

ChildObjFgModule::ChildObjFgModule()
    : MappedRLModule(childObjMappingFun, "ChildObjFg", 2, 81) {
  addObjectFunctions(*this);
  addEveObjectFunctions(*this);
  SetProperty(P_FGBG, OBJ_FG);
}

// -----------------------------------------------------------------------

ChildObjBgModule::ChildObjBgModule()
    : MappedRLModule(childObjMappingFun, "ChildObjBg", 2, 82) {
  addObjectFunctions(*this);
  addEveObjectFunctions(*this);
  SetProperty(P_FGBG, OBJ_BG);
}

// -----------------------------------------------------------------------

ObjRangeFgModule::ObjRangeFgModule()
    : MappedRLModule(rangeMappingFun, "ObjRangeFg", 1, 90) {
  addObjectFunctions(*this);
  SetProperty(P_FGBG, OBJ_FG);
}

// -----------------------------------------------------------------------

ObjRangeBgModule::ObjRangeBgModule()
    : MappedRLModule(rangeMappingFun, "ObjRangeBg", 1, 91) {
  addObjectFunctions(*this);
  SetProperty(P_FGBG, OBJ_BG);
}

// -----------------------------------------------------------------------

ChildObjRangeFgModule::ChildObjRangeFgModule()
    : MappedRLModule(childRangeMappingFun, "ObjChildRangeFg", 2, 90) {
  addObjectFunctions(*this);
  SetProperty(P_FGBG, OBJ_FG);
}
