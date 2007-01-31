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

#ifndef __Module_Obj_hpp__
#define __Module_Obj_hpp__

/**
 * @file   Module_Obj.hpp
 * @author Elliot Glaysher
 * @date   Mon Jan  1 20:48:43 2007
 * 
 * @brief  Helper templates used in modules that work with objects.
 * 
 */

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include "MachineBase/RLMachine.hpp"
#include "MachineBase/RLOperation.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/GraphicsObject.hpp"


// -----------------------------------------------------------------------

class GraphicsObject;

// -----------------------------------------------------------------------

/**
 * Helper template used by Obj* definitions; work on the foreground.
 */
struct FG_LAYER {
  /// Getter
  static GraphicsObject& get(RLMachine& machine, int objNum) {
    return machine.system().graphics().getFgObject(objNum);
  }
};

// -----------------------------------------------------------------------

/**
 * Helper template used by Obj* definitions; work on the background.
 */
struct BG_LAYER {
  static GraphicsObject& get(RLMachine& machine, int objNum) {
    return machine.system().graphics().getBgObject(objNum);
  }
};

// -----------------------------------------------------------------------

/** 
 * Specialized form of Op_SetToIncomingInt to deal with looking up
 * object from the Obj* helper templates; since a lot of Object
 * related functions simply call a setter.
 *
 * This template magic saves having to write out 25 - 30 operation
 * structs.
 */
template<typename LAYER, typename SETTYPE = int>
class Obj_SetOneIntOnObj : public RLOp_Void< IntConstant_T, IntConstant_T > {
  /// The function signature for the setter function
  typedef void(GraphicsObject::*Setter)(const SETTYPE);

  /// The setter function to call on Op_SetToIncoming::reference when
  /// called.
  Setter setter;

public:
  Obj_SetOneIntOnObj(Setter s)
    : setter(s) 
  {}

  void operator()(RLMachine& machine, int buf, int incoming) 
  {
    ((LAYER::get(machine, buf)).*(setter))(incoming);
  }
};


/** 
 * Specialized form of Op_SetToIncomingInt to deal with looking up
 * object from the Obj* helper templates; since a lot of Object
 * related functions simply call a setter.
 *
 * This template magic saves having to write out 25 - 30 operation
 * structs.
 */
template<typename LAYER, typename SETTYPE = int>
class Obj_SetTwoIntOnObj : public RLOp_Void< IntConstant_T, IntConstant_T, IntConstant_T > {
  /// The function signature for the setter function
  typedef void(GraphicsObject::*Setter)(const SETTYPE);

  /// The setter functions to call on Op_SetToIncoming::reference when
  /// called.
  Setter setterOne;
  Setter setterTwo;

public:
  Obj_SetTwoIntOnObj(Setter one, Setter two)
    : setterOne(one), setterTwo(two) 
  {}

  void operator()(RLMachine& machine, int buf, int incomingOne, int incomingTwo) 
  {
    ((LAYER::get(machine, buf)).*(setterOne))(incomingOne);
    ((LAYER::get(machine, buf)).*(setterTwo))(incomingTwo);
  }
};


#endif 
