// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006, 2007 Elliot Glaysher
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

#include "Precompiled.hpp"

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
#include "MachineBase/RLOperation/Argc_T.hpp"
#include "MachineBase/RLOperation/Complex_T.hpp"
#include "MachineBase/RLOperation/Special_T.hpp"
#include "MachineBase/RLOperation/DefaultValue.hpp"

#include "MachineBase/RLMachine.hpp"

#include "Systems/Base/System.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/GraphicsStackFrame.hpp"
#include "Systems/Base/Surface.hpp"
#include "Systems/Base/TextSystem.hpp"

#include "Modules/Effect.hpp"
#include "Modules/EffectFactory.hpp"
#include "Modules/ZoomLongOperation.hpp"

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

namespace graphicsStack {
const string GRP_LOAD = "grpLoad";
const std::string GRP_OPENBG = "grpOpenBg";
}

using namespace graphicsStack;

// -----------------------------------------------------------------------

namespace {

void blitDC1toDC0(RLMachine& machine)
{
  GraphicsSystem& graphics = machine.system().graphics();

  boost::shared_ptr<Surface> src = graphics.getDC(1);
  boost::shared_ptr<Surface> dst = graphics.getDC(0);

  // Blit DC1 onto DC0, with full opacity, and end the operation
  src->blitToSurface(
    *dst,
    0, 0, src->width(), src->height(),
    0, 0, dst->width(), dst->height(), 255);

  // Now force a screen refresh
  machine.system().graphics().markScreenForRefresh();
}

// -----------------------------------------------------------------------

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
 * @param useAlpha Whether to use the alpha
 */
void loadImageToDC1(GraphicsSystem& graphics,
                    const std::string& fileName,
                    int x, int y, int width, int height,
                    int dx, int dy, int opacity, bool useAlpha)
{
  shared_ptr<Surface> dc0 = graphics.getDC(0);
  shared_ptr<Surface> dc1 = graphics.getDC(1);

  // Inclusive ranges are a monstrosity to computer people
  width++;
  height++;

  dc0->blitToSurface(*dc1,
                     0, 0, dc0->width(), dc0->height(),
                     0, 0, dc0->width(), dc0->height(),
                     255);

  // Load the section of the image file on top of dc1
  shared_ptr<Surface> surface(graphics.loadSurfaceFromFile(fileName));
  surface->blitToSurface(*graphics.getDC(1),
                         x, y, width, height,
                         dx, dy, width, height,
                         opacity, useAlpha);
}

// -----------------------------------------------------------------------

void loadDCToDC1(GraphicsSystem& graphics,
                 int srcDc,
                 int x, int y, int width, int height,
                 int dx, int dy, int opacity)
{
  shared_ptr<Surface> dc0 = graphics.getDC(0);
  shared_ptr<Surface> dc1 = graphics.getDC(1);
  shared_ptr<Surface> src = graphics.getDC(srcDc);

  // Inclusive ranges are a monstrosity to computer people
  width++;
  height++;

  dc0->blitToSurface(*dc1,
                     0, 0, dc0->width(), dc0->height(),
                     0, 0, dc0->width(), dc0->height(),
                     255);
  src->blitToSurface(*dc1,
                     x, y, width, height,
                     dx, dy, width, height,
                     opacity, false);
}

// -----------------------------------------------------------------------

void handleOpenBgFileName(
  RLMachine& machine,
  std::string fileName,
  int x1, int y1, int x2, int y2,
  int dx, int dy, int opacity, bool useAlpha)
{
  GraphicsSystem& graphics = machine.system().graphics();

  if(fileName != "?")
  {
    if(fileName == "???") fileName = graphics.defaultGrpName();
    fileName = findFile(machine, fileName);
    
    loadImageToDC1(graphics, fileName, x1, y1, x2, y2, dx, dy, 
                   opacity, useAlpha);
  }
}

}

// -----------------------------------------------------------------------

/** 
 * Abstract interface for a space. Taken as a parameter to most Grp
 * classes.
 */
struct SPACE {
  virtual ~SPACE() {}
  virtual void translateToRec(int x1, int y1, int& x2, int& y2) = 0;
};

// -----------------------------------------------------------------------

/**
 * A large number of the operation structs in the Grp module are
 * written in a generic way so that they can be done in either rec or
 * grp coordinate space. GRP_SPACE or REC_SPACE are passed as
 * parameters to these operations.
 * 
 * @see REC_SPACE
 * @see Grp_open_0
 */
struct GRP_SPACE : public SPACE {
  /** 
   * Changes the coordinate types. All operations internally are done in
   * rec coordinates, (x, y, width, height). The GRP functions pass
   * parameters of the format (x1, y1, x2, y2).
   * 
   * @param x1 X coordinate. Not changed by this function
   * @param y1 Y coordinate. Not changed by this function
   * @param x2 X2. In place changed to width.
   * @param y2 Y2. In place changed to height.
   */
  virtual void translateToRec(int x1, int y1, int& x2, int& y2)
  {
    x2 = x2 - x1;
    y2 = y2 - y1;
  }

  static SPACE& get() {
    static GRP_SPACE space;
    return space;
  }
};

// -----------------------------------------------------------------------

struct REC_SPACE : public SPACE {
  /** 
   * Don't do anything; leave the incoming coordinates as they are.
   */
  virtual void translateToRec(int x1, int y1, int& x2, int& y2)
  {
  }

  static SPACE& get() {
    static REC_SPACE space;
    return space;
  }
};


// -----------------------------------------------------------------------

/** 
 * Implements op<1:Grp:00015, 0>, fun allocDC('DC', 'width', 'height').
 * 
 * Allocates a blank width × height bitmap in dc. Any DC apart from DC
 * 0 may be allocated thus, although DC 1 is never given a size
 * smaller than the screen resolution. Any previous contents of dc are
 * erased.
 */
