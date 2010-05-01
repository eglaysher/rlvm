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

#include "Modules/Module_Grp.hpp"

#include <boost/scoped_ptr.hpp>
#include <iostream>
#include <string>
#include <vector>

#include "Effects/Effect.hpp"
#include "Effects/EffectFactory.hpp"
#include "LongOperations/ZoomLongOperation.hpp"
#include "MachineBase/GeneralOperations.hpp"
#include "MachineBase/RLMachine.hpp"
#include "MachineBase/RLOperation.hpp"
#include "MachineBase/RLOperation/Argc_T.hpp"
#include "MachineBase/RLOperation/Complex_T.hpp"
#include "MachineBase/RLOperation/DefaultValue.hpp"
#include "MachineBase/RLOperation/RGBColour_T.hpp"
#include "MachineBase/RLOperation/Rect_T.hpp"
#include "MachineBase/RLOperation/Special_T.hpp"
#include "Systems/Base/Colour.hpp"
#include "Systems/Base/GraphicsStackFrame.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/Surface.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/TextSystem.hpp"
#include "Utilities/Graphics.hpp"
#include "libReallive/gameexe.h"

using namespace std;
using namespace boost;

namespace fs = boost::filesystem;

namespace {

// Constant names used in the graphics stack.
const std::string GRP_ALLOC = "allocDC";
const std::string GRP_WIPE = "wipe";
const std::string GRP_COPY = "copy";
const std::string GRP_DISPLAY = "display";
const std::string GRP_LOAD = "grpLoad";
const std::string GRP_OPEN = "grpOpen";
const std::string GRP_OPENBG = "grpOpenBg";


void blitDC1toDC0(RLMachine& machine) {
  GraphicsSystem& graphics = machine.system().graphics();

  boost::shared_ptr<Surface> src = graphics.getDC(1);
  boost::shared_ptr<Surface> dst = graphics.getDC(0);

  // Blit DC1 onto DC0, with full opacity, and end the operation
  src->blitToSurface(*dst, src->rect(), dst->rect(), 255);

  // Mark that the background should be DC0 instead of the Haikei.
  graphics.setGraphicsBackground(BACKGROUND_DC0);

  // Promote the objects
  graphics.clearAndPromoteObjects();
}

// Performs half the grunt work of a recOpen command; Copies DC0 to DC1, loads
// a graphics file, and then composites that file to DC1.
//
// Kanon uses the recOpen('?', ...) form for rendering Last Regrets. This isn't
// documented in the rldev manual, and we must check for that case.
void loadImageToDC1(RLMachine& machine,
                    std::string name,
                    const Rect& srcRect,
                    const Point& dest,
                    int opacity, bool useAlpha) {
  GraphicsSystem& graphics = machine.system().graphics();

  if (name != "?") {
    if (name == "???")
      name = graphics.defaultGrpName();

    shared_ptr<Surface> dc0 = graphics.getDC(0);
    shared_ptr<Surface> dc1 = graphics.getDC(1);

    // Inclusive ranges are a monstrosity to computer people
    Size size = srcRect.size() + Size(1, 1);

    dc0->blitToSurface(*dc1, dc0->rect(), dc0->rect(), 255);

    // Load the section of the image file on top of dc1
    shared_ptr<Surface> surface(graphics.loadSurfaceFromFile(machine, name));
    surface->blitToSurface(*graphics.getDC(1),
                           Rect(srcRect.origin(), size),
                           Rect(dest, size),
                           opacity, useAlpha);
  }
}

void loadDCToDC1(GraphicsSystem& graphics,
                 int srcDc,
                 const Rect& srcRect,
                 const Point& dest,
                 int opacity) {
  shared_ptr<Surface> dc1 = graphics.getDC(1);
  shared_ptr<Surface> src = graphics.getDC(srcDc);

  // Inclusive ranges are a monstrosity to computer people
  Size size = srcRect.size() + Size(1, 1);

  src->blitToSurface(*dc1,
                     Rect(srcRect.origin(), size),
                     Rect(dest, size),
                     opacity, false);
}

void performEffect(RLMachine& machine,
                   const shared_ptr<Surface>& src,
                   const shared_ptr<Surface>& dst,
                   int selnum) {
  LongOperation* lop =
      EffectFactory::buildFromSEL(machine, src, dst, selnum);
  machine.pushLongOperation(lop);
}

void performEffect(RLMachine& machine,
                   const shared_ptr<Surface>& src,
                   const shared_ptr<Surface>& dst,
                   int time, int style, int direction, int interpolation,
                   int xsize, int ysize, int a, int b, int c) {
  LongOperation* lop =
      EffectFactory::build(machine, src, dst, time, style, direction,
                           interpolation, xsize, ysize, a, b, c);
  machine.pushLongOperation(lop);
}

// Implements op<1:Grp:00015, 0>, fun allocDC('DC', 'width', 'height').
//
// Allocates a blank width × height bitmap in dc. Any DC apart from DC 0 may be
// allocated thus, although DC 1 is never given a size smaller than the screen
// resolution. Any previous contents of dc are erased.
struct allocDC : public RLOp_Void_3<IntConstant_T, IntConstant_T,
                                    IntConstant_T> {
  void operator()(RLMachine& machine, int dc, int width, int height) {
    machine.system().graphics().addGraphicsStackFrame(GRP_ALLOC)
      .setTargetDC(dc).setTargetCoordinates(Point(width, height));
    machine.system().graphics().allocateDC(dc, Size(width, height));
  }
};

// Implements op<1:Grp:00031, 0>, fun wipe('DC', 'r', 'g', 'b')
//
// Fills dc with the colour indicated by the given RGB triplet.
struct wipe : public RLOp_Void_4<IntConstant_T, IntConstant_T,
                                 IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, int dc, int r, int g, int b) {
    machine.system().graphics().addGraphicsStackFrame(GRP_WIPE)
      .setTargetDC(dc).setRGB(r, g, b);

    machine.system().graphics().getDC(dc)->fill(RGBAColour(r, g, b));
  }
};

// -----------------------------------------------------------------------
// {grp,rec}Load Commands
// -----------------------------------------------------------------------

// Implements the {grp,rec}(Mask)?Load family of functions.
//
// Loads filename into dc; note that filename may not be '???'.
//
// Since this function deals with the entire screen (and therefore doesn't need
// to worry about the difference between grp/rec coordinate space), we write
// one function for both versions.
struct load_1 : public RLOp_Void_3<StrConstant_T, IntConstant_T,
                                   DefaultIntValue_T<255> > {
  bool use_alpha_;
  explicit load_1(bool in) : use_alpha_(in) {}

  void operator()(RLMachine& machine, string filename, int dc, int opacity) {
    GraphicsSystem& graphics = machine.system().graphics();

    graphics.addGraphicsStackFrame(GRP_LOAD)
      .setFilename(filename).setTargetDC(dc).setOpacity(opacity)
      .setMask(use_alpha_);

    shared_ptr<Surface> surface(
        graphics.loadSurfaceFromFile(machine, filename));

    if (dc != 0 && dc != 1) {
      graphics.allocateDC(dc, surface->size());
    }

    surface->blitToSurface(*graphics.getDC(dc),
                           surface->rect(), surface->rect(),
                           opacity, use_alpha_);
  }
};

