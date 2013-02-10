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

#include "Modules/Module_Obj.hpp"
#include "Modules/Module_ObjFgBg.hpp"

#include <string>

#include "LongOperations/WaitLongOperation.hpp"
#include "MachineBase/LongOperation.hpp"
#include "MachineBase/RLOperation.hpp"
#include "MachineBase/RLOperation/DefaultValue.hpp"
#include "MachineBase/RLOperation/Rect_T.hpp"
#include "MachineBase/Properties.hpp"
#include "MachineBase/RLMachine.hpp"
#include "MachineBase/RLModule.hpp"
#include "Modules/ObjectMutatorOperations.hpp"
#include "Systems/Base/ColourFilterObjectData.hpp"
#include "Systems/Base/EventSystem.hpp"
#include "Systems/Base/GraphicsObject.hpp"
#include "Systems/Base/GraphicsObjectData.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/GraphicsTextObject.hpp"
#include "Systems/Base/ObjectMutator.hpp"
#include "Systems/Base/System.hpp"
#include "Utilities/Exception.hpp"
#include "Utilities/Graphics.hpp"
#include "Utilities/StringUtilities.hpp"

#include <cmath>
#include <iomanip>
#include <sstream>

#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>

#include "libReallive/bytecode.h"

using namespace std;
using namespace boost;
using namespace libReallive;

namespace {

struct dispArea_0 : public RLOp_Void_1< IntConstant_T > {
  void operator()(RLMachine& machine, int buf) {
    GraphicsObject& obj = getGraphicsObject(machine, this, buf);
    obj.clearClip();
  }
};

struct dispArea_1 : RLOp_Void_5< IntConstant_T, IntConstant_T,
                                 IntConstant_T, IntConstant_T,
                                 IntConstant_T > {
  void operator()(RLMachine& machine, int buf, int x1, int y1, int x2, int y2) {
    GraphicsObject& obj = getGraphicsObject(machine, this, buf);
    obj.setClip(Rect::GRP(x1, y1, x2, y2));
  }
};

struct dispRect_1 : RLOp_Void_5< IntConstant_T, IntConstant_T,
                                 IntConstant_T, IntConstant_T,
                                 IntConstant_T > {
  void operator()(RLMachine& machine, int buf, int x, int y, int w, int h) {
    GraphicsObject& obj = getGraphicsObject(machine, this, buf);
    obj.setClip(Rect::REC(x, y, w, h));
  }
};

struct dispCorner_1 : RLOp_Void_3< IntConstant_T, IntConstant_T,
                                   IntConstant_T > {
  void operator()(RLMachine& machine, int buf, int x, int y) {
    GraphicsObject& obj = getGraphicsObject(machine, this, buf);
    obj.setClip(Rect::GRP(0, 0, x, y));
  }
};

struct dispOwnArea_0 : public RLOp_Void_1< IntConstant_T > {
  void operator()(RLMachine& machine, int buf) {
    GraphicsObject& obj = getGraphicsObject(machine, this, buf);
    obj.clearOwnClip();
  }
};

struct dispOwnArea_1 : RLOp_Void_5< IntConstant_T, IntConstant_T,
                                    IntConstant_T, IntConstant_T,
                                    IntConstant_T > {
  void operator()(RLMachine& machine, int buf, int x1, int y1, int x2, int y2) {
    GraphicsObject& obj = getGraphicsObject(machine, this, buf);
    obj.setOwnClip(Rect::GRP(x1, y1, x2, y2));
  }
};

struct dispOwnRect_1 : RLOp_Void_5< IntConstant_T, IntConstant_T,
                                    IntConstant_T, IntConstant_T,
                                    IntConstant_T > {
  void operator()(RLMachine& machine, int buf, int x, int y, int w, int h) {
    GraphicsObject& obj = getGraphicsObject(machine, this, buf);
    obj.setOwnClip(Rect::REC(x, y, w, h));
  }
};

struct adjust : RLOp_Void_4< IntConstant_T, IntConstant_T, IntConstant_T,
                               IntConstant_T > {
  void operator()(RLMachine& machine, int buf, int idx, int x, int y) {
    GraphicsObject& obj = getGraphicsObject(machine, this, buf);
    obj.setXAdjustment(idx, x);
    obj.setYAdjustment(idx, y);
  }
};

struct adjustX : RLOp_Void_3< IntConstant_T, IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, int buf, int idx, int x) {
    GraphicsObject& obj = getGraphicsObject(machine, this, buf);
    obj.setXAdjustment(idx, x);
  }
};

