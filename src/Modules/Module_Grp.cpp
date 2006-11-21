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
 * @ingroup ModulesOpcodes
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

/**
 * @defgroup ModuleGrp The Graphics Module (mod<1:33>)
 * @ingroup ModulesOpcodes
 * 
 * Module that describes various graphics commands that operate on the DCs.
 *
 * @{
 */

// -----------------------------------------------------------------------

namespace {

/** 
 * Returns the full path to a g00 file for the basename of the file.
 * 
 * @param fileName The filename given in the source code.
 * @return The full path of the file
 * @todo This function is currently a hack and does not actually
 *       search for the file; it points directly into my copy of the
 *       Kannon all ages DVD.
 */
string findFile(const std::string& fileName)
{
  // Hack until I do this correctly
  string file = "/Users/elliot/KANON_SE_ALL/g00/" + fileName;
  file += ".g00";
  return file;
}

/** 
 * Performs half the grunt work of a recOpen command; Copies DC0 to
 * DC1, loads a graphics file, and then composites that file to DC1.
 * 
 * Note that it works in rec coordinate space; grp commands must
 * convert from grp coordinate space.
 * 
 * @param graphics The graphics system to composite on
 * @param fileName The full filename (as returned by findFile)
 * @param x Source X coordinate
 * @param y Source Y coordinate
 * @param width Width of area to composite
 * @param height Height of area to composite
 * @param dx Destination X coordinate
 * @param dy Destination Y coordinate
 * @param opacity Opacity in range from 0 - 255
 */
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

/** 
 * Implements op<1:Grp:00015, 0>, fun allocDC('DC', 'width', 'height').
 * 
 * Allocates a blank width × height bitmap in dc. Any DC apart from DC
 * 0 may be allocated thus, although DC 1 is never given a size
 * smaller than the screen resolution. Any previous contents of dc are
 * erased.
 */
struct Grp_allocDC : public RLOp_Void< IntConstant_T, IntConstant_T,
                                       IntConstant_T > {
  void operator()(RLMachine& machine, int dc, int width, int height) {
    machine.system().graphics().allocateDC(dc, width, height);
  }
};

// -----------------------------------------------------------------------

/** 
 * Implements op<1:Grp:00016, 0>, fun freeDC('DC').
 * 
 * Frees dc, releasing the memory allocated to it. DC may not be 0; if
 * it is 1, DC 1 will be blanked, but not released.
 */
struct Grp_freeDC : public RLOp_Void< IntConstant_T > {
  void operator()(RLMachine& machine, int dc) {
    machine.system().graphics().freeDC(dc);
  }
};

// -----------------------------------------------------------------------

/** 
 * Implements op<1:Grp:00031, 0>, fun wipe('DC', 'r', 'g', 'b')
 * 
 * Fills dc with the colour indicated by the given RGB triplet.
 */
struct Grp_wipe : public RLOp_Void< IntConstant_T, IntConstant_T,
                                    IntConstant_T, IntConstant_T > {
  void operator()(RLMachine& machine, int dc, int r, int g, int b) {
    machine.system().graphics().getDC(dc).wipe(r, g, b);
  }
};


// -----------------------------------------------------------------------
// GRP COMMANDS
// -----------------------------------------------------------------------

/** 
 * Implements op<1:Grp:00076, 0>, fun grpOpen(strC 'filename', '\#SEL').
 * 
 * Load and display a bitmap. @em filename is loaded into DC1, and
 * then is passed off to whatever transition effect, which will
 * perform some intermediary steps and then render DC1 to DC0.
 */
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

/** 
 * Implements op<1:Grp:00076, 1>, fun grpOpen(strC 'filename', '\#SEL', 'opacity').
 * 
 * Load and display a bitmap. @em filename is loaded into DC1 with
 * opacity @em opacity, and then is passed off to whatever transition
 * effect, which will perform some intermediary steps and then render
 * DC1 to DC0.
 *
 * @todo factor out the common code between grpOpens!
 */
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
                   selEffect[4], selEffect[5], opacity);

    // Set the long operation for the correct transition long operation
    machine.setLongOperation(EffectFactory::buildFromSEL(machine, effectNum));
  }
};

// -----------------------------------------------------------------------

/** 
 * Implements op<1:Grp:00076, 1>, fun grpOpen(strC 'filename', '\#SEL', 'opacity').
 * 
 * Load and display a bitmap. @em filename is loaded into DC1 with
 * opacity @em opacity, and then is passed off to whatever transition
 * effect, which will perform some intermediary steps and then render
 * DC1 to DC0.
 *
 * @todo Finish documentation
 */
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
                           interpolation, xsize, ysize, a, b, c));
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
                           interpolation, xsize, ysize, a, b, c));
  }
};

// -----------------------------------------------------------------------

GrpModule::GrpModule()
  : RLModule("Grp", 1, 33)
{
  addOpcode(15, 0, new Grp_allocDC);
  addOpcode(16, 0, new Grp_freeDC);

  // addOpcode(20, 0, new Grp_grpLoadMask);
  // addOpcode(30, 0, new Grp_grpTextout);

  addOpcode(31, 0, new Grp_wipe);
  // addOpcode(32, 0, new Grp_shake);

//  addOpcode(50, 0, new Grp_grpLoad_0);

  addOpcode(76, 0, new Grp_grpOpen_0);
  addOpcode(76, 1, new Grp_grpOpen_1);
  addOpcode(76, 2, new Grp_grpOpen_2);
  addOpcode(76, 3, new Grp_grpOpen_3);
  addOpcode(76, 4, new Grp_grpOpen_4);

  addOpcode(1056, 0, new Grp_recOpen_0);
  addOpcode(1056, 1, new Grp_recOpen_1);
  addOpcode(1056, 2, new Grp_recOpen_2);
  addOpcode(1056, 3, new Grp_recOpen_3);
  addOpcode(1056, 4, new Grp_recOpen_4);
}

// @}