// Implements op<1:Grp:00050, 3>, fun grpLoad(strC 'filename', 'DC',
// 'x1', 'y1', 'x2', 'y2', 'dx', 'dy', 'alpha').
//
// Loads filename into dc; note that filename may not be '???'. Using this
// form, the given area of the bitmap is loaded at the given location.
template<typename SPACE>
struct load_3 : public RLOp_Void_5<
  StrConstant_T, IntConstant_T, Rect_T<SPACE>, Point_T,
  DefaultIntValue_T<255> > {
  bool use_alpha_;
  explicit load_3(bool in) : use_alpha_(in) {}

  void operator()(RLMachine& machine, string filename, int dc,
                  Rect srcRect, Point dest, int opacity) {
    GraphicsSystem& graphics = machine.system().graphics();
    shared_ptr<Surface> surface(
        graphics.loadSurfaceFromFile(machine, filename));

    graphics.addGraphicsStackFrame(GRP_LOAD)
        .setFilename(filename)
        .setTargetDC(dc)
        .setSourceCoordinates(srcRect)
        .setTargetCoordinates(dest)
        .setOpacity(opacity)
        .setMask(use_alpha_);

    Rect destRect = Rect(dest, srcRect.size());

    if (dc != 0 && dc != 1) {
      graphics.setMinimumSizeForDC(dc, surface->size());
    }

    surface->blitToSurface(*graphics.getDC(dc), srcRect, destRect,
                           opacity, use_alpha_);
  }
};

// -----------------------------------------------------------------------
// {grp,rec}Display
// -----------------------------------------------------------------------

struct display_1
  : public RLOp_Void_3<IntConstant_T, IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, int dc, int effectNum, int opacity) {
    Rect src;
    Point dest;
    getSELPointAndRect(machine, effectNum, src, dest);

    GraphicsSystem& graphics = machine.system().graphics();

    graphics.addGraphicsStackFrame(GRP_DISPLAY)
      .setSourceCoordinates(src)
      .setSourceDC(dc)
      .setTargetCoordinates(dest)
      .setOpacity(opacity);

    shared_ptr<Surface> before = graphics.renderToSurface();

    loadDCToDC1(graphics, dc, src, dest, opacity);
    blitDC1toDC0(machine);

    shared_ptr<Surface> after = graphics.renderToSurface();
    performEffect(machine, after, before, effectNum);
  }
};

struct display_0 : public RLOp_Void_2<IntConstant_T, IntConstant_T> {
  display_1 delegate_;

  void operator()(RLMachine& machine, int dc, int effectNum) {
    vector<int> selEffect = getSELEffect(machine, effectNum);
    delegate_(machine, dc, effectNum, selEffect.at(14));
  }
};

template<typename SPACE>
struct display_3
    : public RLOp_Void_5<IntConstant_T, IntConstant_T, Rect_T<SPACE>, Point_T,
                         IntConstant_T> {
  void operator()(RLMachine& machine, int dc, int effectNum,
                  Rect srcRect, Point dest, int opacity) {
    GraphicsSystem& graphics = machine.system().graphics();

    graphics.addGraphicsStackFrame(GRP_DISPLAY)
      .setSourceDC(dc)
      .setSourceCoordinates(srcRect)
      .setTargetCoordinates(dest)
      .setOpacity(opacity);

    shared_ptr<Surface> before = graphics.renderToSurface();

    loadDCToDC1(graphics, dc, srcRect, dest, opacity);
    blitDC1toDC0(machine);

    shared_ptr<Surface> after = graphics.renderToSurface();
    performEffect(machine, after, before, effectNum);
  }
};

template<typename SPACE>
struct display_2
    : public RLOp_Void_4<IntConstant_T, IntConstant_T, Rect_T<SPACE>, Point_T> {
  void operator()(RLMachine& machine, int dc, int effectNum,
                  Rect src_rect, Point dest) {
    int opacity = getSELEffect(machine, effectNum).at(14);
    display_3<SPACE>()(machine, dc, effectNum, src_rect, dest, opacity);
  }
};

template<typename SPACE>
struct display_4
    : public RLOp_Void_13<IntConstant_T, Rect_T<SPACE>, Point_T,
                          IntConstant_T, IntConstant_T, IntConstant_T,
                          IntConstant_T, IntConstant_T, IntConstant_T,
                          IntConstant_T, IntConstant_T, IntConstant_T,
                          IntConstant_T> {
  void operator()(RLMachine& machine, int dc, Rect srcRect, Point dest,
                  int time, int style, int direction, int interpolation,
                  int xsize, int ysize, int a, int b, int opacity, int c) {
    GraphicsSystem& graphics = machine.system().graphics();

    graphics.addGraphicsStackFrame(GRP_DISPLAY)
      .setSourceDC(dc)
      .setSourceCoordinates(srcRect)
      .setTargetCoordinates(dest)
      .setOpacity(opacity);

    shared_ptr<Surface> before = graphics.renderToSurface();

    loadDCToDC1(graphics, dc, srcRect, dest, opacity);
    blitDC1toDC0(machine);

    shared_ptr<Surface> after = graphics.renderToSurface();
    performEffect(machine, after, before, time, style, direction,
                  interpolation, xsize, ysize, a, b, c);
  }
};

// -----------------------------------------------------------------------
// {grp,rec}Open
// -----------------------------------------------------------------------

// Implements op<1:Grp:00076, 1>, fun grpOpen(strC 'filename', '#SEL', 'opacity').
//
// Load and display a bitmap. |filename| is loaded into DC1 with opacity
// |opacity|, and then is passed off to whatever transition effect, which will
// perform some intermediary steps and then render DC1 to DC0.
//
// TODO(erg): factor out the common code between grpOpens!
struct open_1 : public RLOp_Void_3<StrConstant_T, IntConstant_T,
                                   IntConstant_T > {
  bool use_alpha_;
  explicit open_1(bool in) : use_alpha_(in) {}

  void operator()(RLMachine& machine, string filename, int effectNum,
                  int opacity) {
    Rect src;
    Point dest;
    getSELPointAndRect(machine, effectNum, src, dest);

    GraphicsSystem& graphics = machine.system().graphics();
    shared_ptr<Surface> before = graphics.renderToSurface();

    graphics.addGraphicsStackFrame(GRP_OPEN)
      .setFilename(filename)
      .setSourceCoordinates(src)
      .setTargetCoordinates(dest)
      .setOpacity(opacity)
      .setMask(use_alpha_);

    loadImageToDC1(machine, filename, src, dest, opacity, use_alpha_);
    blitDC1toDC0(machine);

    shared_ptr<Surface> after = graphics.renderToSurface();
    performEffect(machine, after, before, effectNum);
    machine.system().text().hideAllTextWindows();
  }
};

