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
#include <iostream>
#include <iomanip>
#include <sstream>

#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>

using namespace std;
using namespace boost;
using namespace libReallive;

// -----------------------------------------------------------------------

template<typename LAYER>
struct Obj_objOfFile_0 : public RLOp_Void<IntConstant_T, StrConstant_T> {
  void operator()(RLMachine& machine, int buf, string filename) {
    GraphicsSystem& gs = machine.system().graphics();
    LAYER::get(machine, buf).setObjectData(gs.buildObjOfFile(filename));
    LAYER::get(machine, buf).setVisible(true);
    gs.markScreenAsDirty();
  }
};

// -----------------------------------------------------------------------

template<typename LAYER>
struct Obj_objOfFile_1 : public RLOp_Void<IntConstant_T, StrConstant_T, 
                                          IntConstant_T> {
  void operator()(RLMachine& machine, int buf, string filename, int visible) {
    GraphicsSystem& gs = machine.system().graphics();
    LAYER::get(machine, buf).setObjectData(gs.buildObjOfFile(filename));
    LAYER::get(machine, buf).setVisible(visible);
    gs.markScreenAsDirty();
  }
};

// -----------------------------------------------------------------------

template<typename LAYER>
struct Obj_objOfFile_2 : public RLOp_Void<IntConstant_T, StrConstant_T, 
                                          IntConstant_T, 
                                          IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, int buf, string filename, int visible,
                  int x, int y) {
    GraphicsSystem& gs = machine.system().graphics();
    LAYER::get(machine, buf).setObjectData(gs.buildObjOfFile(filename));
    LAYER::get(machine, buf).setVisible(visible);
    LAYER::get(machine, buf).setX(x);
    LAYER::get(machine, buf).setY(y);
    gs.markScreenAsDirty();
  }
};

// -----------------------------------------------------------------------

template<typename LAYER>
struct Obj_objOfFile_3 : public RLOp_Void<IntConstant_T, StrConstant_T, 
                                          IntConstant_T, IntConstant_T,
                                          IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, int buf, string filename, int visible,
                  int x, int y, int pattern) {
    GraphicsSystem& gs = machine.system().graphics();
    LAYER::get(machine, buf).setObjectData(gs.buildObjOfFile(filename));
    LAYER::get(machine, buf).setVisible(visible);
    LAYER::get(machine, buf).setX(x);
    LAYER::get(machine, buf).setY(y);
    LAYER::get(machine, buf).setPattNo(pattern);
    gs.markScreenAsDirty();
  }
};

// -----------------------------------------------------------------------

template<typename LAYER>
struct Obj_objOfFile_4 : public RLOp_Void<
  IntConstant_T, StrConstant_T, IntConstant_T, IntConstant_T,
  IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T>
{
  void operator()(RLMachine& machine, int buf, string filename, int visible,
                  int x, int y, int pattern, int scrollX, int scrollY) {
    GraphicsSystem& gs = machine.system().graphics();
    LAYER::get(machine, buf).setObjectData(gs.buildObjOfFile(filename));
    LAYER::get(machine, buf).setVisible(visible);
    LAYER::get(machine, buf).setX(x);
    LAYER::get(machine, buf).setY(y);
    LAYER::get(machine, buf).setPattNo(pattern);
    LAYER::get(machine, buf).setScrollRateX(scrollX);
    LAYER::get(machine, buf).setScrollRateY(scrollY);
    gs.markScreenAsDirty();
  }
};

// -----------------------------------------------------------------------

template<typename LAYER>
void addObjectCreationFunctions(RLModule& m)
{
  m.addOpcode(1000, 0, new Obj_objOfFile_0<LAYER>());
  m.addOpcode(1000, 1, new Obj_objOfFile_1<LAYER>());
  m.addOpcode(1000, 2, new Obj_objOfFile_2<LAYER>());
  m.addOpcode(1000, 3, new Obj_objOfFile_3<LAYER>());
  m.addOpcode(1000, 4, new Obj_objOfFile_4<LAYER>());
}

// -----------------------------------------------------------------------

ObjFgCreationModule::ObjFgCreationModule()
  : RLModule("ObjFgCreation", 1, 71)
{
  addObjectCreationFunctions<FG_LAYER>(*this);
}

// -----------------------------------------------------------------------

ObjBgCreationModule::ObjBgCreationModule()
  : RLModule("ObjBgCreation", 1, 72)
{   
  addObjectCreationFunctions<BG_LAYER>(*this);
}
