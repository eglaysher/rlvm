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

/**
 * @file   Module_Obj.cpp
 * @author Elliot Glaysher
 * @date   Thu Dec  7 19:44:24 2006
 * @ingroup ModulesOpcodes
 *
 * @brief Contains definitions for object handling functions for the
 * Modules 81 "ObjFg", 82 "ObjBg", 90 "ObjRange", and 91 "ObjBgRange".
 */

#include "Modules/Module_Obj.hpp"
#include "Modules/Module_ObjFgBg.hpp"
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


// List of functions we're goign to have to handle to be compatible
// with Kanon:
//
// objBgAdjust
// objBgAlpha
// objBgClear
// objBgDispRect
// objBgDriftOfFile
// objBgDriftOpts
// objBgMove
// objBgOfFile
// objBgOfFileGan
// objBgPattNo
// objBgShow
//
// objAdjust
// objAlpha
// objClear
// objCopyFgToBg
// objDelete
// objDispRect
// objDriftOfFile
// objDriftOpts
// objGetDims
// objGetPos
// objMove
// objOfFile
// objPattNo
// objShow

// -----------------------------------------------------------------------

template<typename LAYER>
struct Obj_adjust : RLOp_Void_4< IntConstant_T, IntConstant_T, IntConstant_T, 
                               IntConstant_T > {
  void operator()(RLMachine& machine, int buf, int idx, int x, int y) {
    LAYER::get(machine, buf).setXAdjustment(idx, x);
    LAYER::get(machine, buf).setYAdjustment(idx, y);
  }
};

// -----------------------------------------------------------------------

template<typename LAYER>
struct Obj_adjustX : RLOp_Void_3< IntConstant_T, IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, int buf, int idx, int x) {
    LAYER::get(machine, buf).setXAdjustment(idx, x);
  }
};

// -----------------------------------------------------------------------

template<typename LAYER>
struct Obj_adjustY : RLOp_Void_3< IntConstant_T, IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, int buf, int idx, int y) {
    LAYER::get(machine, buf).setYAdjustment(idx, y);
  }
};

// -----------------------------------------------------------------------

template<typename LAYER>
struct Obj_tint : RLOp_Void_4< IntConstant_T, IntConstant_T, IntConstant_T, 
                               IntConstant_T> {
  void operator()(RLMachine& machine, int buf, int r, int g, int b) {
    LAYER::get(machine, buf).setTintR(r);
    LAYER::get(machine, buf).setTintG(g);
    LAYER::get(machine, buf).setTintB(b);
  }
};

// -----------------------------------------------------------------------

template<typename LAYER>
struct Obj_colour : RLOp_Void_5< IntConstant_T, IntConstant_T, IntConstant_T, 
                                 IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, int buf, int r, int g, int b, int level) {
    LAYER::get(machine, buf).setColourR(r);
    LAYER::get(machine, buf).setColourG(g);
    LAYER::get(machine, buf).setColourB(b);
    LAYER::get(machine, buf).setColourLevel(level);
  }
};

// -----------------------------------------------------------------------

/**
 * Special adapter to make any of obj* and objBg* operation structs
 * into an objRange* or objRangeBg* struct.
 * 
 * We extract the first two expression pieces from the incoming
 * command and assume that they are integers and are the bounds on the
 * object number. We then construct a set of parameters to pass to the
 * real implementation.
 *
 * This is certainly not the most efficient way to do it, but it cuts
 * down on a duplicated operation struct for each obj* and objBg*
 * function, alowing us to just use this adapter with the already
 * defined operations.
 */
struct ObjRangeAdapter : RLOp_SpecialCase {
  /// Keep a copy of the operation that we wrap
  scoped_ptr<RLOperation> handler;

  ObjRangeAdapter(RLOperation* in) : handler(in) { }