// Implements op<1:Grp:00076, 0>, fun grpOpen(strC 'filename', '\#SEL').
//
// Load and display a bitmap. |filename| is loaded into DC1, and then is passed
// off to whatever transition effect, which will perform some intermediary
// steps and then render DC1 to DC0.
struct open_0 : public RLOp_Void_2<StrConstant_T, IntConstant_T> {
  open_1 delegate_;
  explicit open_0(bool in) : delegate_(in) {}

  void operator()(RLMachine& machine, string filename, int effectNum) {
    vector<int> selEffect = getSELEffect(machine, effectNum);
    delegate_(machine, filename, effectNum, selEffect[14]);
  }
};

template<typename SPACE>
struct open_3 : public RLOp_Void_5<
  StrConstant_T, IntConstant_T, Rect_T<SPACE>, Point_T, IntConstant_T> {
  bool use_alpha_;
  explicit open_3(bool in) : use_alpha_(in) {}

  void operator()(RLMachine& machine, string filename, int effectNum,
                  Rect srcRect, Point dest, int opacity) {
    GraphicsSystem& graphics = machine.system().graphics();

    graphics.addGraphicsStackFrame(GRP_OPEN)
      .setFilename(filename)
      .setSourceCoordinates(srcRect)
      .setTargetCoordinates(dest)
      .setOpacity(opacity)
      .setMask(use_alpha_);

    shared_ptr<Surface> before = graphics.renderToSurface();

    // Kanon uses the recOpen('?', ...) form for rendering Last Regrets. This
    // isn't documented in the rldev manual.
    loadImageToDC1(machine, filename, srcRect, dest, opacity, use_alpha_);
    blitDC1toDC0(machine);

    shared_ptr<Surface> after = graphics.renderToSurface();
    performEffect(machine, after, before, effectNum);
    machine.system().text().hideAllTextWindows();
  }
};

// Implements op<1:Grp:00076, 1>, fun grpOpen(strC 'filename', '\#SEL', 'opacity').
//
// Load and display a bitmap. |filename| is loaded into DC1 with opacity
// |opacity|, and then is passed off to whatever transition effect, which will
// perform some intermediary steps and then render DC1 to DC0.
template<typename SPACE>
struct open_2 : public RLOp_Void_4<
  StrConstant_T, IntConstant_T, Rect_T<SPACE>, Point_T> {
  open_3<SPACE> delegate_;
  explicit open_2(bool in) : delegate_(in) {}

  void operator()(RLMachine& machine, string filename, int effectNum,
                  Rect src, Point dest) {
    int opacity = getSELEffect(machine, effectNum).at(14);
    delegate_(machine, filename, effectNum, src, dest, opacity);
  }
};

template<typename SPACE>
struct open_4 : public RLOp_Void_13<
  StrConstant_T, Rect_T<SPACE>,
  Point_T, IntConstant_T, IntConstant_T, IntConstant_T,
  IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T,
  IntConstant_T, IntConstant_T> {
  bool use_alpha_;
  explicit open_4(bool in) : use_alpha_(in) {}

  void operator()(RLMachine& machine, string fileName,
                  Rect srcRect, Point dest,
                  int time, int style, int direction, int interpolation,
                  int xsize, int ysize, int a, int b, int opacity, int c) {
    GraphicsSystem& graphics = machine.system().graphics();

    graphics.addGraphicsStackFrame(GRP_OPEN)
      .setFilename(fileName)
      .setSourceCoordinates(srcRect)
      .setTargetCoordinates(dest)
      .setOpacity(opacity)
      .setMask(use_alpha_);

    shared_ptr<Surface> before = graphics.renderToSurface();

    // Kanon uses the recOpen('?', ...) form for rendering Last Regrets. This
    // isn't documented in the rldev manual.
    loadImageToDC1(machine, fileName, srcRect, dest, opacity, use_alpha_);
    blitDC1toDC0(machine);

    shared_ptr<Surface> after = graphics.renderToSurface();
    performEffect(machine, after, before, time, style, direction,
                  interpolation, xsize, ysize, a, b, c);
    machine.system().text().hideAllTextWindows();
  }
};

// TODO(erg): I don't appear to be setting the default '???' filename! Handle
// that next?
struct openBg_1 : public RLOp_Void_3<StrConstant_T, IntConstant_T,
                                     IntConstant_T > {
  void operator()(RLMachine& machine, string fileName, int effectNum,
                  int opacity) {
    GraphicsSystem& graphics = machine.system().graphics();
    Rect srcRect;
    Point destPoint;
    getSELPointAndRect(machine, effectNum, srcRect, destPoint);

    // openBg commands clears the graphics stack.
    graphics.clearStack();

    graphics.addGraphicsStackFrame(GRP_OPENBG)
      .setFilename(fileName)
      .setSourceCoordinates(srcRect)
      .setTargetCoordinates(destPoint)
      .setOpacity(opacity);

    shared_ptr<Surface> before = graphics.renderToSurface();

    loadImageToDC1(machine, fileName, srcRect, destPoint, opacity, false);
    blitDC1toDC0(machine);

    shared_ptr<Surface> after = graphics.renderToSurface();
    performEffect(machine, after, before, effectNum);
    machine.system().text().hideAllTextWindows();
  }
};

struct openBg_0 : public RLOp_Void_2< StrConstant_T, IntConstant_T > {
  openBg_1 delegate_;

  void operator()(RLMachine& machine, string filename, int effectNum) {
    vector<int> selEffect = getSELEffect(machine, effectNum);
    delegate_(machine, filename, effectNum, selEffect[14]);
  }
};

template<typename SPACE>
struct openBg_3 : public RLOp_Void_5<
  StrConstant_T, IntConstant_T, Rect_T<SPACE>, Point_T, IntConstant_T> {
  bool use_alpha_;
  explicit openBg_3(bool in) : use_alpha_(in) {}

  void operator()(RLMachine& machine, string fileName, int effectNum,
                  Rect srcRect, Point destPt, int opacity) {
    GraphicsSystem& graphics = machine.system().graphics();

    // openBg commands clears the graphics stack.
    graphics.clearStack();

    graphics.addGraphicsStackFrame(GRP_OPENBG)
      .setFilename(fileName)
      .setSourceCoordinates(srcRect)
      .setTargetCoordinates(destPt)
      .setOpacity(opacity);

    // Set the long operation for the correct transition long operation
    shared_ptr<Surface> before = graphics.renderToSurface();

    loadImageToDC1(machine, fileName, srcRect, destPt, opacity, use_alpha_);
    blitDC1toDC0(machine);

    shared_ptr<Surface> after = graphics.renderToSurface();
    performEffect(machine, after, before, effectNum);
    machine.system().text().hideAllTextWindows();
  }
};

