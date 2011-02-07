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

// Contains definitions for object handling functions for the Modules 81
// "ObjFg", 82 "ObjBg", 90 "ObjRange", and 91 "ObjBgRange".

#include "Modules/Module_Obj.hpp"
#include "Modules/Module_ObjFgBg.hpp"

#include <string>

#include "MachineBase/RLOperation.hpp"
#include "MachineBase/RLOperation/DefaultValue.hpp"
#include "MachineBase/RLOperation/Rect_T.hpp"
#include "MachineBase/Properties.hpp"
#include "MachineBase/RLMachine.hpp"
#include "MachineBase/RLModule.hpp"
#include "Systems/Base/GraphicsObject.hpp"
#include "Systems/Base/GraphicsObjectData.hpp"
#include "Systems/Base/GraphicsTextObject.hpp"
#include "Utilities/Exception.hpp"
#include "Utilities/StringUtilities.hpp"

#include <cmath>
#include <iomanip>
#include <sstream>

#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>

#include "libReallive/bytecode.h"

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

  m.addOpcode(1024, 0, "objSetText", new objSetText);
  m.addOpcode(1024, 1, "objSetText", new objSetText);
  m.addOpcode(1025, 0, "objTextOpts", new objTextOpts);

  m.addUnsupportedOpcode(1026, 0, "objLayer");
  m.addUnsupportedOpcode(1027, 0, "objDepth");
  m.addUnsupportedOpcode(1028, 0, "objScrollRate");
  m.addOpcode(1029, 0, "objScrollRateX",
              new Obj_SetOneIntOnObj(&GraphicsObject::setScrollRateX));
  m.addOpcode(1030, 0, "objScrollRateY",
              new Obj_SetOneIntOnObj(&GraphicsObject::setScrollRateY));
  m.addOpcode(1031, 0, "objDriftOpts", new objDriftOpts);
  m.addUnsupportedOpcode(1032, 0, "objOrder");
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
}

}  // namespace

// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------

ObjFgModule::ObjFgModule()
  : RLModule("ObjFg", 1, 81) {
  addObjectFunctions(*this);
  setProperty(P_FGBG, OBJ_FG);
}

// -----------------------------------------------------------------------

ObjBgModule::ObjBgModule()
  : RLModule("ObjBg", 1, 82) {
  addObjectFunctions(*this);
  setProperty(P_FGBG, OBJ_BG);
}

// -----------------------------------------------------------------------

ChildObjFgModule::ChildObjFgModule()
    : MappedRLModule(childObjMappingFun, "ChildObjFg", 2, 81) {
  addObjectFunctions(*this);
  setProperty(P_FGBG, OBJ_FG);
}

// -----------------------------------------------------------------------

ChildObjBgModule::ChildObjBgModule()
    : MappedRLModule(childObjMappingFun, "ChildObjBg", 2, 82) {
  addObjectFunctions(*this);
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
