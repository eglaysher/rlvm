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

#include "Utilities.h"

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

void loadImageToDC1(GraphicsSystem& graphics,
                    const std::string& fileName,
                    int x, int y, int width, int height,
                    int dx, int dy, int opacity)
{
  Surface& dc0 = graphics.getDC(0);
  Surface& dc1 = graphics.getDC(1);

  // Inclusive ranges are a monstrosity to computer people
  width++;
  height++;

  dc0.blitToSurface(dc1,
                    0, 0, dc0.width(), dc0.height(),
                    0, 0, dc0.width(), dc0.height(),
                    255);

  // Load the section of the image file on top of dc1
  scoped_ptr<Surface> surface(graphics.loadSurfaceFromFile(fileName));
  surface->blitToSurface(graphics.getDC(1),
                         x, y, width, height,
                         dx, dy, width, height,
                         opacity);
}

}

// -----------------------------------------------------------------------

struct Grp_allocDC : public RLOp_Void< IntConstant_T, IntConstant_T,
                                       IntConstant_T > {
  void operator()(RLMachine& machine, int dc, int width, int height) {
    machine.system().graphics().allocateDC(dc, width, height);
  }
};

// -----------------------------------------------------------------------

struct Grp_freeDC : public RLOp_Void< IntConstant_T > {
  void operator()(RLMachine& machine, int dc) {
    machine.system().graphics().freeDC(dc);
  }
};

// -----------------------------------------------------------------------

struct Grp_wipe : public RLOp_Void< IntConstant_T, IntConstant_T,
                                    IntConstant_T, IntConstant_T > {
  void operator()(RLMachine& machine, int dc, int r, int g, int b) {
    machine.system().graphics().wipe(dc, r, g, b);
  }
};


// -----------------------------------------------------------------------
// GRP COMMANDS
// -----------------------------------------------------------------------


// -----------------------------------------------------------------------

struct Grp_grpOpen_0 : public RLOp_Void< StrConstant_T, IntConstant_T > {
  void operator()(RLMachine& machine, string filename, int effectNum) {
    vector<int> selEffect = machine.system().gameexe()("SEL", effectNum).
      to_intVector();

    GraphicsSystem& graphics = machine.system().graphics();
    if(filename[0] == '?') filename = graphics.defaultBgrName();
    filename = findFile(filename);

    grpToRecCoordinates(selEffect[0], selEffect[1], 
                        selEffect[2], selEffect[3]);
    loadImageToDC1(graphics, filename,
                   selEffect[0], selEffect[1], selEffect[2], selEffect[3],
                   selEffect[4], selEffect[5], selEffect[14]);

    // Set the long operation for the correct transition long operation
    machine.setLongOperation(EffectFactory::buildFromSEL(machine, effectNum));
  }
};

// -----------------------------------------------------------------------

/// @todo factor out the common code between grpOpens!
struct Grp_grpOpen_1 : public RLOp_Void< StrConstant_T, IntConstant_T, 
                                         IntConstant_T > {
  void operator()(RLMachine& machine, string filename, int effectNum, 
                  int opacity)
  {
    vector<int> selEffect = machine.system().gameexe()("SEL", effectNum).
      to_intVector();

    GraphicsSystem& graphics = machine.system().graphics();
    if(filename[0] == '?') filename = graphics.defaultBgrName();
    filename = findFile(filename);

    grpToRecCoordinates(selEffect[0], selEffect[1], 
                        selEffect[2], selEffect[3]);
    loadImageToDC1(graphics, filename,
                   selEffect[0], selEffect[1], selEffect[2], selEffect[3],
                   selEffect[4], selEffect[5], selEffect[14]);

    // Set the long operation for the correct transition long operation
    machine.setLongOperation(EffectFactory::buildFromSEL(machine, effectNum));
  }
};

// -----------------------------------------------------------------------

struct Grp_grpOpen_2 : public RLOp_Void< 
  StrConstant_T, IntConstant_T, IntConstant_T, IntConstant_T, 
  IntConstant_T, IntConstant_T, IntConstant_T>
{
  void operator()(RLMachine& machine, string filename, int effectNum, 
                  int x1, int y1, int x2, int y2, int dx, int dy)
  {
    int opacity = machine.system().gameexe()("SEL", effectNum).
      to_intVector().at(14);

    GraphicsSystem& graphics = machine.system().graphics();
    if(filename[0] == '?') filename = graphics.defaultBgrName();
    filename = findFile(filename);

    grpToRecCoordinates(x1, y1, x2, y2);
    loadImageToDC1(graphics, filename, x1, y1, x2, y2, dx, dy, opacity);

    // Set the long operation for the correct transition long operation
    machine.setLongOperation(EffectFactory::buildFromSEL(machine, effectNum));
  }
};

// -----------------------------------------------------------------------

struct Grp_grpOpen_3 : public RLOp_Void< 
  StrConstant_T, IntConstant_T, IntConstant_T, IntConstant_T, 
  IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T>
{
  void operator()(RLMachine& machine, string filename, int effectNum, 
                  int x1, int y1, int x2, int y2, int dx, int dy, int opacity)
  {
    GraphicsSystem& graphics = machine.system().graphics();
    if(filename[0] == '?') filename = graphics.defaultBgrName();
    filename = findFile(filename);

    grpToRecCoordinates(x1, y1, x2, y2);
    loadImageToDC1(graphics, filename, x1, y1, x2, y2, dx, dy, opacity);

    // Set the long operation for the correct transition long operation
    machine.setLongOperation(EffectFactory::buildFromSEL(machine, effectNum));
  }
};

// -----------------------------------------------------------------------