template<typename SPACE>
struct openBg_2
    : public RLOp_Void_4<StrConstant_T, IntConstant_T, Rect_T<SPACE>, Point_T> {
  openBg_3<SPACE> delegate_;
  explicit openBg_2(bool in) : delegate_(in) {}

  void operator()(RLMachine& machine, string fileName, int effectNum,
                  Rect srcRect, Point destPt) {
    vector<int> selEffect = getSELEffect(machine, effectNum);
    delegate_(machine, fileName, effectNum, srcRect, destPt, selEffect[14]);
  }
};

template<typename SPACE>
struct openBg_4 : public RLOp_Void_13<
  StrConstant_T, Rect_T<SPACE>, Point_T,
  IntConstant_T, IntConstant_T, IntConstant_T,
  IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T,
  IntConstant_T, IntConstant_T> {
  bool use_alpha_;
  explicit openBg_4(bool in) : use_alpha_(in) {}

  void operator()(RLMachine& machine, string fileName,
                  Rect srcRect, Point destPt,
                  int time, int style, int direction, int interpolation,
                  int xsize, int ysize, int a, int b, int opacity, int c) {
    GraphicsSystem& graphics = machine.system().graphics();

    // openBg commands clears the graphics stack.
    graphics.clearStack();

    graphics.addGraphicsStackFrame(GRP_OPENBG)
      .setFilename(fileName)
      .setSourceCoordinates(srcRect)
      .setTargetCoordinates(destPt)
      .setOpacity(opacity);

    // Set the long operation for the correct transition long operation
    shared_ptr<Surface> before = graphics.renderToSurface();

    loadImageToDC1(machine, fileName, srcRect, destPt, opacity, use_alpha_);
    blitDC1toDC0(machine);

    // Render the screen to a temporary
    shared_ptr<Surface> after = graphics.renderToSurface();
    performEffect(machine, after, before, time, style, direction,
                  interpolation, xsize, ysize, a, b, c);

    machine.system().text().hideAllTextWindows();
  }
};

// -----------------------------------------------------------------------
// {grp,rec}Copy
// -----------------------------------------------------------------------
template<typename SPACE>
struct copy_3
    : public RLOp_Void_5<Rect_T<SPACE>, IntConstant_T, Point_T, IntConstant_T,
                         DefaultIntValue_T<255> > {
  bool use_alpha_;
  explicit copy_3(bool in) : use_alpha_(in) {}

  void operator()(RLMachine& machine, Rect srcRect,
                  int src, Point destPoint, int dst, int opacity) {
    // Copying to self is a noop
    if (src == dst)
      return;

    GraphicsSystem& graphics = machine.system().graphics();
    graphics.addGraphicsStackFrame(GRP_COPY)
      .setSourceCoordinates(srcRect)
      .setSourceDC(src)
      .setTargetCoordinates(destPoint)
      .setTargetDC(dst)
      .setOpacity(opacity)
      .setMask(use_alpha_);

    shared_ptr<Surface> sourceSurface = graphics.getDC(src);

    if (dst != 0 && dst != 1) {
      graphics.setMinimumSizeForDC(dst, srcRect.size());
    }

    sourceSurface->blitToSurface(
      *graphics.getDC(dst),
      srcRect, Rect(destPoint, srcRect.size()), opacity, use_alpha_);
  }
};

struct copy_1 : public RLOp_Void_3<IntConstant_T, IntConstant_T,
                                   DefaultIntValue_T<255> > {
  bool use_alpha_;
  explicit copy_1(bool in) : use_alpha_(in) {}

  void operator()(RLMachine& machine, int src, int dst, int opacity) {
    // Copying to self is a noop
    if (src == dst)
      return;

    GraphicsSystem& graphics = machine.system().graphics();
    graphics.addGraphicsStackFrame(GRP_COPY)
      .setSourceDC(src)
      .setTargetDC(dst)
      .setOpacity(opacity)
      .setMask(use_alpha_);

    shared_ptr<Surface> sourceSurface = graphics.getDC(src);

    if (dst != 0 && dst != 1) {
      graphics.setMinimumSizeForDC(dst, sourceSurface->size());
    }

    sourceSurface->blitToSurface(
      *graphics.getDC(dst), sourceSurface->rect(), sourceSurface->rect(),
      opacity, use_alpha_);
  }
};

// -----------------------------------------------------------------------
// {grp,rec}Fill
// -----------------------------------------------------------------------

struct fill_0 : public RLOp_Void_2<IntConstant_T, RGBColour_T> {
  void operator()(RLMachine& machine, int dc, RGBAColour colour) {
    // Justification: Maiden Halo uses fill(x, 0, 0, 0) as a synanom for clear
    // and since it uses haikei, the DC0 needs to be transparent.
    if (colour.r() == 0 && colour.g() == 0 && colour.b() == 0)
      colour.setAlpha(0);

    machine.system().graphics().getDC(dc)->fill(colour);
  }
};

struct fill_1 : public RLOp_Void_2<IntConstant_T, RGBMaybeAColour_T> {
  void operator()(RLMachine& machine, int dc, RGBAColour colour) {
    machine.system().graphics().getDC(dc)->fill(colour);
  }
};

template<typename SPACE>
struct fill_3 : public RLOp_Void_3<
  Rect_T<SPACE>, IntConstant_T, RGBMaybeAColour_T> {
  void operator()(RLMachine& machine, Rect destRect, int dc,
                  RGBAColour colour) {
    machine.system().graphics().getDC(dc)->fill(colour, destRect);
  }
};

struct invert_1 : public RLOp_Void_1<IntConstant_T> {
  void operator()(RLMachine& machine, int dc) {
    boost::shared_ptr<Surface> surface = machine.system().graphics().getDC(dc);
    surface->invert(surface->rect());
  }
};

template<typename SPACE>
struct invert_3 : public RLOp_Void_2<Rect_T<SPACE>, IntConstant_T> {
  void operator()(RLMachine& machine, Rect rect, int dc) {
    machine.system().graphics().getDC(dc)->invert(rect);
  }
};

struct mono_1 : public RLOp_Void_1<IntConstant_T> {
  void operator()(RLMachine& machine, int dc) {
    boost::shared_ptr<Surface> surface = machine.system().graphics().getDC(dc);
    surface->mono(surface->rect());
  }
};

template<typename SPACE>
struct mono_3 : public RLOp_Void_2<Rect_T<SPACE>, IntConstant_T> {
  void operator()(RLMachine& machine, Rect rect, int dc) {
    machine.system().graphics().getDC(dc)->mono(rect);
  }
};

struct colour_1 : public RLOp_Void_2<IntConstant_T, RGBColour_T> {
  void operator()(RLMachine& machine, int dc, RGBAColour colour) {
    boost::shared_ptr<Surface> surface = machine.system().graphics().getDC(dc);
    surface->applyColour(colour.rgb(), surface->rect());
  }
};