struct adjustY : RLOp_Void_3< IntConstant_T, IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, int buf, int idx, int y) {
    GraphicsObject& obj = getGraphicsObject(machine, this, buf);
    obj.setYAdjustment(idx, y);
  }
};

struct tint : RLOp_Void_4< IntConstant_T, IntConstant_T, IntConstant_T,
                               IntConstant_T> {
  void operator()(RLMachine& machine, int buf, int r, int g, int b) {
    GraphicsObject& obj = getGraphicsObject(machine, this, buf);
    obj.setTint(RGBColour(r, g, b));
  }
};

struct colour : RLOp_Void_5< IntConstant_T, IntConstant_T, IntConstant_T,
                                 IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, int buf, int r, int g, int b, int level) {
    GraphicsObject& obj = getGraphicsObject(machine, this, buf);
    obj.setColour(RGBAColour(r, g, b, level));
  }
};

struct objSetRect_1
    : public RLOp_Void_2<IntConstant_T, Rect_T<rect_impl::GRP> > {
  void operator()(RLMachine& machine, int buf, Rect rect) {
    GraphicsObject& obj = getGraphicsObject(machine, this, buf);
    if (obj.hasObjectData()) {
      ColourFilterObjectData* data = dynamic_cast<ColourFilterObjectData*>(
          &obj.objectData());
      if (data) {
        data->setRect(rect);
      }
    }
  }
};

struct objSetRect_0
    : public RLOp_Void_1<IntConstant_T> {
  void operator()(RLMachine& machine, int buf) {
    Rect rect(0, 0, getScreenSize(machine.system().gameexe()));
    objSetRect_1()(machine, buf, rect);
  }
};

struct objSetText
    : public RLOp_Void_2<IntConstant_T, DefaultStrValue_T> {
  void operator()(RLMachine& machine, int buf, string val) {
    GraphicsObject& obj = getGraphicsObject(machine, this, buf);
    std::string utf8str = cp932toUTF8(val, machine.getTextEncoding());
    obj.setTextText(utf8str);
  }
};

struct objTextOpts
  : public RLOp_Void_7<IntConstant_T, IntConstant_T, IntConstant_T,
                       IntConstant_T, IntConstant_T, IntConstant_T,
                       IntConstant_T> {
  void operator()(RLMachine& machine, int buf, int size, int xspace,
                  int yspace, int vert, int colour, int shadow) {
    GraphicsObject& obj = getGraphicsObject(machine, this, buf);
    obj.setTextOps(size, xspace, yspace, vert, colour, shadow);
  }
};

struct objDriftOpts
    : public RLOp_Void_13<IntConstant_T, IntConstant_T, IntConstant_T,
                          IntConstant_T, IntConstant_T, IntConstant_T,
                          IntConstant_T, IntConstant_T, IntConstant_T,
                          IntConstant_T, IntConstant_T, IntConstant_T,
                          Rect_T<rect_impl::GRP> > {
  void operator()(RLMachine& machine, int buf, int count, int use_animation,
                  int start_pattern, int end_pattern,
                  int total_animaton_time_ms, int yspeed, int period,
                  int amplitude, int use_drift, int unknown, int driftspeed,
                  Rect drift_area) {
    GraphicsObject& obj = getGraphicsObject(machine, this, buf);
    obj.setDriftOpts(count, use_animation, start_pattern, end_pattern,
                     total_animaton_time_ms, yspeed, period, amplitude,
                     use_drift, unknown, driftspeed, drift_area);
  }
};

struct objNumOpts
    : public RLOp_Void_6<IntConstant_T, IntConstant_T, IntConstant_T,
                         IntConstant_T, IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, int buf, int digits, int zero,
                  int sign, int pack, int space) {
    GraphicsObject& obj = getGraphicsObject(machine, this, buf);
    obj.setDigitOpts(digits, zero, sign, pack, space);
  }
};