struct Grp_allocDC : public RLOp_Void_3< IntConstant_T, IntConstant_T,
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
struct Grp_freeDC : public RLOp_Void_1< IntConstant_T > {
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
struct Grp_wipe : public RLOp_Void_4< IntConstant_T, IntConstant_T,
                                      IntConstant_T, IntConstant_T > {
  void operator()(RLMachine& machine, int dc, int r, int g, int b) {
    machine.system().graphics().getDC(dc)->fill(r, g, b, 255);
  }
};

// -----------------------------------------------------------------------
// {grp,rec}Load Commands
// -----------------------------------------------------------------------

/**
 * Implements the {grp,rec}(Mask)?Load family of functions.
 *
 * Loads filename into dc; note that filename may not be '???'. 
 *
 * @note Since this function deals with the entire screen (and
 * therefore doesn't need to worry about the difference between
 * grp/rec coordinate space), we write one function for both versions.
 */
struct Grp_load_1 : public RLOp_Void_3< StrConstant_T, IntConstant_T, 
                                        DefaultIntValue_T<255> > {
  bool m_useAlpha;
  Grp_load_1(bool in) : m_useAlpha(in) {}

  void operator()(RLMachine& machine, string filename, int dc, int opacity) {
    GraphicsSystem& graphics = machine.system().graphics();

    graphics.addGraphicsStackFrame(GRP_LOAD)
      .setFilename(filename).setTargetDC(dc).setOpacity(opacity);

    filename = findFile(machine, filename);
    shared_ptr<Surface> surface(graphics.loadSurfaceFromFile(filename));

    if(dc != 0 && dc != 1)
      graphics.allocateDC(dc,
                          max(surface->width(), graphics.screenWidth()),
                          max(surface->height(), graphics.screenHeight()));

    surface->blitToSurface(*graphics.getDC(dc),
                           0, 0, surface->width(), surface->height(),
                           0, 0, surface->width(), surface->height(),
                           opacity, m_useAlpha);

  }
};

// -----------------------------------------------------------------------

/**
 * Implements op<1:Grp:00050, 3>, fun grpLoad(strC 'filename', 'DC',
 * 'x1', 'y1', 'x2', 'y2', 'dx', 'dy', 'alpha').
 *
 * Loads filename into dc; note that filename may not be '???'. Using
 * this form, the given area of the bitmap is loaded at the given
 * location.
 */
struct Grp_load_3 : public RLOp_Void_9< 
  StrConstant_T, IntConstant_T, IntConstant_T, IntConstant_T, 
  IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T,
  DefaultIntValue_T<255> > 
{
  bool m_useAlpha;
  SPACE& m_space;
  Grp_load_3(bool in, SPACE& space) : m_useAlpha(in), m_space(space) {}

  void operator()(RLMachine& machine, string filename, int dc,
                  int x1, int y1, int x2, int y2, int dx, int dy, int opacity) {
    filename = findFile(machine, filename);
    GraphicsSystem& graphics = machine.system().graphics();
    shared_ptr<Surface> surface(graphics.loadSurfaceFromFile(filename));
    m_space.translateToRec(x1, y1, x2, y2);

    if(dc != 0 && dc != 1)
      graphics.allocateDC(dc,
                          max(surface->width(), graphics.screenWidth()),
                          max(surface->height(), graphics.screenHeight()));

    surface->blitToSurface(*graphics.getDC(dc),
                           x1, y1, x2, y2, dx, dy, x2, y2, opacity, m_useAlpha);    
  }
};

// -----------------------------------------------------------------------
// {grp,rec}Display
// -----------------------------------------------------------------------

struct Grp_display_1
  : public RLOp_Void_3< IntConstant_T, IntConstant_T, IntConstant_T > 
{
  void operator()(RLMachine& machine, int dc, int effectNum, int opacity) 
  {
    vector<int> selEffect = getSELEffect(machine, effectNum);

    GraphicsSystem& graphics = machine.system().graphics();
    loadDCToDC1(graphics, dc,
                selEffect[0], selEffect[1], selEffect[2], selEffect[3],
                selEffect[4], selEffect[5], opacity);

    // Set the long operation for the correct transition long operation
    shared_ptr<Surface> dc0 = graphics.getDC(0);
    shared_ptr<Surface> dc1 = graphics.getDC(1);
    LongOperation* lop = EffectFactory::buildFromSEL(machine, dc1, dc0, effectNum);
    decorateEffectWithBlit(lop, dc1, dc0);
    machine.pushLongOperation(lop);
  }
};

// -----------------------------------------------------------------------

struct Grp_display_0 : public RLOp_Void_2< IntConstant_T, IntConstant_T > {
  Grp_display_1 m_delegate;