template<typename SPACE>
struct colour_2 : public RLOp_Void_3<Rect_T<SPACE>, IntConstant_T,
                                         RGBColour_T> {
  void operator()(RLMachine& machine, Rect rect, int dc, RGBAColour colour) {
    boost::shared_ptr<Surface> surface = machine.system().graphics().getDC(dc);
    surface->applyColour(colour.rgb(), rect);
  }
};

struct light_1 : public RLOp_Void_2<IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, int dc, int level) {
    boost::shared_ptr<Surface> surface = machine.system().graphics().getDC(dc);
    surface->applyColour(RGBColour(level, level, level), surface->rect());
  }
};

template<typename SPACE>
struct light_2 : public RLOp_Void_3<Rect_T<SPACE>, IntConstant_T,
                                         IntConstant_T> {
  void operator()(RLMachine& machine, Rect rect, int dc, int level) {
    boost::shared_ptr<Surface> surface = machine.system().graphics().getDC(dc);
    surface->applyColour(RGBColour(level, level, level), rect);
  }
};

// -----------------------------------------------------------------------
// {grp,rec}Fade
// -----------------------------------------------------------------------

template<typename SPACE>
struct fade_7 : public RLOp_Void_3<
  Rect_T<SPACE>, RGBColour_T, DefaultIntValue_T<0> > {
  void operator()(RLMachine& machine, Rect rect,
                  RGBAColour colour, int time) {
    GraphicsSystem& graphics = machine.system().graphics();
    shared_ptr<Surface> before = graphics.renderToSurface();
    graphics.getDC(0)->fill(colour, rect);
    shared_ptr<Surface> after = graphics.renderToSurface();

    if (time > 0) {
      performEffect(machine, after, before, time, 0, 0, 0, 0, 0, 0, 0, 0);
    }
  }
};

template<typename SPACE>
struct fade_5 : public RLOp_Void_3<
  Rect_T<SPACE>, IntConstant_T, DefaultIntValue_T<0> > {
  fade_7<SPACE> delegate_;

  void operator()(RLMachine& machine, Rect rect, int colour_num, int time) {
    Gameexe& gexe = machine.system().gameexe();
    const vector<int>& rgb = gexe("COLOR_TABLE", colour_num).to_intVector();
    delegate_(machine, rect, RGBAColour(rgb), time);
  }
};

struct fade_3 : public RLOp_Void_2<RGBColour_T, DefaultIntValue_T<0> > {
  fade_7<rect_impl::REC> delegate_;

  void operator()(RLMachine& machine, RGBAColour colour, int time) {
    Size screenSize = machine.system().graphics().screenSize();
    delegate_(machine, Rect(0, 0, screenSize), colour, time);
  }
};

struct fade_1 : public RLOp_Void_2<IntConstant_T, DefaultIntValue_T<0> > {
  fade_7<rect_impl::REC> delegate_;

  void operator()(RLMachine& machine, int colour_num, int time) {
    Size screenSize = machine.system().graphics().screenSize();
    Gameexe& gexe = machine.system().gameexe();
    const vector<int>& rgb = gexe("COLOR_TABLE", colour_num).to_intVector();
    delegate_(machine, Rect(0, 0, screenSize), RGBAColour(rgb), time);
  }
};


// -----------------------------------------------------------------------
// {grp,rec}StretchBlit
// -----------------------------------------------------------------------
template<typename SPACE>
struct stretchBlit_1
    : public RLOp_Void_5<Rect_T<SPACE>, IntConstant_T,
                         Rect_T<SPACE>, IntConstant_T,
                         DefaultIntValue_T<255> > {
  bool use_alpha_;
  explicit stretchBlit_1(bool in) : use_alpha_(in) {}

  void operator()(RLMachine& machine, Rect src_rect, int src,
                  Rect dst_rect, int dst, int opacity) {
    // Copying to self is a noop
    if (src == dst)
      return;

    GraphicsSystem& graphics = machine.system().graphics();
    shared_ptr<Surface> sourceSurface = graphics.getDC(src);

    if (dst != 0 && dst != 1) {
      graphics.setMinimumSizeForDC(dst, sourceSurface->size());
    }

    sourceSurface->blitToSurface(
      *graphics.getDC(dst), src_rect, dst_rect, opacity, use_alpha_);
  }
};

template<typename SPACE>
struct zoom : public RLOp_Void_5<
  Rect_T<SPACE>, Rect_T<SPACE>, IntConstant_T, Rect_T<SPACE>, IntConstant_T> {
  void operator()(RLMachine& machine, Rect frect, Rect trect, int srcDC,
                  Rect drect, int time) {
    GraphicsSystem& gs = machine.system().graphics();
    gs.setGraphicsBackground(BACKGROUND_DC0);

    LongOperation* zoomOp =
      new ZoomLongOperation(
        machine, gs.getDC(0), gs.getDC(srcDC),
        frect, trect, drect, time);
    BlitAfterEffectFinishes* blitOp =
      new BlitAfterEffectFinishes(
        zoomOp,
        gs.getDC(srcDC), gs.getDC(0),
        trect, drect);
    machine.pushLongOperation(blitOp);
  }
};

// -----------------------------------------------------------------------
// {grp,rec}multi
// -----------------------------------------------------------------------

// Defines the fairly complex parameter definition for the list of functions to
// call in a {grp,rec}Multi command.
typedef Argc_T<
  Special_T<
    // 0:copy(strC 'filename')
    StrConstant_T,
    // 1:copy(strC 'filename', 'effect')
    Complex2_T<StrConstant_T, IntConstant_T>,
    // 2:copy(strC 'filename', 'effect', 'alpha')
    Complex3_T<StrConstant_T, IntConstant_T, IntConstant_T>,
    // 3:area(strC 'filename', 'x1', 'y1', 'x2', 'y2', 'dx', 'dy')
    Complex7_T<StrConstant_T, IntConstant_T, IntConstant_T,
               IntConstant_T, IntConstant_T, IntConstant_T,
               IntConstant_T>,
    // 4:area(strC 'filename', 'x1', 'y1', 'x2', 'y2', 'dx', 'dy', 'alpha')
    Complex8_T<StrConstant_T, IntConstant_T, IntConstant_T,
               IntConstant_T, IntConstant_T, IntConstant_T,
               IntConstant_T, IntConstant_T>
    > >  MultiCommand;

// -----------------------------------------------------------------------

// Defines the weird multi commands. I will be the first to admit that the
// following is fairly difficult to read; it comes from the quagmire of
// composing Special_T and ComplexX_T templates.
//
// In the end, this operation struct simply dispatches the Special/Complex
// commands to functions and other operation structs that are clearer in
// purpose.
//
// TODO(erg): Finish this operation; it's in an incomplete, but compiling state.

/// All work is applied to DC 1.
const int MULTI_TARGET_DC = 1;

