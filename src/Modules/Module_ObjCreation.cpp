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

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

// Insert headers here

#include "MachineBase/RLMachine.hpp"
#include "Modules/Module_Obj.hpp"
#include "Modules/Module_ObjCreation.hpp"
#include "MachineBase/RLOperation.hpp"
#include "MachineBase/RLModule.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/Surface.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/GraphicsObject.hpp"
#include "Systems/Base/GanGraphicsObjectData.hpp"
#include "Systems/Base/GraphicsTextObject.hpp"
  
#include "Modules/cp932toUnicode.hpp"

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
  const string& ganFilename)
{
//  GraphicsSystem& gs = machine.system().graphics();

  /// @todo This is a hack and probably a source of errors. Figure
  ///       out what '???' means when used as the first parameter to
  ///       objOfFileGan.
  if(imgFilename == "???")
    imgFilename = ganFilename;
  obj.setObjectData(
    new GanGraphicsObjectData(machine, ganFilename, imgFilename));
}

// -----------------------------------------------------------------------

typedef boost::function<void(RLMachine&, GraphicsObject& obj, const string&)> DataFunction;

// -----------------------------------------------------------------------

void objOfFileLoader(RLMachine& machine, GraphicsObject& obj, const string& val)
{
  GraphicsSystem& gs = machine.system().graphics();
  obj.setObjectData(gs.buildObjOfFile(machine, val));
}

// -----------------------------------------------------------------------

void objOfTextBuilder(RLMachine& machine, GraphicsObject& obj, const string& val)
{
  // The text at this point is still cp932. Convert it.
  string utf8str = cp932toUTF8(val, machine.getTextEncoding());
  obj.setTextText(utf8str);
  obj.setObjectData(new GraphicsTextObject(machine));
}

}

// -----------------------------------------------------------------------

struct Obj_objGeneric_0 : public RLOp_Void_2<IntConstant_T, StrConstant_T> {
  int m_layer;
  DataFunction m_dataFun;
  Obj_objGeneric_0(int layer, const DataFunction& fun) 
	: m_layer(layer), m_dataFun(fun) {}

  void operator()(RLMachine& machine, int buf, string filename) {
    GraphicsObject& obj = getGraphicsObject(machine, m_layer, buf);
    m_dataFun(machine, obj, filename);
  }
};

// -----------------------------------------------------------------------

struct Obj_objGeneric_1 : public RLOp_Void_3<IntConstant_T, StrConstant_T, 
											 IntConstant_T> 
{
  int m_layer;
  DataFunction m_dataFun;
  Obj_objGeneric_1(int layer, const DataFunction& fun) 
	: m_layer(layer), m_dataFun(fun) {}

  void operator()(RLMachine& machine, int buf, string filename, int visible) {
    GraphicsObject& obj = getGraphicsObject(machine, m_layer, buf);
    m_dataFun(machine, obj, filename);
    obj.setVisible(visible);
  }
};

// -----------------------------------------------------------------------

struct Obj_objGeneric_2 
  : public RLOp_Void_5<IntConstant_T, StrConstant_T, IntConstant_T, 
					   IntConstant_T, IntConstant_T> 
{
  int m_layer;
  DataFunction m_dataFun;
  Obj_objGeneric_2(int layer, const DataFunction& fun) 
	: m_layer(layer), m_dataFun(fun) {}

  void operator()(RLMachine& machine, int buf, string filename, int visible,
                  int x, int y) {
    GraphicsObject& obj = getGraphicsObject(machine, m_layer, buf);
    m_dataFun(machine, obj, filename);
    obj.setVisible(visible);
    obj.setX(x);
    obj.setY(y);
  }
};

// -----------------------------------------------------------------------

struct Obj_objGeneric_3 : public RLOp_Void_6<IntConstant_T, StrConstant_T, 
                                          IntConstant_T, IntConstant_T,
                                          IntConstant_T, IntConstant_T> 
{
  int m_layer;
  DataFunction m_dataFun;
  Obj_objGeneric_3(int layer, const DataFunction& fun) 
    : m_layer(layer), m_dataFun(fun) {}

  void operator()(RLMachine& machine, int buf, string filename, int visible,
                  int x, int y, int pattern) {
    GraphicsObject& obj = getGraphicsObject(machine, m_layer, buf);
    m_dataFun(machine, obj, filename);
    obj.setVisible(visible);
    obj.setX(x);
    obj.setY(y);
    obj.setPattNo(pattern);
  }
};

// -----------------------------------------------------------------------

struct Obj_objGeneric_4 : public RLOp_Void_8<
  IntConstant_T, StrConstant_T, IntConstant_T, IntConstant_T,
  IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T>
{
  int m_layer;
  DataFunction m_dataFun;
  Obj_objGeneric_4(int layer, const DataFunction& fun) 
    : m_layer(layer), m_dataFun(fun) {}

  void operator()(RLMachine& machine, int buf, string filename, int visible,
                  int x, int y, int pattern, int scrollX, int scrollY) {
    GraphicsObject& obj = getGraphicsObject(machine, m_layer, buf);

    m_dataFun(machine, obj, filename);
    obj.setVisible(visible);
    obj.setX(x);
    obj.setY(y);
    obj.setPattNo(pattern);
    obj.setScrollRateX(scrollX);
    obj.setScrollRateY(scrollY);
  }
};