struct objAdjustAlpha
    : public RLOp_Void_3<IntConstant_T, IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, int buf, int idx, int alpha) {
    GraphicsObject& obj = getGraphicsObject(machine, this, buf);
    obj.setAlphaAdjustment(idx, alpha);
  }
};

struct objButtonOpts
    : public RLOp_Void_5<IntConstant_T, IntConstant_T, IntConstant_T,
                         IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, int buf, int action, int se,
                  int group, int button_number) {
    GraphicsObject& obj = getGraphicsObject(machine, this, buf);
    obj.setButtonOpts(action, se, group, button_number);
  }
};

// -----------------------------------------------------------------------

// Special adapter to make any of obj* and objBg* operation structs
// into an objRange* or objRangeBg* struct.
//
// We extract the first two expression pieces from the incoming
// command and assume that they are integers and are the bounds on the
// object number. We then construct a set of parameters to pass to the
// real implementation.
//
// This is certainly not the most efficient way to do it, but it cuts
// down on a duplicated operation struct for each obj* and objBg*
// function, alowing us to just use this adapter with the already
// defined operations.
//
// @see rangeMappingFun
struct ObjRangeAdapter : RLOp_SpecialCase {
  // Keep a copy of the operation that we wrap
  scoped_ptr<RLOperation> handler;

  explicit ObjRangeAdapter(RLOperation* in) : handler(in) { }

  void operator()(RLMachine& machine, const libReallive::CommandElement& ff) {
    const ptr_vector<ExpressionPiece>& allParameters = ff.getParameters();

    // Range check the data
    if (allParameters.size() < 2)
      throw rlvm::Exception("Less then two arguments to an objRange function!");

    // BIG WARNING ABOUT THE FOLLOWING CODE: Note that we copy half of
    // what RLOperation.dispatchFunction() does; we manually call the
    // subclass's dispatch() so that we can get around the automated
    // incrementing of the instruction pointer.
    int lowerRange = allParameters[0].integerValue(machine);
    int upperRange = allParameters[1].integerValue(machine);
    for (int i = lowerRange; i <= upperRange; ++i) {
      // Create a new list of expression pieces that contain the
      // current object we're dealing with and
      ptr_vector<ExpressionPiece> currentInstantiation;
      currentInstantiation.push_back(new IntegerConstant(i));

      // Copy everything after the first two items
      ptr_vector<ExpressionPiece>::const_iterator it = allParameters.begin();
      std::advance(it, 2);
      for (; it != allParameters.end(); ++it) {
        currentInstantiation.push_back(it->clone());
      }

      // Now dispatch based on these parameters.
      handler->dispatch(machine, currentInstantiation);
    }

    machine.advanceInstructionPointer();
  }
};