template<typename SPACE>
struct multi_command {
  void handleMultiCommands(
    RLMachine& machine, const MultiCommand::type& commands);
};

template<typename SPACE>
void multi_command<SPACE>::handleMultiCommands(
    RLMachine& machine, const MultiCommand::type& commands) {
  for (MultiCommand::type::const_iterator it = commands.begin();
       it != commands.end(); it++) {
    switch (it->type) {
    case 0:
      // 0:copy(strC 'filename')
      load_1(true)(machine, it->first, MULTI_TARGET_DC, 255);
      break;
    case 1: {
      // 1:copy(strC 'filename', 'effect')
      Rect src;
      Point dest;
      getSELPointAndRect(machine, it->second.get<1>(), src, dest);

      load_3<SPACE>(true)
        (machine, it->second.get<0>(), MULTI_TARGET_DC,
         src, dest, 255);
      break;
    }
    case 2: {
      // 2:copy(strC 'filename', 'effect', 'alpha')
      Rect src;
      Point dest;
      getSELPointAndRect(machine, it->third.get<1>(), src, dest);

      load_3<SPACE>(true)
        (machine, it->third.get<0>(), MULTI_TARGET_DC,
         src, dest, it->third.get<2>());
      break;
    }
    case 3: {
      // 3:area(strC 'filename', 'x1', 'y1', 'x2', 'y2', 'dx', 'dy')
      load_3<SPACE>(true)
        (machine, it->fourth.get<0>(), MULTI_TARGET_DC,
         SPACE::makeRect(it->fourth.get<1>(), it->fourth.get<2>(),
                         it->fourth.get<3>(), it->fourth.get<4>()),
         Point(it->fourth.get<5>(), it->fourth.get<6>()), 255);
      break;
    }
    case 4: {
      // 4:area(strC 'filename', 'x1', 'y1', 'x2', 'y2', 'dx', 'dy', 'alpha')
      load_3<SPACE>(true)
        (machine, it->fifth.get<0>(), MULTI_TARGET_DC,
         SPACE::makeRect(it->fifth.get<1>(), it->fifth.get<2>(),
                         it->fifth.get<3>(), it->fifth.get<4>()),
         Point(it->fifth.get<5>(), it->fifth.get<6>()), it->fifth.get<7>());
      break;
    }
    }
  }
}

// fun grpMulti <1:Grp:00075, 4> (<strC 'filename', <'effect', MultiCommand)
template<typename SPACE>
struct multi_str_1
    : public RLOp_Void_4<StrConstant_T, IntConstant_T, IntConstant_T,
                         MultiCommand>,
      public multi_command<SPACE> {
  void operator()(RLMachine& machine, string filename, int effect, int alpha,
                  MultiCommand::type commands) {
    load_1(false)(machine, filename, MULTI_TARGET_DC, 255);
    multi_command<SPACE>::handleMultiCommands(machine, commands);
    display_0()(machine, MULTI_TARGET_DC, effect);
  }
};

template<typename SPACE>
struct multi_str_0
    : public RLOp_Void_3<StrConstant_T, IntConstant_T, MultiCommand> {
  multi_str_1<SPACE> delegate_;

  void operator()(RLMachine& machine, string filename, int effect,
                  MultiCommand::type commands) {
    delegate_(machine, filename, effect, 255, commands);
  }
};

template<typename SPACE>
struct multi_dc_1
    : public RLOp_Void_4<IntConstant_T, IntConstant_T, IntConstant_T,
                         MultiCommand>,
      public multi_command<SPACE> {
  void operator()(RLMachine& machine, int dc, int effect, int alpha,
                  MultiCommand::type commands) {
    copy_1(false)(machine, dc, MULTI_TARGET_DC, 255);
    multi_command<SPACE>::handleMultiCommands(machine, commands);
    display_0()(machine, MULTI_TARGET_DC, effect);
  }
};

template<typename SPACE>
struct multi_dc_0
    : public RLOp_Void_3<IntConstant_T, IntConstant_T, MultiCommand> {
  multi_dc_1<SPACE> delegate_;

  void operator()(RLMachine& machine, int dc, int effect,
                  MultiCommand::type commands) {
    delegate_(machine, dc, effect, 255, commands);
  }
};

} // namespace

