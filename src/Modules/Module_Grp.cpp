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
 * @file   Module_Grp.cpp
 * @author Elliot Glaysher
 * @date   Wed Oct  4 16:45:44 2006
 * 
 * @brief  Implements the Graphics module (mod<1:33>).
 */

#include "Modules/Module_Grp.hpp"
#include "MachineBase/RLOperation.hpp"
#include "MachineBase/RLMachine.hpp"

#include "Systems/Base/System.hpp"
#include "Systems/Base/GraphicsSystem.hpp"

#include "Modules/EffectFactory.hpp"

#include "libReallive/gameexe.h"

#include <iostream>

#include <boost/scoped_ptr.hpp>

using namespace std;
using namespace boost;

const int SEL_SIZE = 16;

// -----------------------------------------------------------------------

namespace {

string findFile(const std::string& fileName)
{
  // Hack until I do this correctly
  string file = "/Users/elliot/KANON_SE_ALL/g00/" + fileName;
  file += ".g00";
  return file;
}

}

// -----------------------------------------------------------------------

struct Grp_allocDC : public RLOp_Void_3< IntConstant_T, IntConstant_T,
                                         IntConstant_T > {
  void operator()(RLMachine& machine, int dc, int width, int height) {
    machine.system().graphics().allocateDC(dc, width, height);
  }
};

// -----------------------------------------------------------------------

struct Grp_freeDC : public RLOp_Void_1< IntConstant_T > {
  void operator()(RLMachine& machine, int dc) {
    machine.system().graphics().freeDC(dc);
  }
};

// -----------------------------------------------------------------------

struct Grp_wipe : public RLOp_Void_4< IntConstant_T, IntConstant_T,
                                      IntConstant_T, IntConstant_T > {
  void operator()(RLMachine& machine, int dc, int r, int g, int b) {
    machine.system().graphics().wipe(dc, r, g, b);
  }
};

// -----------------------------------------------------------------------

struct Grp_grpOpen_0 : public RLOp_Void_2< StrConstant_T, IntConstant_T > {
  void operator()(RLMachine& machine, string filename, int effectNum) {
    int opacity = machine.system().gameexe().getInt("SEL", effectNum, 14);

    GraphicsSystem& graphics = machine.system().graphics();
    if(filename[0] == '?') filename = graphics.defaultBgrName();
    filename = findFile(filename);
    cerr << "b(" << filename<< ")" << endl;
    // First, load the file to DC1.
    scoped_ptr<Surface> surface(graphics.loadSurfaceFromFile(filename));
    surface->blitToSurface(graphics.getDC(1),
                          0, 0, surface->width(), surface->height(),
                          0, 0, graphics.screenWidth(), graphics.screenHeight(),
                          // Modify this value later:
                          255);

    // Set the long operation for the correct transition long operation
    machine.setLongOperation(EffectFactory::buildFromSEL(machine, effectNum));
  }
};

// -----------------------------------------------------------------------

GrpModule::GrpModule()
  : RLModule("Grp", 1, 33)
{
  addOpcode(15, 0, new Grp_allocDC);
  addOpcode(16, 0, new Grp_freeDC);

  addOpcode(31, 0, new Grp_wipe);

  addOpcode(76, 0, new Grp_grpOpen_0);
}