struct Grp_grpOpen_4 : public RLOp_Void<
  StrConstant_T, IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T,
  IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T,
  IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T,
  IntConstant_T, IntConstant_T>
{
  void operator()(RLMachine& machine, string filename, int effectNum,
                  int x1, int y1, int x2, int y2, int dx, int dy,
                  int time, int style, int direction, int interpolation,
                  int xsize, int ysize, int a, int b, int opacity, int c)
  {
    GraphicsSystem& graphics = machine.system().graphics();
    if(filename[0] == '?') filename = graphics.defaultBgrName();
    filename = findFile(filename);

    grpToRecCoordinates(x1, y1, x2, y2);
    loadImageToDC1(graphics, filename, x1, y1, x2, y2, dx, dy, opacity);

    // Set the long operation for the correct transition long operation
    machine.setLongOperation(
      EffectFactory::build(machine, time, style, direction, 
                           interpolation, xsize, ysize, a, b, opacity, c));
  }
};

// -----------------------------------------------------------------------
// REC COMMANDS
// -----------------------------------------------------------------------


// -----------------------------------------------------------------------

struct Grp_recOpen_0 : public RLOp_Void< StrConstant_T, IntConstant_T > {
  void operator()(RLMachine& machine, string filename, int effectNum) {
    vector<int> selEffect = machine.system().gameexe()("SELR", effectNum).
      to_intVector();

    GraphicsSystem& graphics = machine.system().graphics();
    if(filename[0] == '?') filename = graphics.defaultBgrName();
    filename = findFile(filename);

    loadImageToDC1(graphics, filename,
                   selEffect[0], selEffect[1], selEffect[2], selEffect[3],
                   selEffect[4], selEffect[5], selEffect[14]);

    // Set the long operation for the correct transition long operation
    machine.setLongOperation(EffectFactory::buildFromSELR(machine, effectNum));
  }
};

// -----------------------------------------------------------------------

struct Grp_recOpen_1 : public RLOp_Void< StrConstant_T, IntConstant_T, 
                                         IntConstant_T > {
  void operator()(RLMachine& machine, string filename, int effectNum, 
                  int opacity)
  {
    vector<int> selEffect = machine.system().gameexe()("SELR", effectNum).
      to_intVector();

    GraphicsSystem& graphics = machine.system().graphics();
    if(filename[0] == '?') filename = graphics.defaultBgrName();
    filename = findFile(filename);

    loadImageToDC1(graphics, filename,
                   selEffect[0], selEffect[1], selEffect[2], selEffect[3],
                   selEffect[4], selEffect[5], selEffect[14]);

    // Set the long operation for the correct transition long operation
    machine.setLongOperation(EffectFactory::buildFromSELR(machine, effectNum));
  }
};

// -----------------------------------------------------------------------

struct Grp_recOpen_2 : public RLOp_Void< 
  StrConstant_T, IntConstant_T, IntConstant_T, IntConstant_T, 
  IntConstant_T, IntConstant_T, IntConstant_T>
{
  void operator()(RLMachine& machine, string filename, int effectNum, 
                  int x1, int y1, int x2, int y2, int dx, int dy)
  {
    int opacity = machine.system().gameexe()("SELR", effectNum).
      to_intVector().at(14);

    GraphicsSystem& graphics = machine.system().graphics();
    if(filename[0] == '?') filename = graphics.defaultBgrName();
    filename = findFile(filename);

    loadImageToDC1(graphics, filename, x1, y1, x2, y2, dx, dy, opacity);

    // Set the long operation for the correct transition long operation
    machine.setLongOperation(EffectFactory::buildFromSELR(machine, effectNum));
  }
};

// -----------------------------------------------------------------------

struct Grp_recOpen_3 : public RLOp_Void< 
  StrConstant_T, IntConstant_T, IntConstant_T, IntConstant_T, 
  IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T>
{
  void operator()(RLMachine& machine, string filename, int effectNum, 
                  int x1, int y1, int x2, int y2, int dx, int dy, int opacity)
  {
    GraphicsSystem& graphics = machine.system().graphics();
    if(filename[0] == '?') filename = graphics.defaultBgrName();
    filename = findFile(filename);

    loadImageToDC1(graphics, filename, x1, y1, x2, y2, dx, dy, opacity);

    // Set the long operation for the correct transition long operation
    machine.setLongOperation(EffectFactory::buildFromSELR(machine, effectNum));
  }
};

// -----------------------------------------------------------------------

struct Grp_recOpen_4 : public RLOp_Void<
  StrConstant_T, IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T,
  IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T,
  IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T,
  IntConstant_T, IntConstant_T>
{
  void operator()(RLMachine& machine, string filename, int effectNum,
                  int x1, int y1, int x2, int y2, int dx, int dy,
                  int time, int style, int direction, int interpolation,
                  int xsize, int ysize, int a, int b, int opacity, int c)
  {
    GraphicsSystem& graphics = machine.system().graphics();
    if(filename[0] == '?') filename = graphics.defaultBgrName();
    filename = findFile(filename);

    loadImageToDC1(graphics, filename, x1, y1, x2, y2, dx, dy, opacity);

    // Set the long operation for the correct transition long operation
    machine.setLongOperation(
      EffectFactory::build(machine, time, style, direction, 
                           interpolation, xsize, ysize, a, b, opacity, c));
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
  addOpcode(76, 1, new Grp_grpOpen_1);
  addOpcode(76, 2, new Grp_grpOpen_2);
  addOpcode(76, 3, new Grp_grpOpen_3);
  addOpcode(76, 4, new Grp_grpOpen_4);

  addOpcode(1056, 0, new Grp_recOpen_0);
  addOpcode(1056, 1, new Grp_recOpen_1);
  addOpcode(1057, 2, new Grp_recOpen_2);
  addOpcode(1058, 3, new Grp_recOpen_3);
  addOpcode(1059, 4, new Grp_recOpen_4);
}