class objEveAdjust
    : public RLOp_Void_7<IntConstant_T, IntConstant_T, IntConstant_T,
                         IntConstant_T, IntConstant_T, IntConstant_T,
                         IntConstant_T> {
 public:
  virtual void operator()(RLMachine& machine,
                          int obj, int repno, int x,
                          int y, int duration_time, int delay, int type) {
    unsigned int creation_time = machine.system().event().getTicks();

    GraphicsObject& object = getGraphicsObject(machine, this, obj);
    int start_x = object.xAdjustment(repno);
    int start_y = object.yAdjustment(repno);

    object.AddObjectMutator(
        new AdjustMutator(machine, repno,
                          creation_time, duration_time, delay,
                          type, start_x, x, start_y, y));
  }

 private:
  // We need a custom mutator here. One of the parameters isn't varying.
  class AdjustMutator : public ObjectMutator {
   public:
    AdjustMutator(RLMachine& machine, int repno,
                  int creation_time, int duration_time, int delay,
                  int type, int start_x, int target_x, int start_y,
                  int target_y)
        : ObjectMutator(repno, "objEveAdjust",
                        creation_time, duration_time, delay, type),
          repno_(repno),
          start_x_(start_x),
          end_x_(target_x),
          start_y_(start_y),
          end_y_(target_y) {
    }

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

struct LongOp_MutatorWait : public LongOperation {
  LongOp_MutatorWait(RLOperation* op, int obj, int repno, const char* name)
      : op_(op),
        obj_(obj),
        repno_(repno),
        name_(name) {
  }

  bool operator()(RLMachine& machine) {
    return getGraphicsObject(machine, op_, obj_).IsMutatorRunningMatching(
        repno_, name_) == false;
  }

  RLOperation* op_;
    int obj_, repno_;
  const char* name_;
};

class Op_MutatorWaitNormal : public RLOp_Void_1<IntConstant_T> {
 public:
  Op_MutatorWaitNormal(const char* name) : name_(name) {}

  virtual void operator()(RLMachine& machine, int obj) {
    machine.pushLongOperation(new LongOp_MutatorWait(
        this, obj, -1, name_));
  }

 private:
  const char* name_;
};

class Op_MutatorWaitRepNo
    : public RLOp_Void_2<IntConstant_T, IntConstant_T> {
 public:
  Op_MutatorWaitRepNo(const char* name) : name_(name) {}

  virtual void operator()(RLMachine& machine, int obj, int repno) {
    machine.pushLongOperation(new LongOp_MutatorWait(
        this, obj, repno, name_));
  }

 private:
  const char* name_;
};

bool objectMutatorIsWorking(RLMachine& machine, RLOperation* op, int obj,
                            int repno, const char* name) {
  return getGraphicsObject(machine, op, obj).IsMutatorRunningMatching(
      repno, name) == false;
}

class Op_MutatorWaitCNormal
    : public RLOp_Void_1<IntConstant_T> {
 public:
  Op_MutatorWaitCNormal(const char* name) : name_(name) {}

  virtual void operator()(RLMachine& machine, int obj) {
    WaitLongOperation* wait_op = new WaitLongOperation(machine);
    wait_op->breakOnClicks();
    wait_op->breakOnEvent(
        boost::bind(objectMutatorIsWorking,
                    boost::ref(machine), this, obj, -1, name_));
    machine.pushLongOperation(wait_op);
  }

 private:
  const char* name_;
};

class Op_MutatorWaitCRepNo
    : public RLOp_Void_2<IntConstant_T, IntConstant_T> {
 public:
  Op_MutatorWaitCRepNo(const char* name) : name_(name) {}

  virtual void operator()(RLMachine& machine, int obj, int repno) {
    WaitLongOperation* wait_op = new WaitLongOperation(machine);
    wait_op->breakOnClicks();
    wait_op->breakOnEvent(
        boost::bind(objectMutatorIsWorking,
                    boost::ref(machine), this, obj, repno, name_));
    machine.pushLongOperation(wait_op);
  }

 private:
  const char* name_;
};

// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------

void addObjectFunctions(RLModule& m) {
  m.addOpcode(1000, 0, "objMove", new Obj_SetTwoIntOnObj(
                  &GraphicsObject::setX,
                  &GraphicsObject::setY));
  m.addOpcode(1001, 0, "objLeft",
              new Obj_SetOneIntOnObj(&GraphicsObject::setX));
  m.addOpcode(1002, 0, "objTop",
              new Obj_SetOneIntOnObj(&GraphicsObject::setY));
  m.addOpcode(1003, 0, "objAlpha",
              new Obj_SetOneIntOnObj(&GraphicsObject::setAlpha));
  m.addOpcode(1004, 0, "objShow",
              new Obj_SetOneIntOnObj(&GraphicsObject::setVisible));
  m.addOpcode(1005, 0, "objDispArea", new dispArea_0);
  m.addOpcode(1005, 1, "objDispArea", new dispArea_1);
  m.addOpcode(1006, 0, "objAdjust", new adjust);
  m.addOpcode(1007, 0, "objAdjustX", new adjustX);
  m.addOpcode(1008, 0, "objAdjustY", new adjustY);
  m.addOpcode(1009, 0, "objMono",
              new Obj_SetOneIntOnObj(&GraphicsObject::setMono));
  m.addOpcode(1010, 0, "objInvert",
              new Obj_SetOneIntOnObj(&GraphicsObject::setInvert));
  m.addOpcode(1011, 0, "objLight",
              new Obj_SetOneIntOnObj(&GraphicsObject::setLight));
  m.addOpcode(1012, 0, "objTint", new tint);
  m.addOpcode(1013, 0, "objTintR",
              new Obj_SetOneIntOnObj(&GraphicsObject::setTintR));
  m.addOpcode(1014, 0, "objTintG",
              new Obj_SetOneIntOnObj(&GraphicsObject::setTintG));
  m.addOpcode(1015, 0, "objTintB",
              new Obj_SetOneIntOnObj(&GraphicsObject::setTintB));
  m.addOpcode(1016, 0, "objColour", new colour);
  m.addOpcode(1017, 0, "objColR",
              new Obj_SetOneIntOnObj(&GraphicsObject::setColourR));
  m.addOpcode(1018, 0, "objColG",
              new Obj_SetOneIntOnObj(&GraphicsObject::setColourG));
  m.addOpcode(1019, 0, "objColB",
              new Obj_SetOneIntOnObj(&GraphicsObject::setColourB));
  m.addOpcode(1020, 0, "objColLevel",
              new Obj_SetOneIntOnObj(&GraphicsObject::setColourLevel));
  m.addOpcode(1021, 0, "objComposite",
              new Obj_SetOneIntOnObj(&GraphicsObject::setCompositeMode));

  m.addOpcode(1022, 0, "objSetRect", new objSetRect_0);
  m.addOpcode(1022, 1, "objSetRect", new objSetRect_1);

  m.addOpcode(1024, 0, "objSetText", new objSetText);
  m.addOpcode(1024, 1, "objSetText", new objSetText);
  m.addOpcode(1025, 0, "objTextOpts", new objTextOpts);

  m.addOpcode(1026, 0, "objLayer",
              new Obj_SetOneIntOnObj(&GraphicsObject::setZLayer));
  m.addOpcode(1027, 0, "objDepth",
              new Obj_SetOneIntOnObj(&GraphicsObject::setZDepth));
  m.addUnsupportedOpcode(1028, 0, "objScrollRate");
  m.addOpcode(1029, 0, "objScrollRateX",
              new Obj_SetOneIntOnObj(&GraphicsObject::setScrollRateX));
  m.addOpcode(1030, 0, "objScrollRateY",
              new Obj_SetOneIntOnObj(&GraphicsObject::setScrollRateY));
  m.addOpcode(1031, 0, "objDriftOpts", new objDriftOpts);
  m.addOpcode(1032, 0, "objOrder",
              new Obj_SetOneIntOnObj(&GraphicsObject::setZOrder));
  m.addUnsupportedOpcode(1033, 0, "objQuarterView");

  m.addOpcode(1034, 0, "objDispRect", new dispArea_0);
  m.addOpcode(1034, 1, "objDispRect", new dispRect_1);
  m.addOpcode(1035, 0, "objDispCorner", new dispArea_0);
  m.addOpcode(1035, 1, "objDispCorner", new dispCorner_1);
  m.addOpcode(1036, 0, "objAdjustVert",
              new Obj_SetOneIntOnObj(&GraphicsObject::setVert));
  m.addOpcode(1037, 0, "objSetDigits",
              new Obj_SetOneIntOnObj(&GraphicsObject::setDigitValue));
  m.addOpcode(1038, 0, "objNumOpts", new objNumOpts);
  m.addOpcode(1039, 0, "objPattNo",
              new Obj_SetOneIntOnObj(&GraphicsObject::setPattNo));

  m.addOpcode(1040, 0, "objAdjustAlpha", new objAdjustAlpha);
  m.addUnsupportedOpcode(1041, 0, "objAdjustAll");
  m.addUnsupportedOpcode(1042, 0, "objAdjustAllX");
  m.addUnsupportedOpcode(1043, 0, "objAdjustAllY");

  m.addOpcode(1046, 0, "objScale", new Obj_SetTwoIntOnObj(
      &GraphicsObject::setWidth, &GraphicsObject::setHeight));
  m.addOpcode(1047, 0, "objWidth",
              new Obj_SetOneIntOnObj(&GraphicsObject::setWidth));
  m.addOpcode(1048, 0, "objHeight",
              new Obj_SetOneIntOnObj(&GraphicsObject::setHeight));
  m.addOpcode(1049, 0, "objRotate",
              new Obj_SetOneIntOnObj(&GraphicsObject::setRotation));

  m.addOpcode(1050, 0, "objRepOrigin", new Obj_SetTwoIntOnObj(
      &GraphicsObject::setXRepOrigin, &GraphicsObject::setYRepOrigin));
  m.addOpcode(1051, 0, "objRepOriginX",
              new Obj_SetOneIntOnObj(&GraphicsObject::setXRepOrigin));
  m.addOpcode(1052, 0, "objRepOriginY",
              new Obj_SetOneIntOnObj(&GraphicsObject::setYRepOrigin));
  m.addOpcode(1053, 0, "objOrigin", new Obj_SetTwoIntOnObj(
      &GraphicsObject::setXOrigin, &GraphicsObject::setYOrigin));
  m.addOpcode(1054, 0, "objOriginX",
              new Obj_SetOneIntOnObj(&GraphicsObject::setXOrigin));
  m.addOpcode(1055, 0, "objOriginY",
              new Obj_SetOneIntOnObj(&GraphicsObject::setYOrigin));
  m.addUnsupportedOpcode(1056, 0, "objFadeOpts");

  m.addOpcode(1061, 0, "objHqScale", new Obj_SetTwoIntOnObj(
      &GraphicsObject::setHqWidth, &GraphicsObject::setHqHeight));
  m.addOpcode(1062, 0, "objHqWidth", new Obj_SetOneIntOnObj(
      &GraphicsObject::setHqWidth));
  m.addOpcode(1063, 0, "objHqHeight", new Obj_SetOneIntOnObj(
      &GraphicsObject::setHqHeight));

  m.addOpcode(1064, 2, "objButtonOpts", new objButtonOpts);
  m.addOpcode(1066, 0, "objBtnState", new Obj_SetOneIntOnObj(
      &GraphicsObject::setButtonState));

  m.addOpcode(1070, 0, "objOwnDispArea", new dispOwnArea_0);
  m.addOpcode(1070, 1, "objOwnDispArea", new dispOwnArea_1);
  m.addOpcode(1071, 0, "objOwnDispRect", new dispOwnArea_0);
  m.addOpcode(1071, 1, "objOwnDispRect", new dispOwnRect_1);
}

void addEveObjectFunctions(RLModule& m) {
  m.addOpcode(2000, 0, "objEveMove", new Obj_SetTwoIntOnObj(
      &GraphicsObject::setX,
      &GraphicsObject::setY));
  m.addOpcode(2000, 1, "objEveMove",
              new Op_ObjectMutatorIntInt(&GraphicsObject::x,
                                         &GraphicsObject::setX,
                                         &GraphicsObject::y,
                                         &GraphicsObject::setY,
                                         "objEveMove"));

  m.addOpcode(2001, 0, "objEveLeft",
              new Obj_SetOneIntOnObj(&GraphicsObject::setX));
  m.addOpcode(2001, 1, "objEveLeft",
              new Op_ObjectMutatorInt(&GraphicsObject::x,
                                      &GraphicsObject::setX,
                                      "objEveLeft"));

  m.addOpcode(2002, 0, "objEveTop",
              new Obj_SetOneIntOnObj(&GraphicsObject::setY));
  m.addOpcode(2002, 1, "objEveTop",
              new Op_ObjectMutatorInt(&GraphicsObject::y,
                                      &GraphicsObject::setY,
                                      "objEveTop"));

  m.addOpcode(2003, 0, "objEveAlpha",
              new Obj_SetOneIntOnObj(&GraphicsObject::setAlpha));
  m.addOpcode(2003, 1, "objEveAlpha",
              new Op_ObjectMutatorInt(&GraphicsObject::rawAlpha,
                                      &GraphicsObject::setAlpha,
                                      "objEveAlpha"));

  m.addOpcode(2006, 0, "objEveAdjust", new adjust);
  m.addOpcode(2006, 1, "objEveAdjust", new objEveAdjust);

  m.addOpcode(4000, 0, "objEveMoveWait",
              new Op_MutatorWaitNormal("objEveMove"));
  m.addOpcode(4001, 0, "objEveLeftWait",
              new Op_MutatorWaitNormal("objEveLeft"));
  m.addOpcode(4002, 0, "objEveTopWait",
              new Op_MutatorWaitNormal("objEveTop"));
  m.addOpcode(4003, 0, "objEveAlphaWait",
              new Op_MutatorWaitNormal("objEveAlpha"));
  m.addOpcode(4006, 0, "objEveAdjustEnd",
              new Op_MutatorWaitRepNo("objEveAdjust"));

  m.addOpcode(5000, 0, "objEveMoveWaitC",
              new Op_MutatorWaitCNormal("objEveMove"));
  m.addOpcode(5001, 0, "objEveLeftWaitC",
              new Op_MutatorWaitCNormal("objEveLeft"));
  m.addOpcode(5002, 0, "objEveTopWaitC",
              new Op_MutatorWaitCNormal("objEveTop"));
  m.addOpcode(5003, 0, "objEveAlphaWaitC",
              new Op_MutatorWaitCNormal("objEveAlpha"));
  m.addOpcode(5006, 0, "objEveAdjustWaitC",
              new Op_MutatorWaitRepNo("objEveAdjust"));

  m.addOpcode(6000, 0, "objEveMoveEnd",
              new Op_EndObjectMutation_Normal("objEveMove"));
  m.addOpcode(6001, 0, "objEveLeftEnd",
              new Op_EndObjectMutation_Normal("objEveLeft"));
  m.addOpcode(6002, 0, "objEveTopEnd",
              new Op_EndObjectMutation_Normal("objEveTop"));
  m.addOpcode(6003, 0, "objEveAlphaEnd",
              new Op_EndObjectMutation_Normal("objEveAlpha"));

  m.addOpcode(6006, 0, "objEveAdjustEnd",
              new Op_EndObjectMutation_RepNo("objEveAdjust"));
}

}  // namespace

// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------

ObjFgModule::ObjFgModule()
  : RLModule("ObjFg", 1, 81) {
  addObjectFunctions(*this);
  addEveObjectFunctions(*this);
  setProperty(P_FGBG, OBJ_FG);
}

// -----------------------------------------------------------------------

ObjBgModule::ObjBgModule()
  : RLModule("ObjBg", 1, 82) {
  addObjectFunctions(*this);
  addEveObjectFunctions(*this);
  setProperty(P_FGBG, OBJ_BG);
}

// -----------------------------------------------------------------------

ChildObjFgModule::ChildObjFgModule()
    : MappedRLModule(childObjMappingFun, "ChildObjFg", 2, 81) {
  addObjectFunctions(*this);
  addEveObjectFunctions(*this);
  setProperty(P_FGBG, OBJ_FG);
}

// -----------------------------------------------------------------------

ChildObjBgModule::ChildObjBgModule()
    : MappedRLModule(childObjMappingFun, "ChildObjBg", 2, 82) {
  addObjectFunctions(*this);
  addEveObjectFunctions(*this);
  setProperty(P_FGBG, OBJ_BG);
}

// -----------------------------------------------------------------------

// Mapping function for a MappedRLModule which turns operation op into
// a ranged operation.
//
// The wrapper takes ownership of the incoming op pointer, and the
// caller takes ownership of the resultant RLOperation.
RLOperation* rangeMappingFun(RLOperation* op) {
  return new ObjRangeAdapter(op);
}

// -----------------------------------------------------------------------

ObjRangeFgModule::ObjRangeFgModule()
  : MappedRLModule(rangeMappingFun, "ObjRangeFg", 1, 90) {
  addObjectFunctions(*this);
  setProperty(P_FGBG, OBJ_FG);
}

// -----------------------------------------------------------------------

ObjRangeBgModule::ObjRangeBgModule()
  : MappedRLModule(rangeMappingFun, "ObjRangeBg", 1, 91) {
  addObjectFunctions(*this);
  setProperty(P_FGBG, OBJ_BG);
}