GrpModule::GrpModule()
    : RLModule("Grp", 1, 33) {
  using namespace rect_impl;

  addOpcode(15, 0, "allocDC", new allocDC);
  addOpcode(16, 0, "freeDC", callFunction(&GraphicsSystem::freeDC));

  addUnsupportedOpcode(20, 0, "grpLoadMask");
  // addOpcode(30, 0, new grpTextout);

  addOpcode(31, 0, "wipe", new wipe);
  addUnsupportedOpcode(32, 0, "shake");

  addOpcode(50, 0, "grpLoad", new load_1(false));
  addOpcode(50, 1, "grpLoad", new load_1(false));
  addOpcode(50, 2, "grpLoad", new load_3<GRP>(false));
  addOpcode(50, 3, "grpLoad", new load_3<GRP>(false));
  addOpcode(51, 0, "grpMaskLoad", new load_1(true));
  addOpcode(51, 1, "grpMaskLoad", new load_1(true));
  addOpcode(51, 2, "grpMaskLoad", new load_3<GRP>(true));
  addOpcode(51, 3, "grpMaskLoad", new load_3<GRP>(true));

  // These are grpBuffer, which is very similar to grpLoad and Haeleth
  // doesn't know how they differ. For now, we just assume they're
  // equivalent.
  addOpcode(70, 0, "grpBuffer", new load_1(false));
  addOpcode(70, 1, "grpBuffer", new load_1(false));
  addOpcode(70, 2, "grpBuffer", new load_3<GRP>(false));
  addOpcode(70, 3, "grpBuffer", new load_3<GRP>(false));
  addOpcode(71, 0, "grpMaskBuffer", new load_1(true));
  addOpcode(71, 1, "grpMaskBuffer", new load_1(true));
  addOpcode(71, 2, "grpMaskBuffer", new load_3<GRP>(true));
  addOpcode(71, 3, "grpMaskBuffer", new load_3<GRP>(true));

  addOpcode(72, 0, "grpDisplay", new display_0);
  addOpcode(72, 1, "grpDisplay", new display_1);
  addOpcode(72, 2, "grpDisplay", new display_2<GRP>());
  addOpcode(72, 3, "grpDisplay", new display_3<GRP>());
  addOpcode(72, 4, "grpDisplay", new display_4<GRP>());

  addOpcode(73, 0, "grpOpenBg", new openBg_0);
  addOpcode(73, 1, "grpOpenBg", new openBg_1);
  addOpcode(73, 2, "grpOpenBg", new openBg_2<GRP>(false));
  addOpcode(73, 3, "grpOpenBg", new openBg_3<GRP>(false));
  addOpcode(73, 4, "grpOpenBg", new openBg_4<GRP>(false));

  addOpcode(74, 0, "grpMaskOpen", new open_0(true));
  addOpcode(74, 1, "grpMaskOpen", new open_1(true));
  addOpcode(74, 2, "grpMaskOpen", new open_2<GRP>(true));
  addOpcode(74, 3, "grpMaskOpen", new open_3<GRP>(true));
  addOpcode(74, 4, "grpMaskOpen", new open_4<GRP>(true));

  addOpcode(75, 0, "grpMulti", new multi_str_0<GRP>());
  addOpcode(75, 1, "grpMulti", new multi_str_1<GRP>());
  addUnsupportedOpcode(75, 2, "grpMulti");
  addUnsupportedOpcode(75, 3, "grpMulti");
  addUnsupportedOpcode(75, 4, "grpMulti");

  addOpcode(77, 0, "grpMulti", new multi_dc_0<GRP>());
  addOpcode(77, 1, "grpMulti", new multi_dc_1<GRP>());
  addUnsupportedOpcode(77, 2, "grpMulti");
  addUnsupportedOpcode(77, 3, "grpMulti");
  addUnsupportedOpcode(77, 4, "grpMulti");

  addOpcode(76, 0, "grpOpen", new open_0(false));
  addOpcode(76, 1, "grpOpen", new open_1(false));
  addOpcode(76, 2, "grpOpen", new open_2<GRP>(false));
  addOpcode(76, 3, "grpOpen", new open_3<GRP>(false));
  addOpcode(76, 4, "grpOpen", new open_4<GRP>(false));

  addUnsupportedOpcode(77, 0, "grpMulti");
  addUnsupportedOpcode(77, 1, "grpMulti");
  addUnsupportedOpcode(77, 2, "grpMulti");
  addUnsupportedOpcode(77, 3, "grpMulti");
  addUnsupportedOpcode(77, 4, "grpMulti");

  addOpcode(100, 0, "grpCopy", new copy_1(false));
  addOpcode(100, 1, "grpCopy", new copy_1(false));
  addOpcode(100, 2, "grpCopy", new copy_3<GRP>(false));
  addOpcode(100, 3, "grpCopy", new copy_3<GRP>(false));
  addOpcode(101, 0, "grpMaskCopy", new copy_1(true));
  addOpcode(101, 1, "grpMaskCopy", new copy_1(true));
  addOpcode(101, 2, "grpMaskCopy", new copy_3<GRP>(true));
  addOpcode(101, 3, "grpMaskCopy", new copy_3<GRP>(true));

  addUnsupportedOpcode(120, 5, "grpCopyWithMask");
  addUnsupportedOpcode(140, 5, "grpCopyInvMask");

  addOpcode(201, 0, "grpFill", new fill_0);
  addOpcode(201, 1, "grpFill", new fill_1);
  addOpcode(201, 2, "grpFill", new fill_3<GRP>());
  addOpcode(201, 3, "grpFill", new fill_3<GRP>());

  addOpcode(300, 0, "grpInvert", new invert_1);
  addUnsupportedOpcode(300, 1, "grpInvert");
  addOpcode(300, 2, "grpInvert", new invert_3<GRP>());
  addUnsupportedOpcode(300, 3, "grpInvert");

  addOpcode(301, 0, "grpMono", new mono_1);
  addUnsupportedOpcode(301, 1, "grpMono");
  addOpcode(301, 2, "grpMono", new mono_3<GRP>());
  addUnsupportedOpcode(301, 3, "grpMono");

  addOpcode(302, 0, "grpColour", new colour_1);
  addOpcode(302, 1, "grpColour", new colour_2<GRP>());

  addOpcode(303, 0, "grpLight", new light_1);
  addOpcode(303, 1, "grpLight", new light_2<GRP>());

  addUnsupportedOpcode(400, 0, "grpSwap");
  addUnsupportedOpcode(400, 1, "grpSwap");

  addOpcode(401, 0, "grpStretchBlt", new stretchBlit_1<GRP>(false));
  addOpcode(401, 1, "grpStretchBlt", new stretchBlit_1<GRP>(false));

  addOpcode(402, 0, "grpZoom", new zoom<GRP>());

  addOpcode(403, 0, "grpFade", new fade_1);
  addOpcode(403, 1, "grpFade", new fade_1);
  addOpcode(403, 2, "grpFade", new fade_3);
  addOpcode(403, 3, "grpFade", new fade_3);
  addOpcode(403, 4, "grpFade", new fade_5<GRP>());
  addOpcode(403, 5, "grpFade", new fade_5<GRP>());
  addOpcode(403, 6, "grpFade", new fade_7<GRP>());
  addOpcode(403, 7, "grpFade", new fade_7<GRP>());

  addOpcode(409, 0, "grpMaskStretchBlt", new stretchBlit_1<GRP>(true));
  addOpcode(409, 1, "grpMaskStretchBlt", new stretchBlit_1<GRP>(true));

  addUnsupportedOpcode(601, 0, "grpMaskAdd");
  addUnsupportedOpcode(601, 1, "grpMaskAdd");
  addUnsupportedOpcode(601, 2, "grpMaskAdd");
  addUnsupportedOpcode(601, 3, "grpMaskAdd");

  // -----------------------------------------------------------------------

  addOpcode(1050, 0, "recLoad", new load_1(false));
  addOpcode(1050, 1, "recLoad", new load_1(false));
  addOpcode(1050, 2, "recLoad", new load_3<REC>(false));
  addOpcode(1050, 3, "recLoad", new load_3<REC>(false));

  addOpcode(1051, 0, "recMaskLoad", new load_1(true));
  addOpcode(1051, 1, "recMaskLoad", new load_1(true));
  addOpcode(1051, 2, "recMaskLoad", new load_3<REC>(true));
  addOpcode(1051, 3, "recMaskLoad", new load_3<REC>(true));

  addOpcode(1052, 0, "recDisplay", new display_0);
  addOpcode(1052, 1, "recDisplay", new display_1);
  addOpcode(1052, 2, "recDisplay", new display_2<REC>());
  addOpcode(1052, 3, "recDisplay", new display_3<REC>());
  addOpcode(1052, 4, "recDisplay", new display_4<REC>());

  addOpcode(1053, 0, "recOpenBg", new openBg_0);
  addOpcode(1053, 1, "recOpenBg", new openBg_1);
  addOpcode(1053, 2, "recOpenBg", new openBg_2<REC>(false));
  addOpcode(1053, 3, "recOpenBg", new openBg_3<REC>(false));
  addOpcode(1053, 4, "recOpenBg", new openBg_4<REC>(false));

  addOpcode(1054, 0, "recMaskOpen", new open_0(true));
  addOpcode(1054, 1, "recMaskOpen", new open_1(true));
  addOpcode(1054, 2, "recMaskOpen", new open_2<REC>(true));
  addOpcode(1054, 3, "recMaskOpen", new open_3<REC>(true));
  addOpcode(1054, 4, "recMaskOpen", new open_4<REC>(true));

  addOpcode(1056, 0, "recOpen", new open_0(false));
  addOpcode(1056, 1, "recOpen", new open_1(false));
  addOpcode(1056, 2, "recOpen", new open_2<REC>(false));
  addOpcode(1056, 3, "recOpen", new open_3<REC>(false));
  addOpcode(1056, 4, "recOpen", new open_4<REC>(false));

  addOpcode(1055, 0, "recMulti", new multi_str_0<REC>());
  addOpcode(1055, 1, "recMulti", new multi_str_1<REC>());
  addUnsupportedOpcode(1055, 2, "recMulti");
  addUnsupportedOpcode(1055, 3, "recMulti");
  addUnsupportedOpcode(1055, 4, "recMulti");

  addOpcode(1057, 0, "recMulti", new multi_dc_0<REC>());
  addOpcode(1057, 1, "recMulti", new multi_dc_1<REC>());
  addUnsupportedOpcode(1057, 2, "recMulti");
  addUnsupportedOpcode(1057, 3, "recMulti");
  addUnsupportedOpcode(1057, 4, "recMulti");

  addOpcode(1100, 0, "recCopy", new copy_1(false));
  addOpcode(1100, 1, "recCopy", new copy_1(false));
  addOpcode(1100, 2, "recCopy", new copy_3<REC>(false));
  addOpcode(1100, 3, "recCopy", new copy_3<REC>(false));
  addOpcode(1101, 0, "recMaskCopy", new copy_1(true));
  addOpcode(1101, 1, "recMaskCopy", new copy_1(true));
  addOpcode(1101, 2, "recMaskCopy", new copy_3<REC>(true));
  addOpcode(1101, 3, "recMaskCopy", new copy_3<REC>(true));

  addOpcode(1201, 0, "recFill", new fill_0);
  addOpcode(1201, 1, "recFill", new fill_1);
  addOpcode(1201, 2, "recFill", new fill_3<REC>());
  addOpcode(1201, 3, "recFill", new fill_3<REC>());

  addOpcode(1300, 0, "recInvert", new invert_1);
  addUnsupportedOpcode(1300, 1, "recInvert");
  addOpcode(1300, 2, "recInvert", new invert_3<REC>());
  addUnsupportedOpcode(1300, 3, "recInvert");

  addOpcode(1301, 0, "recMono", new mono_1);
  addUnsupportedOpcode(1301, 1, "recMono");
  addOpcode(1301, 2, "recMono", new mono_3<REC>());
  addUnsupportedOpcode(1301, 3, "recMono");

  addOpcode(1302, 0, "recColour", new colour_1);
  addOpcode(1302, 1, "recColour", new colour_2<REC>());

  addOpcode(1303, 0, "recLight", new light_1);
  addOpcode(1303, 1, "recLight", new light_2<REC>());

  addUnsupportedOpcode(1400, 0, "recSwap");
  addUnsupportedOpcode(1400, 1, "recSwap");

  addOpcode(1401, 0, "recStretchBlt", new stretchBlit_1<REC>(false));
  addOpcode(1401, 1, "recStretchBlt", new stretchBlit_1<REC>(false));

  addOpcode(1402, 0, "recZoom", new zoom<REC>());

  addOpcode(1403, 0, "recFade", new fade_1);
  addOpcode(1403, 1, "recFade", new fade_1);
  addOpcode(1403, 2, "recFade", new fade_3);
  addOpcode(1403, 3, "recFade", new fade_3);
  addOpcode(1403, 4, "recFade", new fade_5<REC>());
  addOpcode(1403, 5, "recFade", new fade_5<REC>());
  addOpcode(1403, 6, "recFade", new fade_7<REC>());
  addOpcode(1403, 7, "recFade", new fade_7<REC>());

  addUnsupportedOpcode(1404, 0, "recFlash");
  addUnsupportedOpcode(1404, 1, "recFlash");
  addUnsupportedOpcode(1404, 2, "recFlash");
  addUnsupportedOpcode(1404, 3, "recFlash");

  addUnsupportedOpcode(1406, 0, "recPan");
  addUnsupportedOpcode(1407, 0, "recShift");
  addUnsupportedOpcode(1408, 0, "recSlide");
  addOpcode(1409, 0, "recMaskStretchBlt", new stretchBlit_1<REC>(true));
  addOpcode(1409, 1, "recMaskStretchBlt", new stretchBlit_1<REC>(true));
}