  void operator()(RLMachine& machine, const libReallive::CommandElement& ff) {
    ptr_vector<ExpressionPiece> range;
    ptr_vector<ExpressionPiece> parameters;

    // Range check the data
    int totalParameters = ff.param_count();
    if(totalParameters < 2) 
    {
      throw Error("Less then two arguments to an objRange function!");
    }

    // Get the first two elements as the range
    const char* rawdata = ff.get_param(0).c_str();
    range.push_back(libReallive::get_data(rawdata));
    rawdata = ff.get_param(0).c_str();
    range.push_back(libReallive::get_data(rawdata));

    for(int i = 0; i < totalParameters; ++i) 
    {
      addParameterTo(ff.get_param(i), parameters);
    }

    // BIG WARNING ABOUT THE FOLLOWING CODE: Note that we copy half of
    // what RLOperation.dispatchFunction() does; we manually call the
    // subclass's disptach() so that we can get around the automated
    // incrementing of the instruction pointer.
    int lowerRange = range[0].integerValue(machine);
    int upperRange = range[1].integerValue(machine);
    for(int i = lowerRange; i <= upperRange; ++i) {
      // Create a new list of expression pieces that contain the
      // current object we're dealing with and 
      ptr_vector<ExpressionPiece> currentInstantiation;
      currentInstantiation.push_back(new IntegerConstant(i));
      for(ptr_vector<ExpressionPiece>::iterator it = parameters.begin();
          it != parameters.end(); ++it) {
        currentInstantiation.push_back(it->clone());
      }

      // Verify it against the type
      if(!handler->checkTypes(machine, currentInstantiation)) {
        throw Error("Expected type mismatch in parameters.");
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

template<typename LAYER>
void addObjectFunctions(RLModule& m)
{
  m.addOpcode(1000, 0, "objMove", new Obj_SetTwoIntOnObj<LAYER>(
                &GraphicsObject::setX, 
                &GraphicsObject::setY));
  m.addOpcode(1001, 0, new Obj_SetOneIntOnObj<LAYER>(&GraphicsObject::setX));
  m.addOpcode(1002, 0, new Obj_SetOneIntOnObj<LAYER>(&GraphicsObject::setY));
  m.addOpcode(1003, 0, new Obj_SetOneIntOnObj<LAYER>(&GraphicsObject::setAlpha));
  m.addOpcode(1004, 0, new Obj_SetOneIntOnObj<LAYER, bool>(&GraphicsObject::setVisible));
//  m.addOpcode(1005, 0, new Obj_dispArea_0<LAYER>);
//  m.addOpcode(1005, 1, new Obj_dispArea_1<LAYER>);
  m.addOpcode(1006, 0, new Obj_adjust<LAYER>);
  m.addOpcode(1007, 0, new Obj_adjustX<LAYER>);
  m.addOpcode(1008, 0, new Obj_adjustY<LAYER>);
  m.addOpcode(1009, 0, new Obj_SetOneIntOnObj<LAYER>(&GraphicsObject::setMono));
  m.addOpcode(1010, 0, new Obj_SetOneIntOnObj<LAYER>(&GraphicsObject::setInvert));
  m.addOpcode(1011, 0, new Obj_SetOneIntOnObj<LAYER>(&GraphicsObject::setLight));
  m.addOpcode(1012, 0, new Obj_tint<LAYER>);
  m.addOpcode(1013, 0, new Obj_SetOneIntOnObj<LAYER>(&GraphicsObject::setTintR));
  m.addOpcode(1014, 0, new Obj_SetOneIntOnObj<LAYER>(&GraphicsObject::setTintG));
  m.addOpcode(1015, 0, new Obj_SetOneIntOnObj<LAYER>(&GraphicsObject::setTintB));
  m.addOpcode(1016, 0, new Obj_colour<LAYER>);
  m.addOpcode(1017, 0, new Obj_SetOneIntOnObj<LAYER>(&GraphicsObject::setColourR));
  m.addOpcode(1018, 0, new Obj_SetOneIntOnObj<LAYER>(&GraphicsObject::setColourG));
  m.addOpcode(1019, 0, new Obj_SetOneIntOnObj<LAYER>(&GraphicsObject::setColourB));
  m.addOpcode(1020, 0, new Obj_SetOneIntOnObj<LAYER>(&GraphicsObject::setColourLevel));
  m.addOpcode(1021, 0, new Obj_SetOneIntOnObj<LAYER>(&GraphicsObject::setCompositeMode));


/*  m.addOpcode(1028, 0, new  */
  m.addOpcode(1030, 0, new Obj_SetOneIntOnObj<LAYER>(&GraphicsObject::setScrollRateX));
  m.addOpcode(1031, 0, new Obj_SetOneIntOnObj<LAYER>(&GraphicsObject::setScrollRateY));

  m.addOpcode(1036, 0, new Obj_SetOneIntOnObj<LAYER>(&GraphicsObject::setVert));
  m.addOpcode(1039, 0, new Obj_SetOneIntOnObj<LAYER>(&GraphicsObject::setPattNo));

  m.addOpcode(1046, 0, new Obj_SetTwoIntOnObj<LAYER>(
                &GraphicsObject::setWidth,
                &GraphicsObject::setHeight));
  m.addOpcode(1047, 0, new Obj_SetOneIntOnObj<LAYER>(&GraphicsObject::setWidth));
  m.addOpcode(1048, 0, new Obj_SetOneIntOnObj<LAYER>(&GraphicsObject::setHeight));
  m.addOpcode(1049, 0, new Obj_SetOneIntOnObj<LAYER>(&GraphicsObject::setRotation));


  m.addOpcode(1053, 0, "objOrigin", new Obj_SetTwoIntOnObj<LAYER>(
                &GraphicsObject::setXOrigin,
                &GraphicsObject::setYOrigin));
  m.addOpcode(1054, 0, new Obj_SetOneIntOnObj<LAYER>(&GraphicsObject::setXOrigin));
  m.addOpcode(1055, 0, new Obj_SetOneIntOnObj<LAYER>(&GraphicsObject::setYOrigin));
}

// -----------------------------------------------------------------------

// @todo Make this reflect the normal ones. I've let this fall out of
//       sync with the previous function.
template<typename LAYER>
void addRangeObjectFunctions(RLModule& m)
{
//  m.addOpcode(1000, 0, new ObjRangeAdapter<Obj_move<LAYER> >( new Obj_move<LAYER> ));
  m.addOpcode(1001, 0, new ObjRangeAdapter( 
                new Obj_SetOneIntOnObj<LAYER>(&GraphicsObject::setX) ));
  m.addOpcode(1002, 0, new ObjRangeAdapter( 
                new Obj_SetOneIntOnObj<LAYER>(&GraphicsObject::setY) ));
  m.addOpcode(1003, 0, new ObjRangeAdapter( 
                new Obj_SetOneIntOnObj<LAYER>(&GraphicsObject::setAlpha) ));
  m.addOpcode(1004, 0, new ObjRangeAdapter( 
                new Obj_SetOneIntOnObj<LAYER, bool>(&GraphicsObject::setVisible) ));
}

// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------

ObjFgModule::ObjFgModule()
  : RLModule("ObjFg", 1, 81) 
{
  addObjectFunctions<FG_LAYER>(*this);
}

// -----------------------------------------------------------------------

ObjBgModule::ObjBgModule()
  : RLModule("ObjBg", 1, 82)
{
  addObjectFunctions<BG_LAYER>(*this);
}

// -----------------------------------------------------------------------

ObjRangeFgModule::ObjRangeFgModule()
  : RLModule("ObjRangeFg", 1, 90)
{
  addRangeObjectFunctions<FG_LAYER>(*this);
}

// -----------------------------------------------------------------------

ObjRangeBgModule::ObjRangeBgModule()
  : RLModule("ObjRangeBg", 1, 91)
{
  addRangeObjectFunctions<BG_LAYER>(*this);
}
