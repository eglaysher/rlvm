// This file is part of RLVM, a RealLive virutal machine clone.
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

// Insert headers here

#include "Modules/Module_Obj.hpp"
#include "Modules/Module_ObjCreation.hpp"
#include "MachineBase/RLOperation.hpp"
#include "MachineBase/RLModule.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/GraphicsObject.hpp"

#include <cmath>

#include <boost/shared_ptr.hpp>

using namespace std;
using namespace boost;
using namespace libReallive;

// -----------------------------------------------------------------------

struct Obj_objOfFile_0 : public RLOp_Void_2<IntConstant_T, StrConstant_T> {
  int m_layer;
  Obj_objOfFile_0(int layer) : m_layer(layer) {}

  void operator()(RLMachine& machine, int buf, string filename) {
    GraphicsSystem& gs = machine.system().graphics();
    GraphicsObject& obj = getGraphicsObject(machine, m_layer, buf);
    obj.setObjectData(gs.buildObjOfFile(machine, filename));
    obj.setVisible(true);
  }
};

// -----------------------------------------------------------------------

struct Obj_objOfFile_1 : public RLOp_Void_3<IntConstant_T, StrConstant_T, 
                                          IntConstant_T> {
  int m_layer;
  Obj_objOfFile_1(int layer) : m_layer(layer) {}
  
  void operator()(RLMachine& machine, int buf, string filename, int visible) {
    GraphicsSystem& gs = machine.system().graphics();
    GraphicsObject& obj = getGraphicsObject(machine, m_layer, buf);
    obj.setObjectData(gs.buildObjOfFile(machine, filename));
    obj.setVisible(visible);
  }
};

// -----------------------------------------------------------------------

struct Obj_objOfFile_2 : public RLOp_Void_5<IntConstant_T, StrConstant_T, 
                                            IntConstant_T, 
                                            IntConstant_T, IntConstant_T> {
  int m_layer;
  Obj_objOfFile_2(int layer) : m_layer(layer) {}

  void operator()(RLMachine& machine, int buf, string filename, int visible,
                  int x, int y) {
    GraphicsSystem& gs = machine.system().graphics();
    GraphicsObject& obj = getGraphicsObject(machine, m_layer, buf);
    obj.setObjectData(gs.buildObjOfFile(machine, filename));
    obj.setVisible(visible);
    obj.setX(x);
    obj.setY(y);
  }
};

// -----------------------------------------------------------------------

struct Obj_objOfFile_3 : public RLOp_Void_6<IntConstant_T, StrConstant_T, 
                                          IntConstant_T, IntConstant_T,
                                          IntConstant_T, IntConstant_T> {
  int m_layer;
  Obj_objOfFile_3(int layer) : m_layer(layer) {}

  void operator()(RLMachine& machine, int buf, string filename, int visible,
                  int x, int y, int pattern) {
    GraphicsSystem& gs = machine.system().graphics();
    GraphicsObject& obj = getGraphicsObject(machine, m_layer, buf);
    obj.setObjectData(gs.buildObjOfFile(machine, filename));
    obj.setVisible(visible);
    obj.setX(x);
    obj.setY(y);
    obj.setPattNo(pattern);
  }
};

// -----------------------------------------------------------------------

struct Obj_objOfFile_4 : public RLOp_Void_8<
  IntConstant_T, StrConstant_T, IntConstant_T, IntConstant_T,
  IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T>
{
  int m_layer;
  Obj_objOfFile_4(int layer) : m_layer(layer) {}

  void operator()(RLMachine& machine, int buf, string filename, int visible,
                  int x, int y, int pattern, int scrollX, int scrollY) {
    GraphicsSystem& gs = machine.system().graphics();
    GraphicsObject& obj = getGraphicsObject(machine, m_layer, buf);

    obj.setObjectData(gs.buildObjOfFile(machine, filename));
    obj.setVisible(visible);
    obj.setX(x);
    obj.setY(y);
    obj.setPattNo(pattern);
    obj.setScrollRateX(scrollX);
    obj.setScrollRateY(scrollY);
  }
};

// -----------------------------------------------------------------------

void addObjectCreationFunctions(RLModule& m, int layer)
{
  m.addOpcode(1000, 0, new Obj_objOfFile_0(layer));
  m.addOpcode(1000, 1, new Obj_objOfFile_1(layer));
  m.addOpcode(1000, 2, new Obj_objOfFile_2(layer));
  m.addOpcode(1000, 3, new Obj_objOfFile_3(layer));
  m.addOpcode(1000, 4, new Obj_objOfFile_4(layer));
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
