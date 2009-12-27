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

#include <string>

#include "MachineBase/RLMachine.hpp"
#include "Modules/Module_Obj.hpp"
#include "Modules/Module_ObjCreation.hpp"
#include "MachineBase/Properties.hpp"
#include "MachineBase/RLOperation.hpp"
#include "MachineBase/RLModule.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/Surface.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/GraphicsObject.hpp"
#include "Systems/Base/GanGraphicsObjectData.hpp"
#include "Systems/Base/GraphicsTextObject.hpp"
#include "Utilities/StringUtilities.hpp"

#include <cmath>

#include <boost/shared_ptr.hpp>

using namespace std;
using namespace boost;
using namespace libReallive;

// -----------------------------------------------------------------------

namespace {

void setObjectDataToGan(
  RLMachine& machine,
  GraphicsObject& obj,
  string& imgFilename,
  const string& ganFilename) {
  /// @todo This is a hack and probably a source of errors. Figure
  ///       out what '???' means when used as the first parameter to
  ///       objOfFileGan.
  if (imgFilename == "???")
    imgFilename = ganFilename;
  obj.setObjectData(
      new GanGraphicsObjectData(machine.system(), ganFilename, imgFilename));
}

typedef boost::function<void(RLMachine&, GraphicsObject& obj,
                             const string&)> DataFunction;

void objOfFileLoader(RLMachine& machine, GraphicsObject& obj,
                     const string& val) {
  obj.setObjectData(machine.system().graphics().buildObjOfFile(val));
}

void objOfTextBuilder(RLMachine& machine, GraphicsObject& obj,
                      const string& val) {
  // The text at this point is still cp932. Convert it.
  string utf8str = cp932toUTF8(val, machine.getTextEncoding());
  obj.setTextText(utf8str);
  obj.setObjectData(new GraphicsTextObject(machine.system()));
}

struct objGeneric_0 : public RLOp_Void_2<IntConstant_T, StrConstant_T> {
  DataFunction data_fun_;
  explicit objGeneric_0(const DataFunction& fun) : data_fun_(fun) {}

  void operator()(RLMachine& machine, int buf, string filename) {
    GraphicsObject& obj = getGraphicsObject(machine, this, buf);
    data_fun_(machine, obj, filename);
  }
};

struct objGeneric_1 : public RLOp_Void_3<IntConstant_T, StrConstant_T,
                                         IntConstant_T> {
  DataFunction data_fun_;
  explicit objGeneric_1(const DataFunction& fun) : data_fun_(fun) {}

  void operator()(RLMachine& machine, int buf, string filename, int visible) {
    GraphicsObject& obj = getGraphicsObject(machine, this, buf);
    data_fun_(machine, obj, filename);
    obj.setVisible(visible);
  }
};

struct objGeneric_2
    : public RLOp_Void_5<IntConstant_T, StrConstant_T, IntConstant_T,
                         IntConstant_T, IntConstant_T> {
  DataFunction data_fun_;
  explicit objGeneric_2(const DataFunction& fun) : data_fun_(fun) {}

  void operator()(RLMachine& machine, int buf, string filename, int visible,
                  int x, int y) {
    GraphicsObject& obj = getGraphicsObject(machine, this, buf);
    data_fun_(machine, obj, filename);
    obj.setVisible(visible);
    obj.setX(x);
    obj.setY(y);
  }
};

struct objGeneric_3 : public RLOp_Void_6<IntConstant_T, StrConstant_T,
                                         IntConstant_T, IntConstant_T,
                                         IntConstant_T, IntConstant_T> {
  DataFunction data_fun_;
  explicit objGeneric_3(const DataFunction& fun) : data_fun_(fun) {}

  void operator()(RLMachine& machine, int buf, string filename, int visible,
                  int x, int y, int pattern) {
    GraphicsObject& obj = getGraphicsObject(machine, this, buf);
    data_fun_(machine, obj, filename);
    obj.setVisible(visible);
    obj.setX(x);
    obj.setY(y);
    obj.setPattNo(pattern);
  }
};

struct objGeneric_4 : public RLOp_Void_8<
  IntConstant_T, StrConstant_T, IntConstant_T, IntConstant_T,
  IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T> {
  DataFunction data_fun_;
  explicit objGeneric_4(const DataFunction& fun) : data_fun_(fun) {}

  void operator()(RLMachine& machine, int buf, string filename, int visible,
                  int x, int y, int pattern, int scrollX, int scrollY) {
    GraphicsObject& obj = getGraphicsObject(machine, this, buf);

    data_fun_(machine, obj, filename);
    obj.setVisible(visible);
    obj.setX(x);
    obj.setY(y);
    obj.setPattNo(pattern);
    obj.setScrollRateX(scrollX);
    obj.setScrollRateY(scrollY);
  }
};

struct objOfFileGan_0
    : public RLOp_Void_3<IntConstant_T, StrConstant_T, StrConstant_T> {
  void operator()(RLMachine& machine, int buf, string imgFilename,
                  string ganFilename) {
    GraphicsObject& obj = getGraphicsObject(machine, this, buf);
    setObjectDataToGan(machine, obj, imgFilename, ganFilename);
    obj.setVisible(true);
  }
};

struct objOfFileGan_1
    : public RLOp_Void_4<IntConstant_T, StrConstant_T, StrConstant_T,
                         IntConstant_T> {
  void operator()(RLMachine& machine, int buf, string imgFilename,
                  string ganFilename, int visible) {
    GraphicsObject& obj = getGraphicsObject(machine, this, buf);
    setObjectDataToGan(machine, obj, imgFilename, ganFilename);
    obj.setVisible(visible);
  }
};

struct objOfFileGan_2
    : public RLOp_Void_6<IntConstant_T, StrConstant_T, StrConstant_T,
                         IntConstant_T, IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, int buf, string imgFilename,
                  string ganFilename, int visible, int x, int y) {
    GraphicsObject& obj = getGraphicsObject(machine, this, buf);
    setObjectDataToGan(machine, obj, imgFilename, ganFilename);
    obj.setVisible(visible);
    obj.setX(x);
    obj.setY(y);
  }
};