void replayOpenBg(RLMachine& machine, const GraphicsStackFrame& f) {
  loadImageToDC1(machine, f.filename(), f.sourceRect(), f.targetPoint(),
                 f.opacity(), false);

  blitDC1toDC0(machine);
}

void replayGraphicsStackVector(
    RLMachine& machine,
    const std::vector<GraphicsStackFrame>& gstack) {
  for (vector<GraphicsStackFrame>::const_iterator it = gstack.begin();
       it != gstack.end(); ++it) {
    try {
      if (it->name() == GRP_LOAD) {
        if (it->hasTargetCoordinates()) {
          load_3<rect_impl::REC>(it->mask())(
              machine, it->filename(), it->targetDC(),
              it->sourceRect(), it->targetPoint(),
              it->opacity());
        } else {
          // Older versions of rlvm didn't record the mask bit, so make sure we
          // check for that since we don't want to break old save games.
          bool mask = (it->hasMask() ? it->mask() : true);
          load_1 loader(mask);
          loader(machine, it->filename(), it->targetDC(), it->opacity());
        }
      } else if (it->name() == GRP_OPEN) {
        // open is just a load + an animation.
        loadImageToDC1(machine, it->filename(), it->sourceRect(),
                       it->targetPoint(), it->opacity(), it->mask());
        blitDC1toDC0(machine);
      } else if (it->name() == GRP_COPY) {
        if (it->hasSourceCoordinates()) {
          copy_3<rect_impl::REC>(it->mask())(
              machine,
              it->sourceRect(), it->sourceDC(),
              it->targetPoint(), it->targetDC(),
              it->opacity());
        } else {
          copy_1(it->mask())(
              machine, it->sourceDC(), it->targetDC(), it->opacity());
        }
      } else if (it->name() == GRP_DISPLAY) {
        GraphicsSystem& graphics = machine.system().graphics();
        loadDCToDC1(graphics,
                    it->sourceDC(), it->sourceRect(),
                    it->targetPoint(), it->opacity());
        blitDC1toDC0(machine);
      } else if (it->name() == GRP_OPENBG) {
        replayOpenBg(machine, *it);
      } else if (it->name() == GRP_ALLOC) {
        Point target = it->targetPoint();
        allocDC()(machine, it->targetDC(), target.x(), target.y());
      } else if (it->name() == GRP_WIPE) {
        wipe()(machine, it->targetDC(), it->r(), it->g(), it->b());
      }
    } catch(rlvm::Exception& e) {
      cerr << "WARNING: Error while thawing graphics stack: " << e.what()
           << endl;
    }
  }
}