  void operator()(RLMachine& machine, int dc, int effectNum) {
    vector<int> selEffect = getSELEffect(machine, effectNum);
    m_delegate(machine, dc, effectNum, selEffect.at(14));
  }
};

// -----------------------------------------------------------------------

struct Grp_display_3 : public RLOp_Void_9< 
  IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T, 
  IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T,
  IntConstant_T>
{
  SPACE& m_space;
  Grp_display_3(SPACE& space) : m_space(space) {}

  void operator()(RLMachine& machine, int dc, int effectNum, 
                  int x1, int y1, int x2, int y2, int dx, int dy, int opacity)
  {
    GraphicsSystem& graphics = machine.system().graphics();
    m_space.translateToRec(x1, y1, x2, y2);
    loadDCToDC1(graphics, dc, x1, y1, x2, y2, dx, dy, opacity);

    // Set the long operation for the correct transition long operation
    shared_ptr<Surface> dc0 = graphics.getDC(0);
    shared_ptr<Surface> dc1 = graphics.getDC(1);
    LongOperation* lop = EffectFactory::buildFromSEL(machine, dc1, dc0, effectNum);
//    LongOperation* lop = m_space.buildEffectFrom(machine, dc1, dc0, effectNum);
    decorateEffectWithBlit(lop, dc1, dc0);
    machine.pushLongOperation(lop);
  }
};

// -----------------------------------------------------------------------

/** 
 * @todo Finish documentation
 */
struct Grp_display_2 : public RLOp_Void_8< 
  IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T, 
  IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T>
{
  Grp_display_3 delegate;
  Grp_display_2(SPACE& space) : delegate(space) {}

  void operator()(RLMachine& machine, int dc, int effectNum, 
                  int x1, int y1, int x2, int y2, int dx, int dy)
  {
    int opacity = getSELEffect(machine, effectNum).at(14);
    delegate(machine, dc, effectNum, x1, y1, x2, y2, dx, dy, 
             opacity);
  }
};


// -----------------------------------------------------------------------
// {grp,rec}Open
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
struct Grp_open_1 : public RLOp_Void_3< StrConstant_T, IntConstant_T, 
                                        IntConstant_T > {
  SPACE& m_space;
  bool m_useAlpha;
  Grp_open_1(bool in, SPACE& space) : m_space(space), m_useAlpha(in) {}

  void operator()(RLMachine& machine, string filename, int effectNum, 
                  int opacity)
  {
    vector<int> selEffect = getSELEffect(machine, effectNum);

    GraphicsSystem& graphics = machine.system().graphics();
    if(filename == "???") filename = graphics.defaultGrpName();
    filename = findFile(machine, filename);

    loadImageToDC1(graphics, filename,
                   selEffect[0], selEffect[1], selEffect[2], selEffect[3],
                   selEffect[4], selEffect[5], opacity, m_useAlpha);

    // Set the long operation for the correct transition long operation
    shared_ptr<Surface> dc0 = graphics.getDC(0);
    shared_ptr<Surface> dc1 = graphics.getDC(1);
    LongOperation* lop = EffectFactory::buildFromSEL(machine, dc1, dc0, effectNum);
    decorateEffectWithBlit(lop, dc1, dc0);
    machine.pushLongOperation(lop);
    machine.system().text().hideAllTextWindows();
  }
};

// -----------------------------------------------------------------------

/** 
 * Implements op<1:Grp:00076, 0>, fun grpOpen(strC 'filename', '\#SEL').
 * 
 * Load and display a bitmap. @em filename is loaded into DC1, and
 * then is passed off to whatever transition effect, which will
 * perform some intermediary steps and then render DC1 to DC0.
 */
struct Grp_open_0 : public RLOp_Void_2< StrConstant_T, IntConstant_T > {
  SPACE& m_space;
  Grp_open_1 m_delegate;
  Grp_open_0(bool in, SPACE& space) 
    : m_space(space), m_delegate(in, space) {}