// -----------------------------------------------------------------------

struct Obj_objOfFileGan_0 
  : public RLOp_Void_3<IntConstant_T, StrConstant_T, StrConstant_T>
{
  int m_layer;
  Obj_objOfFileGan_0(int layer) : m_layer(layer) {}

  void operator()(RLMachine& machine, int buf, string imgFilename, 
                  string ganFilename) 
  { 
    GraphicsObject& obj = getGraphicsObject(machine, m_layer, buf);
    setObjectDataToGan(machine, obj, imgFilename, ganFilename);
    obj.setVisible(true);
  }
};

// -----------------------------------------------------------------------

struct Obj_objOfFileGan_1
  : public RLOp_Void_4<IntConstant_T, StrConstant_T, StrConstant_T, 
                       IntConstant_T>
{
  int m_layer;
  Obj_objOfFileGan_1(int layer) : m_layer(layer) {}

  void operator()(RLMachine& machine, int buf, string imgFilename, 
                  string ganFilename, int visible) 
  { 
    GraphicsObject& obj = getGraphicsObject(machine, m_layer, buf);
    setObjectDataToGan(machine, obj, imgFilename, ganFilename);
    obj.setVisible(visible);
  }
};

// -----------------------------------------------------------------------

struct Obj_objOfFileGan_2
  : public RLOp_Void_6<IntConstant_T, StrConstant_T, StrConstant_T, 
                       IntConstant_T, IntConstant_T, IntConstant_T>
{
  int m_layer;
  Obj_objOfFileGan_2(int layer) : m_layer(layer) {}

  void operator()(RLMachine& machine, int buf, string imgFilename, 
                  string ganFilename, int visible, int x, int y) 
  { 
    GraphicsObject& obj = getGraphicsObject(machine, m_layer, buf);
    setObjectDataToGan(machine, obj, imgFilename, ganFilename);
    obj.setVisible(visible);
    obj.setX(x);
    obj.setY(y);
  }
};

// -----------------------------------------------------------------------

struct Obj_objOfFileGan_3
  : public RLOp_Void_7<IntConstant_T, StrConstant_T, StrConstant_T, 
                       IntConstant_T, IntConstant_T, IntConstant_T,
                       IntConstant_T>
{
  int m_layer;
  Obj_objOfFileGan_3(int layer) : m_layer(layer) {}

  void operator()(RLMachine& machine, int buf, string imgFilename, 
                  string ganFilename, int visible, int x, int y, int pattern) 
  { 
    GraphicsObject& obj = getGraphicsObject(machine, m_layer, buf);
    setObjectDataToGan(machine, obj, imgFilename, ganFilename);
    obj.setVisible(visible);
    obj.setX(x);
    obj.setY(y);
    obj.setPattNo(pattern);
  }
};

// -----------------------------------------------------------------------

void addObjectCreationFunctions(RLModule& m, int layer)
{
  m.addOpcode(1000, 0, new Obj_objGeneric_0(layer, objOfFileLoader));
  m.addOpcode(1000, 1, new Obj_objGeneric_1(layer, objOfFileLoader));
  m.addOpcode(1000, 2, new Obj_objGeneric_2(layer, objOfFileLoader));
  m.addOpcode(1000, 3, new Obj_objGeneric_3(layer, objOfFileLoader));
  m.addOpcode(1000, 4, new Obj_objGeneric_4(layer, objOfFileLoader));

  m.addOpcode(1003, 0, "objOfFileGan", new Obj_objOfFileGan_0(layer));
  m.addOpcode(1003, 1, "objOfFileGan", new Obj_objOfFileGan_1(layer));
  m.addOpcode(1003, 2, "objOfFileGan", new Obj_objOfFileGan_2(layer));
  m.addOpcode(1003, 3, "objOfFileGan", new Obj_objOfFileGan_3(layer));
  m.addUnsupportedOpcode(1003, 4, "objOfFileGan");

  m.addUnsupportedOpcode(1101, 0, "objOfRect");
  m.addUnsupportedOpcode(1101, 1, "objOfRect");
  m.addUnsupportedOpcode(1101, 2, "objOfRect");
  m.addUnsupportedOpcode(1101, 3, "objOfRect");

  m.addOpcode(1200, 0, new Obj_objGeneric_0(layer, objOfTextBuilder));
  m.addOpcode(1200, 1, new Obj_objGeneric_1(layer, objOfTextBuilder));
  m.addOpcode(1200, 2, new Obj_objGeneric_2(layer, objOfTextBuilder));
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
  : RLModule("ObjFgCreation", 1, 71)
{
  addObjectCreationFunctions(*this, OBJ_FG_LAYER);
}

// -----------------------------------------------------------------------

ObjBgCreationModule::ObjBgCreationModule()
  : RLModule("ObjBgCreation", 1, 72)
{   
  addObjectCreationFunctions(*this, OBJ_BG_LAYER);
}