struct objOfFileGan_3
    : public RLOp_Void_7<IntConstant_T, StrConstant_T, StrConstant_T,
                         IntConstant_T, IntConstant_T, IntConstant_T,
                         IntConstant_T> {
  void operator()(RLMachine& machine, int buf, string imgFilename,
                  string ganFilename, int visible, int x, int y, int pattern) {
    GraphicsObject& obj = getGraphicsObject(machine, this, buf);
    setObjectDataToGan(machine, obj, imgFilename, ganFilename);
    obj.setVisible(visible);
    obj.setX(x);
    obj.setY(y);
    obj.setPattNo(pattern);
  }
};

}  // namespace

// -----------------------------------------------------------------------

void addObjectCreationFunctions(RLModule& m) {
  m.addOpcode(1000, 0, "objOfFile", new objGeneric_0(objOfFileLoader));
  m.addOpcode(1000, 1, "objOfFile", new objGeneric_1(objOfFileLoader));
  m.addOpcode(1000, 2, "objOfFile", new objGeneric_2(objOfFileLoader));
  m.addOpcode(1000, 3, "objOfFile", new objGeneric_3(objOfFileLoader));
  m.addOpcode(1000, 4, "objOfFile", new objGeneric_4(objOfFileLoader));

  m.addOpcode(1003, 0, "objOfFileGan", new objOfFileGan_0);
  m.addOpcode(1003, 1, "objOfFileGan", new objOfFileGan_1);
  m.addOpcode(1003, 2, "objOfFileGan", new objOfFileGan_2);
  m.addOpcode(1003, 3, "objOfFileGan", new objOfFileGan_3);
  m.addUnsupportedOpcode(1003, 4, "objOfFileGan");

  m.addUnsupportedOpcode(1101, 0, "objOfRect");
  m.addUnsupportedOpcode(1101, 1, "objOfRect");
  m.addUnsupportedOpcode(1101, 2, "objOfRect");
  m.addUnsupportedOpcode(1101, 3, "objOfRect");

  m.addOpcode(1200, 0, "objOfText", new objGeneric_0(objOfTextBuilder));
  m.addOpcode(1200, 1, "objOfText", new objGeneric_1(objOfTextBuilder));
  m.addOpcode(1200, 2, "objOfText", new objGeneric_2(objOfTextBuilder));
  m.addUnsupportedOpcode(1200, 3, "objOfText");

  m.addUnsupportedOpcode(1300, 0, "objDriftOfFile");
  m.addUnsupportedOpcode(1300, 1, "objDriftOfFile");
  m.addUnsupportedOpcode(1300, 2, "objDriftOfFile");
  m.addUnsupportedOpcode(1300, 3, "objDriftOfFile");

  m.addUnsupportedOpcode(1400, 0, "objOfDigits");
  m.addUnsupportedOpcode(1400, 1, "objOfDigits");
  m.addUnsupportedOpcode(1400, 2, "objOfDigits");
  m.addUnsupportedOpcode(1400, 3, "objOfDigits");
}

// -----------------------------------------------------------------------

ObjFgCreationModule::ObjFgCreationModule()
    : RLModule("ObjFgCreation", 1, 71) {
  addObjectCreationFunctions(*this);
  setProperty(P_FGBG, OBJ_FG);
}

// -----------------------------------------------------------------------

ObjBgCreationModule::ObjBgCreationModule()
    : RLModule("ObjBgCreation", 1, 72) {
  addObjectCreationFunctions(*this);
  setProperty(P_FGBG, OBJ_BG);
}

// -----------------------------------------------------------------------

ChildObjFgCreationModule::ChildObjFgCreationModule()
    : MappedRLModule(childObjMappingFun, "ChildObjFgCreation", 2, 71) {
  addObjectCreationFunctions(*this);
  setProperty(P_FGBG, OBJ_FG);
}

// -----------------------------------------------------------------------

ChildObjBgCreationModule::ChildObjBgCreationModule()
    : MappedRLModule(childObjMappingFun, "ChildObjBgCreation", 2, 72) {
  addObjectCreationFunctions(*this);
  setProperty(P_FGBG, OBJ_BG);
}