  void operator()(RLMachine& machine, string filename, int effectNum) {
    vector<int> selEffect = getSELEffect(machine, effectNum);
    m_delegate(machine, filename, effectNum, selEffect[14]);
  }
};

// -----------------------------------------------------------------------

struct Grp_open_3 : public RLOp_Void_9< 
  StrConstant_T, IntConstant_T, IntConstant_T, IntConstant_T, 
  IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T,
  IntConstant_T>
{
  SPACE& m_space;
  bool m_useAlpha;
  Grp_open_3(bool in, SPACE& space) : m_space(space), m_useAlpha(in) {}

  void operator()(RLMachine& machine, string filename, int effectNum, 
                  int x1, int y1, int x2, int y2, int dx, int dy, int opacity)
  {
    GraphicsSystem& graphics = machine.system().graphics();
    if(filename == "???") filename = graphics.defaultGrpName();
    filename = findFile(machine, filename);

    m_space.translateToRec(x1, y1, x2, y2);
    loadImageToDC1(graphics, filename, x1, y1, x2, y2, dx, dy, opacity, m_useAlpha);

    // Set the long operation for the correct transition long operation
    shared_ptr<Surface> dc0 = graphics.getDC(0);
    shared_ptr<Surface> dc1 = graphics.getDC(1);
    LongOperation* lop = EffectFactory::buildFromSEL(machine, dc1, dc0, effectNum);
    decorateEffectWithBlit(lop, dc1, dc0);
    machine.pushLongOperation(lop);
    machine.system().text().hideAllTextWindows();
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
struct Grp_open_2 : public RLOp_Void_8< 
  StrConstant_T, IntConstant_T, IntConstant_T, IntConstant_T, 
  IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T>
{
  Grp_open_3 m_delegate;
  Grp_open_2(bool in, SPACE& space) : m_delegate(in, space) {}

  void operator()(RLMachine& machine, string filename, int effectNum, 
                  int x1, int y1, int x2, int y2, int dx, int dy)
  {
    int opacity = getSELEffect(machine, effectNum).at(14);
    m_delegate(machine, filename, effectNum, x1, y1, x2, y2, 
               dx, dy, opacity);
  }
};

// -----------------------------------------------------------------------

//(strC 'filename', 'x1', 'y1', 'x2', 'y2', 
// 'dx', 'dy', 'steps', 'effect', 'direction',
// 'interpolation', 'density', 'speed', '?', '?',
// 'alpha', '?')
struct Grp_open_4 : public RLOp_Void_17<
  StrConstant_T, IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T,
  IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T,
  IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T,
  IntConstant_T, IntConstant_T>
{
  SPACE& m_space;
  bool m_useAlpha;
  Grp_open_4(bool in, SPACE& space) : m_space(space), m_useAlpha(in) {}

  void operator()(RLMachine& machine, string fileName,
                  int x1, int y1, int x2, int y2, int dx, int dy,
                  int time, int style, int direction, int interpolation,
                  int xsize, int ysize, int a, int b, int opacity, int c)
  {
    GraphicsSystem& graphics = machine.system().graphics();
    m_space.translateToRec(x1, y1, x2, y2);

    // Set the long operation for the correct transition long operation
    shared_ptr<Surface> dc0 = 
      graphics.renderToSurfaceWithBg(machine, graphics.getDC(0));

    handleOpenBgFileName(machine, fileName, x1, y1, x2, y2, dx, dy, 
                         opacity, m_useAlpha);

    // Promote the objects 
    graphics.promoteObjects();

    // Set the long operation for the correct transition long operation
    shared_ptr<Surface> dc1 = graphics.getDC(1);
    shared_ptr<Surface> tmp = graphics.renderToSurfaceWithBg(machine, dc1);

    LongOperation* lop = 
      EffectFactory::build(machine, tmp, dc0, time, style, direction, 
                           interpolation, xsize, ysize, a, b, c);
    decorateEffectWithBlit(lop, dc1, graphics.getDC(0));
    machine.pushLongOperation(lop);
    machine.system().text().hideAllTextWindows();
  }
};

// -----------------------------------------------------------------------

struct Grp_openBg_1 : public RLOp_Void_3< StrConstant_T, IntConstant_T,
                                          IntConstant_T >
{
  void operator()(RLMachine& machine, string fileName, int effectNum, int opacity)
  {
    GraphicsSystem& graphics = machine.system().graphics();
    vector<int> selEffect = getSELEffect(machine, effectNum);

    graphics.addGraphicsStackFrame(GRP_OPENBG)
      .setFilename(fileName)
      .setSourceCoordinates(selEffect[0], selEffect[1], 
                            selEffect[2], selEffect[3])
      .setTargetCoordinates(selEffect[4], selEffect[5])
      .setOpacity(opacity);

    // Set the long operation for the correct transition long operation
    shared_ptr<Surface> dc0 = 
      graphics.renderToSurfaceWithBg(machine, graphics.getDC(0));

    handleOpenBgFileName(machine, fileName, selEffect[0], selEffect[1], 
                         selEffect[2], selEffect[3], selEffect[4], 
                         selEffect[5], opacity, false);

    // Promote the objects 
    graphics.clearAndPromoteObjects();

    // Render the screen to a temporary 
    shared_ptr<Surface> dc1 = graphics.getDC(1);    
    shared_ptr<Surface> tmp = graphics.renderToSurfaceWithBg(machine, dc1);

    LongOperation* effect = EffectFactory::buildFromSEL(machine, tmp, dc0, effectNum);
    decorateEffectWithBlit(effect, graphics.getDC(1), graphics.getDC(0));
    machine.pushLongOperation(effect);
    machine.system().text().hideAllTextWindows();
  }
};

// -----------------------------------------------------------------------

struct Grp_openBg_0 : public RLOp_Void_2< StrConstant_T, IntConstant_T > {
  Grp_openBg_1 m_delegate;
  
  void operator()(RLMachine& machine, string filename, int effectNum) {
    vector<int> selEffect = getSELEffect(machine, effectNum);
    m_delegate(machine, filename, effectNum, selEffect[14]);
  }
};

// -----------------------------------------------------------------------

struct Grp_openBg_4 : public RLOp_Void_17<
  StrConstant_T, IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T,
  IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T,
  IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T,
  IntConstant_T, IntConstant_T>
{
  SPACE& m_space;
  bool m_useAlpha;
  Grp_openBg_4(bool in, SPACE& space) : m_space(space), m_useAlpha(in) {}

  void operator()(RLMachine& machine, string fileName,
                  int x1, int y1, int x2, int y2, int dx, int dy,
                  int time, int style, int direction, int interpolation,
                  int xsize, int ysize, int a, int b, int opacity, int c)
  {
    GraphicsSystem& graphics = machine.system().graphics();
    m_space.translateToRec(x1, y1, x2, y2);

    graphics.addGraphicsStackFrame(GRP_OPENBG)
      .setFilename(fileName)
      .setSourceCoordinates(x1, y1, x2, y2)
      .setTargetCoordinates(dx, dy)
      .setOpacity(opacity);

    // Set the long operation for the correct transition long operation
    shared_ptr<Surface> dc0 = 
      graphics.renderToSurfaceWithBg(machine, 
                                     graphics.getDC(0));

    handleOpenBgFileName(machine, fileName, x1, y1, x2, y2, dx, dy, 
                         opacity, m_useAlpha);

    // Promote the objects 
    graphics.clearAndPromoteObjects();

    // Render the screen to a temporary 
    shared_ptr<Surface> dc1 = graphics.getDC(1);    
    shared_ptr<Surface> tmp = graphics.renderToSurfaceWithBg(machine, dc1);

    LongOperation* effect = EffectFactory::build(machine, tmp, dc0, time, 
                                          style, direction, interpolation,
                                          xsize, ysize, a, b, c);
    decorateEffectWithBlit(effect, graphics.getDC(1), graphics.getDC(0));
    machine.pushLongOperation(effect);
    machine.system().text().hideAllTextWindows();
  }
};

// -----------------------------------------------------------------------
// {grp,rec}Copy
// -----------------------------------------------------------------------

struct Grp_copy_3 : public RLOp_Void_9<
  IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T,
  IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T,
  DefaultIntValue_T<255> > 
{
  SPACE& m_space;
  bool m_useAlpha;
  Grp_copy_3(bool in, SPACE& space) : m_space(space), m_useAlpha(in) {}

  void operator()(RLMachine& machine, int x1, int y1, int x2, int y2,
                  int src, int dx, int dy, int dst, int opacity) {
    // Copying to self is a noop
    if(src == dst)
      return;

    GraphicsSystem& graphics = machine.system().graphics();
    shared_ptr<Surface> sourceSurface = graphics.getDC(src);

    // Reallocate the destination so that it's the same size as the first.
//    graphics.allocateDC(dst, sourceSurface.width(), sourceSurface.height());
    // @todo allocateDC?

    m_space.translateToRec(x1, y1, x2, y2);

    sourceSurface->blitToSurface(
      *graphics.getDC(dst),
      x1, y1, x2, y2, dx, dy, x2, y2, opacity, m_useAlpha);
  }
};

// -----------------------------------------------------------------------

struct Grp_copy_1 : public RLOp_Void_3<IntConstant_T, IntConstant_T, 
                                       DefaultIntValue_T<255> > 
{
  bool m_useAlpha;
  Grp_copy_1(bool in) : m_useAlpha(in) {}

  void operator()(RLMachine& machine, int src, int dst, int opacity) {
    // Copying to self is a noop
    if(src == dst)
      return;

    GraphicsSystem& graphics = machine.system().graphics();
    shared_ptr<Surface> sourceSurface = graphics.getDC(src);

    // Reallocate the destination so that it's the same size as the first.
//     if(dst != 0)
//       graphics.allocateDC(dst, sourceSurface->width(), sourceSurface->height());
    if(dst != 0 && dst != 1)
      graphics.allocateDC(dst,
                          max(sourceSurface->width(), graphics.screenWidth()),
                          max(sourceSurface->height(), graphics.screenHeight()));

    sourceSurface->blitToSurface(
      *graphics.getDC(dst),
      0, 0, sourceSurface->width(), sourceSurface->height(),
      0, 0, sourceSurface->width(), sourceSurface->height(),
      opacity, m_useAlpha);
  }
};

// -----------------------------------------------------------------------
// {grp,rec}Fill
// -----------------------------------------------------------------------

struct Grp_fill_1 : public RLOp_Void_5<
  IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T, 
  DefaultIntValue_T<255> > {
  void operator()(RLMachine& machine, int dc, int r, int g, int b, int alpha) {
    machine.system().graphics().getDC(dc)->fill(r, g, b, alpha);
  }
};

// -----------------------------------------------------------------------

struct Grp_fill_3 : public RLOp_Void_9<
  IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T, 
  IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T, 
  DefaultIntValue_T<255> > 
{
  SPACE& m_space;
  Grp_fill_3(SPACE& space) : m_space(space) {}

  void operator()(RLMachine& machine, int x1, int y1, int x2, int y2,
                  int dc, int r, int g, int b, int alpha) {
    m_space.translateToRec(x1, y1, x2, y2);
    machine.system().graphics().getDC(dc)->fill(r, g, b, alpha, x1, y1, x2, y2);
  }
};

// -----------------------------------------------------------------------
// {grp,rec}Fade
// -----------------------------------------------------------------------

struct Grp_fade_7 : public RLOp_Void_8<
  IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T,
  IntConstant_T, IntConstant_T, IntConstant_T, DefaultIntValue_T<0> >
{
  SPACE& m_space;
  Grp_fade_7(SPACE& space) : m_space(space) {}

  void operator()(RLMachine& machine, int x1, int y1, int x2, int y2, 
                  int r, int g, int b, int time) {
    m_space.translateToRec(x1, y1, x2, y2);
    GraphicsSystem& graphics = machine.system().graphics();
    if (time == 0) {
      graphics.getDC(0)->fill(r, g, b, 255, x1, y1, x2, y2);
    }
    else {
      // FIXME: this needs checking for sanity of implementation, lack
      // of memory leaks, correct functioning in the presence of
      // objects, etc.
      m_space.translateToRec(x1, y1, x2, y2);
      shared_ptr<Surface> dc0 = graphics.getDC(0);
      shared_ptr<Surface> tmp(dc0->clone());
      tmp->fill(r, g, b, 255, x1, y1, x2, y2);
      LongOperation* lop = 
        EffectFactory::build(machine, tmp, dc0, time, 0, 0, 
                             0, 0, 0, 0, 0, 0);
      decorateEffectWithBlit(lop, tmp, dc0);
      machine.pushLongOperation(lop);
    }
  }
};

struct Grp_fade_5 : public RLOp_Void_6<
  IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T, 
  IntConstant_T, DefaultIntValue_T<0> >
{
  SPACE& m_space;
  Grp_fade_7 m_delegate;
  Grp_fade_5(SPACE& space) : m_space(space), m_delegate(space) {}

  void operator()(RLMachine& machine, int x1, int y1, int x2, int y2, 
                  int color_num, int time) {
    Gameexe& gexe = machine.system().gameexe();
    const vector<int>& rgb = gexe("COLOR_TABLE", color_num).to_intVector();
    m_delegate(machine, x1, y1, x2, y2, rgb[0], rgb[1], rgb[2], time);
  }
};

struct Grp_fade_3 : public RLOp_Void_4<
  IntConstant_T, IntConstant_T, IntConstant_T, DefaultIntValue_T<0> >
{
  Grp_fade_7 m_delegate;
  Grp_fade_3() : m_delegate(REC_SPACE::get()) {}

  void operator()(RLMachine& machine, int r, int g, int b, int time) {
    GraphicsSystem& graphics = machine.system().graphics();
    m_delegate(machine, 0, 0, graphics.screenWidth(), graphics.screenHeight(),
               r, g, b, time);
  }
};

struct Grp_fade_1 : public RLOp_Void_2<
  IntConstant_T, DefaultIntValue_T<0> >
{
  Grp_fade_7 m_delegate;
  Grp_fade_1() : m_delegate(REC_SPACE::get()) {}

  void operator()(RLMachine& machine, int color_num, int time) {
    GraphicsSystem& graphics = machine.system().graphics();
    Gameexe& gexe = machine.system().gameexe();
    const vector<int>& rgb = gexe("COLOR_TABLE", color_num).to_intVector();
    m_delegate(machine, 0, 0, graphics.screenWidth(), graphics.screenHeight(),
               rgb[0], rgb[1], rgb[2], time);
  }
};

// -----------------------------------------------------------------------

struct Grp_zoom : public RLOp_Void_14<
  IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T,
  IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T,
  IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T>
{
  SPACE& m_space;
  Grp_zoom(SPACE& space) : m_space(space) {}

  void operator()(RLMachine& machine, int fx, int fy, int fwidth, int fheight,
                  int tx, int ty, int twidth, int theight, int srcDC, int dx,
                  int dy, int dwidth, int dheight, int time)
  {
    GraphicsSystem& gs = machine.system().graphics();
    m_space.translateToRec(fx, fy, fwidth, fheight);
    m_space.translateToRec(tx, ty, twidth, theight);
    m_space.translateToRec(dx, dy, dwidth, dheight);

    LongOperation* zoomOp =
      new ZoomLongOperation(
        machine, gs.getDC(0), gs.getDC(srcDC),
        fx, fy, fwidth, fheight, tx, ty, twidth, theight, dx, dy, 
        dwidth, dheight, time);
    BlitAfterEffectFinishes* blitOp = 
      new BlitAfterEffectFinishes(
        zoomOp, 
        gs.getDC(srcDC), gs.getDC(0), /* order? */
        tx, ty, twidth, theight,
        dx, dy, dwidth, dheight);
    machine.pushLongOperation(blitOp);
  }
};

// -----------------------------------------------------------------------
// {grp,rec}multi
// -----------------------------------------------------------------------

/**
 * defines the fairly complex parameter definition for the list of
 * functions to call in a {grp,rec}Multi command.
 */
typedef Argc_T< 
  Special_T<
    StrConstant_T, // 0:copy
    Complex2_T<StrConstant_T, IntConstant_T>, // 1:copy
    Complex3_T<StrConstant_T, IntConstant_T, IntConstant_T>, // 2:copy
    // 3:area
    Complex7_T<StrConstant_T, IntConstant_T, IntConstant_T,
               IntConstant_T, IntConstant_T, IntConstant_T, 
               IntConstant_T>,
    // 4:area
    Complex8_T<StrConstant_T, IntConstant_T, IntConstant_T,
               IntConstant_T, IntConstant_T, IntConstant_T, 
               IntConstant_T, IntConstant_T>
    > >  MultiCommand;

// -----------------------------------------------------------------------

/**
 * Defines the weird multi commands. I will be the first to admit that
 * the following is fairly difficult to read; it comes from the
 * quagmire of composing Special_T and ComplexX_T templates.
 *
 * In the end, this operation struct simply dispatches the
 * Special/Complex commands to functions and other operation structs
 * that are clearer in purpose.
 * 
 * @todo Finish this operation; it's in an incomplete, but compiling
 *       state.
 * @see MultiCommand
 */
//template<typename SPACE>
/*
struct Grp_multi_1 : public RLOp_Void_4<StrConstant_T, IntConstant_T, 
                                        IntConstant_T, MultiCommand>
{
  void operator()(RLMachine& machine, string filename, int effect, int alpha,
                  MultiCommand::type commands)
  {
//    Grp_load_0(false)(machine, filename, effect);

    for(MultiCommand::type::iterator it = commands.begin(); it != commands.end();
        it++) 
    {
      switch(it->type)
      {
      case 0:
//        Grp_load_0(true)(machine, it->first.get<0>(), it->first.get<1>(),
//                         it->first.get<2>(), it->first.get<3>());
        break;
      case 1:
//        Grp_load_1(true)(machine, it->second.get<0>(), it->second.get<1>(),
//                         it->second.get<2>(), it->second.get<3>());
        break;
      case 2:

        break;
      case 3:
        Grp_load_3<SPACE>(true)(machine, it->fourth.get<0>(), 
                                it->fourth.get<1>(),
                                it->fourth.get<2>(),
                                it->fourth.get<3>(),
                                it->fourth.get<4>(),
                                it->fourth.get<5>(),
                                it->fourth.get<6>(),
                                255);
        break;
      case 4:
        Grp_load_3<SPACE>(true)(machine, it->fifth.get<0>(), 
                                it->fifth.get<1>(),
                                it->fifth.get<2>(),
                                it->fifth.get<3>(),
                                it->fifth.get<4>(),
                                it->fifth.get<5>(),
                                it->fifth.get<6>(),
                                it->fifth.get<7>());
        break;
      }
    }

    // Does this work?
    Grp_display_0<SPACE>()(1, effect);
  }
};
*/

// -----------------------------------------------------------------------
/*
template<typename SPACE>
struct Grp_multi_0 : public RLOp_Void_3<StrConstant_T, IntConstant_T, MultiCommand>
{
  void operator()(RLMachine& machine, string dc, int effect, 
                  MultiCommand::type commands)
  {
    Grp_multi_1<SPACE>()(machine, dc, effect, 255, commands);
  }     
};
*/     
                                                                 
// -----------------------------------------------------------------------

/**
 *
 * At minimum, we need to get these functions working for Kanon:
 *
 * $  grep grp *.ke -h | cut -f 1 -d " " | sort | uniq  
 * grpBuffer
 * grpCopy
 * grpFill
 * grpMaskCopy
 * grpMaskOpen
 * grpMulti
 * grpOpenBg
 * $  grep rec *.ke -h | cut -f 1 -d " " | sort | uniq
 * recCopy
 * recFill
 * recOpen
 * recOpenBg
 */
GrpModule::GrpModule()
  : RLModule("Grp", 1, 33)
{
  SPACE& GRP = GRP_SPACE::get();
  SPACE& REC = REC_SPACE::get();

  addOpcode(15, 0, "allocDC", new Grp_allocDC);
  addOpcode(16, 0, "freeDC", new Grp_freeDC);

  addUnsupportedOpcode(20, 0, "grpLoadMask");
  // addOpcode(30, 0, new Grp_grpTextout);

  addOpcode(31, 0, "wipe", new Grp_wipe);
  // addOpcode(32, 0, new Grp_shake);

  addOpcode(50, 0, "grpLoad", new Grp_load_1(false));
  addOpcode(50, 1, "grpLoad", new Grp_load_1(false));
  addOpcode(50, 2, "grpLoad", new Grp_load_3(false, GRP));
  addOpcode(50, 3, "grpLoad", new Grp_load_3(false, GRP));
  addOpcode(51, 0, "grpMaskLoad", new Grp_load_1(true));
  addOpcode(51, 1, "grpMaskLoad", new Grp_load_1(true));
  addOpcode(51, 2, "grpMaskLoad", new Grp_load_3(true, GRP));
  addOpcode(51, 3, "grpMaskLoad", new Grp_load_3(true, GRP));

  // These are grpBuffer, which is very similar to grpLoad and Haeleth
  // doesn't know how they differ. For now, we just assume they're
  // equivalent.
  addOpcode(70, 0, "grpBuffer", new Grp_load_1(false));
  addOpcode(70, 1, "grpBuffer", new Grp_load_1(false));
  addOpcode(70, 2, "grpBuffer", new Grp_load_3(false, GRP));
  addOpcode(70, 3, "grpBuffer", new Grp_load_3(false, GRP));
  addOpcode(71, 0, "grpMaskBuffer", new Grp_load_1(true));
  addOpcode(71, 1, "grpMaskBuffer", new Grp_load_1(true));
  addOpcode(71, 2, "grpMaskBuffer", new Grp_load_3(true, GRP));
  addOpcode(71, 3, "grpMaskBuffer", new Grp_load_3(true, GRP));

  addOpcode(72, 0, "grpDisplay", new Grp_display_0);
  addOpcode(72, 1, "grpDisplay", new Grp_display_1);
  addOpcode(72, 2, "grpDisplay", new Grp_display_2(GRP));
  addOpcode(72, 3, "grpDisplay", new Grp_display_3(GRP));

  // These are supposed to be grpOpenBg, but until I have the object
  // layer working, this simply does the same thing.
  addOpcode(73, 0, "grpOpenBg", new Grp_openBg_0);
  addOpcode(73, 1, "grpOpenBg", new Grp_openBg_1);
  addOpcode(73, 2, "grpOpenBg", new Grp_open_2(false, GRP));
  addOpcode(73, 3, "grpOpenBg", new Grp_open_3(false, GRP));
  addOpcode(73, 4, "grpOpenBg", new Grp_openBg_4(false, GRP));

  addOpcode(74, 0, "grpMaskOpen", new Grp_open_0(true, GRP));
  addOpcode(74, 1, "grpMaskOpen", new Grp_open_1(true, GRP));
  addOpcode(74, 2, "grpMaskOpen", new Grp_open_2(true, GRP));
  addOpcode(74, 3, "grpMaskOpen", new Grp_open_3(true, GRP));
  addOpcode(74, 4, "grpMaskOpen", new Grp_open_4(true, GRP));

  addUnsupportedOpcode(75, 0, "grpMulti");
  addUnsupportedOpcode(75, 1, "grpMulti");
  addUnsupportedOpcode(75, 2, "grpMulti");
  addUnsupportedOpcode(75, 3, "grpMulti");
  addUnsupportedOpcode(75, 4, "grpMulti");

  addOpcode(76, 0, "grpOpen", new Grp_open_0(false, GRP));
  addOpcode(76, 1, "grpOpen", new Grp_open_1(false, GRP));
  addOpcode(76, 2, "grpOpen", new Grp_open_2(false, GRP));
  addOpcode(76, 3, "grpOpen", new Grp_open_3(false, GRP));
  addOpcode(76, 4, "grpOpen", new Grp_open_4(false, GRP));

  addUnsupportedOpcode(77, 0, "grpMulti");
  addUnsupportedOpcode(77, 1, "grpMulti");
  addUnsupportedOpcode(77, 2, "grpMulti");
  addUnsupportedOpcode(77, 3, "grpMulti");
  addUnsupportedOpcode(77, 4, "grpMulti");

  addOpcode(100, 0, "grpCopy", new Grp_copy_1(false));
  addOpcode(100, 1, "grpCopy", new Grp_copy_1(false));
  addOpcode(100, 2, "grpCopy", new Grp_copy_3(false, GRP));
  addOpcode(100, 3, "grpCopy", new Grp_copy_3(false, GRP));
  addOpcode(101, 0, "grpMaskCopy", new Grp_copy_1(true));
  addOpcode(101, 1, "grpMaskCopy", new Grp_copy_1(true));
  addOpcode(101, 2, "grpMaskCopy", new Grp_copy_3(true, GRP));
  addOpcode(101, 3, "grpMaskCopy", new Grp_copy_3(true, GRP));

  addUnsupportedOpcode(120, 5, "grpCopyWithMask");
  addUnsupportedOpcode(140, 5, "grpCopyInvMask");

  addOpcode(201, 0, "grpFill", new Grp_fill_1);
  addOpcode(201, 1, "grpFill", new Grp_fill_1);
  addOpcode(201, 2, "grpFill", new Grp_fill_3(GRP));
  addOpcode(201, 3, "grpFill", new Grp_fill_3(GRP));

  addUnsupportedOpcode(400, 0, "grpSwap");
  addUnsupportedOpcode(400, 1, "grpSwap");

  addOpcode(402, 0, "grpZoom", new Grp_zoom(GRP));

  addOpcode(403, 0, "grpFade", new Grp_fade_1);
  addOpcode(403, 1, "grpFade", new Grp_fade_1);
  addOpcode(403, 2, "grpFade", new Grp_fade_3);
  addOpcode(403, 3, "grpFade", new Grp_fade_3);
  addOpcode(403, 4, "grpFade", new Grp_fade_5(GRP));
  addOpcode(403, 5, "grpFade", new Grp_fade_5(GRP));
  addOpcode(403, 6, "grpFade", new Grp_fade_7(GRP));
  addOpcode(403, 7, "grpFade", new Grp_fade_7(GRP));


  // -----------------------------------------------------------------------
  
  addOpcode(1050, 0, "recLoad", new Grp_load_1(false));
  addOpcode(1050, 1, "recLoad", new Grp_load_1(false));
  addOpcode(1050, 2, "recLoad", new Grp_load_3(false, REC));
  addOpcode(1050, 3, "recLoad", new Grp_load_3(false, REC));

  addOpcode(1052, 0, "recDisplay", new Grp_display_0);
  addOpcode(1052, 1, "recDisplay", new Grp_display_1);
  addOpcode(1052, 2, "recDisplay", new Grp_display_2(REC));
  addOpcode(1052, 3, "recDisplay", new Grp_display_3(REC));

  // These are supposed to be recOpenBg, but until I have the object
  // layer working, this simply does the same thing.
  addOpcode(1053, 0, "recOpenBg", new Grp_openBg_0);
  addOpcode(1053, 1, "recOpenBg", new Grp_openBg_1);
  addOpcode(1053, 2, "recOpenBg", new Grp_open_2(false, REC));
  addOpcode(1053, 3, "recOpenBg", new Grp_open_3(false, REC));
  addOpcode(1053, 4, "recOpenBg", new Grp_openBg_4(false, REC));

  addOpcode(1054, 0, "recMaskOpen", new Grp_open_0(true, REC));
  addOpcode(1054, 1, "recMaskOpen", new Grp_open_1(true, REC));
  addOpcode(1054, 2, "recMaskOpen", new Grp_open_2(true, REC));
  addOpcode(1054, 3, "recMaskOpen", new Grp_open_3(true, REC));
  addOpcode(1054, 4, "recMaskOpen", new Grp_open_4(true, REC));

  addOpcode(1056, 0, "recOpen", new Grp_open_0(false, REC));
  addOpcode(1056, 1, "recOpen", new Grp_open_1(false, REC));
  addOpcode(1056, 2, "recOpen", new Grp_open_2(false, REC));
  addOpcode(1056, 3, "recOpen", new Grp_open_3(false, REC));
  addOpcode(1056, 4, "recOpen", new Grp_open_4(false, REC));

  addOpcode(1100, 0, "recCopy", new Grp_copy_1(false));
  addOpcode(1100, 1, "recCopy", new Grp_copy_1(false));
  addOpcode(1100, 2, "recCopy", new Grp_copy_3(false, REC));
  addOpcode(1100, 3, "recCopy", new Grp_copy_3(false, REC));
  addOpcode(1101, 0, "recMaskCopy", new Grp_copy_1(true));
  addOpcode(1101, 1, "recMaskCopy", new Grp_copy_1(true));
  addOpcode(1101, 2, "recMaskCopy", new Grp_copy_3(true, REC));
  addOpcode(1101, 3, "recMaskCopy", new Grp_copy_3(true, REC));

  addOpcode(1201, 0, "recFill", new Grp_fill_1);
  addOpcode(1201, 1, "recFill", new Grp_fill_1);
  addOpcode(1201, 2, "recFill", new Grp_fill_3(REC));
  addOpcode(1201, 3, "recFill", new Grp_fill_3(REC));

  addOpcode(1402, 0, "recZoom", new Grp_zoom(REC));

  addOpcode(1403, 0, "recFade", new Grp_fade_1);
  addOpcode(1403, 1, "recFade", new Grp_fade_1);
  addOpcode(1403, 2, "recFade", new Grp_fade_3);
  addOpcode(1403, 3, "recFade", new Grp_fade_3);
  addOpcode(1403, 4, "recFade", new Grp_fade_5(REC));
  addOpcode(1403, 5, "recFade", new Grp_fade_5(REC));
  addOpcode(1403, 6, "recFade", new Grp_fade_7(REC));
  addOpcode(1403, 7, "recFade", new Grp_fade_7(REC));
}

// @}


// -----------------------------------------------------------------------

void replayOpenBg(RLMachine& machine, const GraphicsStackFrame& f)
{ 
  handleOpenBgFileName(
    machine, f.filename(), 
    f.sourceX1(), f.sourceY1(), f.sourceX2(), f.sourceY2(),
    f.targetX1(), f.targetY1(), f.opacity(), false);

  blitDC1toDC0(machine);
}

// -----------------------------------------------------------------------

void replayGraphicsStackVector(
  RLMachine& machine, 
  const std::vector<GraphicsStackFrame>& gstack)
{
  for(vector<GraphicsStackFrame>::const_iterator it = gstack.begin(); 
      it != gstack.end(); ++it)
  {
    if(it->name() == GRP_LOAD)
    {
      if(it->hasTargetCoordinates())
      {
        cerr << "Ignoring because we are dumb!" << endl;
      }
      else
        Grp_load_1(true)(machine, it->filename(), it->targetDC(),
                         it->opacity());
    }
    else if(it->name() == GRP_OPENBG)
    {
      replayOpenBg(machine, *it);
    }
  }
}
