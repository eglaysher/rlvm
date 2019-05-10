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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
//
// -----------------------------------------------------------------------

#include "modules/module_grp.h"

#include <boost/algorithm/string.hpp>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include "effects/effect.h"
#include "effects/effect_factory.h"
#include "libreallive/bytecode.h"
#include "libreallive/expression.h"
#include "libreallive/gameexe.h"
#include "long_operations/wait_long_operation.h"
#include "long_operations/zoom_long_operation.h"
#include "machine/general_operations.h"
#include "machine/rlmachine.h"
#include "machine/rloperation.h"
#include "machine/rloperation/argc_t.h"
#include "machine/rloperation/complex_t.h"
#include "machine/rloperation/default_value.h"
#include "machine/rloperation/rect_t.h"
#include "machine/rloperation/rgb_colour_t.h"
#include "machine/rloperation/special_t.h"
#include "systems/base/colour.h"
#include "systems/base/graphics_stack_frame.h"
#include "systems/base/graphics_system.h"
#include "systems/base/surface.h"
#include "systems/base/system.h"
#include "systems/base/text_system.h"
#include "utilities/graphics.h"

using std::get;

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

  std::shared_ptr<Surface> src = graphics.GetDC(1);
  std::shared_ptr<Surface> dst = graphics.GetDC(0);

  // Blit DC1 onto DC0, with full opacity, and end the operation
  src->BlitToSurface(*dst, src->GetRect(), dst->GetRect(), 255);

  // Mark that the background should be DC0 instead of the Haikei.
  graphics.set_graphics_background(BACKGROUND_DC0);

  // Promote the objects if we're in normal mode. If we're restoring the
  // graphics stack, we already have our layers promoted.
  if (!machine.replaying_graphics_stack())
    graphics.ClearAndPromoteObjects();
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
                    int opacity,
                    bool useAlpha) {
  GraphicsSystem& graphics = machine.system().graphics();

  if (name != "?") {
    if (name == "???")
      name = graphics.default_grp_name();

    std::shared_ptr<Surface> dc0 = graphics.GetDC(0);
    std::shared_ptr<Surface> dc1 = graphics.GetDC(1);

    // Inclusive ranges are a monstrosity to computer people
    Size size = srcRect.size() + Size(1, 1);

    dc0->BlitToSurface(*dc1, dc0->GetRect(), dc0->GetRect(), 255);

    // Load the section of the image file on top of dc1
    std::shared_ptr<const Surface> surface(
        graphics.GetSurfaceNamedAndMarkViewed(machine, name));
    surface->BlitToSurface(*graphics.GetDC(1),
                           Rect(srcRect.origin(), size),
                           Rect(dest, size),
                           opacity,
                           useAlpha);
  }
}

void loadDCToDC1(RLMachine& machine,
                 int srcDc,
                 const Rect& srcRect,
                 const Point& dest,
                 int opacity) {
  GraphicsSystem& graphics = machine.system().graphics();
  std::shared_ptr<Surface> dc1 = graphics.GetDC(1);
  std::shared_ptr<Surface> src = graphics.GetDC(srcDc);

  // Inclusive ranges are a monstrosity to computer people
  Size size = srcRect.size() + Size(1, 1);

  src->BlitToSurface(
      *dc1, Rect(srcRect.origin(), size), Rect(dest, size), opacity, false);
}

void performEffect(RLMachine& machine,
                   const std::shared_ptr<Surface>& src,
                   const std::shared_ptr<Surface>& dst,
                   int selnum) {
  if (!machine.replaying_graphics_stack()) {
    LongOperation* lop = EffectFactory::BuildFromSEL(machine, src, dst, selnum);
    machine.PushLongOperation(lop);
  }
}

void performEffect(RLMachine& machine,
                   const std::shared_ptr<Surface>& src,
                   const std::shared_ptr<Surface>& dst,
                   int time,
                   int style,
                   int direction,
                   int interpolation,
                   int xsize,
                   int ysize,
                   int a,
                   int b,
                   int c) {
  if (!machine.replaying_graphics_stack()) {
    LongOperation* lop = EffectFactory::Build(machine,
                                              src,
                                              dst,
                                              time,
                                              style,
                                              direction,
                                              interpolation,
                                              xsize,
                                              ysize,
                                              a,
                                              b,
                                              c);
    machine.PushLongOperation(lop);
  }
}

// We don't hide text windows when replaying the stack because hiding the
// window won't be undone like it normally is!
void performHideAllTextWindows(RLMachine& machine) {
  if (!machine.replaying_graphics_stack()) {
    machine.system().text().HideAllTextWindows();
  }
}

// Common code to all the openBg commands.
void OpenBgPrelude(RLMachine& machine, const std::string& filename) {
  if (!boost::starts_with(filename, "?")) {
    GraphicsSystem& graphics = machine.system().graphics();
    graphics.set_default_grp_name(filename);

    // Only clear the stack when we are the command setting the background.
    graphics.ClearStack();
  }
}

// Implements op<1:Grp:00015, 0>, fun allocDC('DC', 'width', 'height').
//
// Allocates a blank width * height bitmap in dc. Any DC apart from DC 0 may be
// allocated thus, although DC 1 is never given a size smaller than the screen
// resolution. Any previous contents of dc are erased.
struct allocDC
    : public RLOpcode<IntConstant_T, IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, int dc, int width, int height) {
    machine.system().graphics().AllocateDC(dc, Size(width, height));
  }
};

// Implements op<1:Grp:00031, 0>, fun wipe('DC', 'r', 'g', 'b')
//
// Fills dc with the colour indicated by the given RGB triplet.
struct wipe : public RLOpcode<IntConstant_T,
                             IntConstant_T,
                             IntConstant_T,
                             IntConstant_T> {
  void operator()(RLMachine& machine, int dc, int r, int g, int b) {
    machine.system().graphics().GetDC(dc)->Fill(RGBAColour(r, g, b));
  }
};

struct shake : public RLOpcode<IntConstant_T> {
  void operator()(RLMachine& machine, int spec) {
    machine.system().graphics().QueueShakeSpec(spec);

    WaitLongOperation* wait_op = new WaitLongOperation(machine);
    wait_op->BreakOnEvent(std::bind(StopShaking, std::ref(machine)));
    machine.PushLongOperation(wait_op);
  }

  static bool StopShaking(RLMachine& machine) {
    return machine.system().graphics().IsShaking() == false;
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
struct load_1
    : public RLOpcode<StrConstant_T, IntConstant_T, DefaultIntValue_T<255>> {
  bool use_alpha_;
  explicit load_1(bool in) : use_alpha_(in) {}

  void operator()(RLMachine& machine, string filename, int dc, int opacity) {
    GraphicsSystem& graphics = machine.system().graphics();

    std::shared_ptr<const Surface> surface(
        graphics.GetSurfaceNamedAndMarkViewed(machine, filename));

    if (dc != 0 && dc != 1) {
      graphics.AllocateDC(dc, surface->GetSize());
    }

    surface->BlitToSurface(*graphics.GetDC(dc),
                           surface->GetRect(),
                           surface->GetRect(),
                           opacity,
                           use_alpha_);
  }
};

// Implements op<1:Grp:00050, 3>, fun grpLoad(strC 'filename', 'DC',
// 'x1', 'y1', 'x2', 'y2', 'dx', 'dy', 'alpha').
//
// Loads filename into dc; note that filename may not be '???'. Using this
// form, the given area of the bitmap is loaded at the given location.
template <typename SPACE>
struct load_3 : public RLOpcode<StrConstant_T,
                               IntConstant_T,
                               Rect_T<SPACE>,
                               Point_T,
                               DefaultIntValue_T<255>> {
  bool use_alpha_;
  explicit load_3(bool in) : use_alpha_(in) {}

  void operator()(RLMachine& machine,
                  string filename,
                  int dc,
                  Rect srcRect,
                  Point dest,
                  int opacity) {
    GraphicsSystem& graphics = machine.system().graphics();
    std::shared_ptr<const Surface> surface(
        graphics.GetSurfaceNamedAndMarkViewed(machine, filename));

    Rect destRect = Rect(dest, srcRect.size());

    if (dc != 0 && dc != 1) {
      graphics.SetMinimumSizeForDC(dc, surface->GetSize());
    }

    surface->BlitToSurface(
        *graphics.GetDC(dc), srcRect, destRect, opacity, use_alpha_);
  }
};

// -----------------------------------------------------------------------
// {grp,rec}Display
// -----------------------------------------------------------------------

struct display_1
    : public RLOpcode<IntConstant_T, IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, int dc, int effectNum, int opacity) {
    Rect src;
    Point dest;
    GetSELPointAndRect(machine, effectNum, src, dest);

    GraphicsSystem& graphics = machine.system().graphics();

    std::shared_ptr<Surface> before = graphics.RenderToSurface();

    loadDCToDC1(machine, dc, src, dest, opacity);
    blitDC1toDC0(machine);

    std::shared_ptr<Surface> after = graphics.RenderToSurface();
    performEffect(machine, after, before, effectNum);
  }
};

struct display_0 : public RLOpcode<IntConstant_T, IntConstant_T> {
  display_1 delegate_;

  void operator()(RLMachine& machine, int dc, int effectNum) {
    std::vector<int> selEffect = GetSELEffect(machine, effectNum);
    delegate_(machine, dc, effectNum, selEffect.at(14));
  }
};

template <typename SPACE>
struct display_3 : public RLOpcode<IntConstant_T,
                                  IntConstant_T,
                                  Rect_T<SPACE>,
                                  Point_T,
                                  IntConstant_T> {
  void operator()(RLMachine& machine,
                  int dc,
                  int effectNum,
                  Rect srcRect,
                  Point dest,
                  int opacity) {
    GraphicsSystem& graphics = machine.system().graphics();

    std::shared_ptr<Surface> before = graphics.RenderToSurface();

    loadDCToDC1(machine, dc, srcRect, dest, opacity);
    blitDC1toDC0(machine);

    std::shared_ptr<Surface> after = graphics.RenderToSurface();
    performEffect(machine, after, before, effectNum);
  }
};

template <typename SPACE>
struct display_2
    : public RLOpcode<IntConstant_T, IntConstant_T, Rect_T<SPACE>, Point_T> {
  void operator()(RLMachine& machine,
                  int dc,
                  int effectNum,
                  Rect src_rect,
                  Point dest) {
    int opacity = GetSELEffect(machine, effectNum).at(14);
    display_3<SPACE>()(machine, dc, effectNum, src_rect, dest, opacity);
  }
};

template <typename SPACE>
struct display_4 : public RLOpcode<IntConstant_T,
                                  Rect_T<SPACE>,
                                  Point_T,
                                  IntConstant_T,
                                  IntConstant_T,
                                  IntConstant_T,
                                  IntConstant_T,
                                  IntConstant_T,
                                  IntConstant_T,
                                  IntConstant_T,
                                  IntConstant_T,
                                  IntConstant_T,
                                  IntConstant_T> {
  void operator()(RLMachine& machine,
                  int dc,
                  Rect srcRect,
                  Point dest,
                  int time,
                  int style,
                  int direction,
                  int interpolation,
                  int xsize,
                  int ysize,
                  int a,
                  int b,
                  int opacity,
                  int c) {
    GraphicsSystem& graphics = machine.system().graphics();

    std::shared_ptr<Surface> before = graphics.RenderToSurface();

    loadDCToDC1(machine, dc, srcRect, dest, opacity);
    blitDC1toDC0(machine);

    std::shared_ptr<Surface> after = graphics.RenderToSurface();
    performEffect(machine,
                  after,
                  before,
                  time,
                  style,
                  direction,
                  interpolation,
                  xsize,
                  ysize,
                  a,
                  b,
                  c);
  }
};

// -----------------------------------------------------------------------
// {grp,rec}Open
// -----------------------------------------------------------------------

// Implements op<1:Grp:00076, 1>, fun grpOpen(strC 'filename', '#SEL',
// 'opacity').
//
// Load and display a bitmap. |filename| is loaded into DC1 with opacity
// |opacity|, and then is passed off to whatever transition effect, which will
// perform some intermediary steps and then render DC1 to DC0.
//
// TODO(erg): factor out the common code between grpOpens!
struct open_1
    : public RLOpcode<StrConstant_T, IntConstant_T, IntConstant_T> {
  bool use_alpha_;
  explicit open_1(bool in) : use_alpha_(in) {}

  void operator()(RLMachine& machine,
                  string filename,
                  int effectNum,
                  int opacity) {
    Rect src;
    Point dest;
    GetSELPointAndRect(machine, effectNum, src, dest);

    GraphicsSystem& graphics = machine.system().graphics();
    std::shared_ptr<Surface> before = graphics.RenderToSurface();

    loadImageToDC1(machine, filename, src, dest, opacity, use_alpha_);
    blitDC1toDC0(machine);

    std::shared_ptr<Surface> after = graphics.RenderToSurface();
    performEffect(machine, after, before, effectNum);
    performHideAllTextWindows(machine);
  }
};

// Implements op<1:Grp:00076, 0>, fun grpOpen(strC 'filename', '\#SEL').
//
// Load and display a bitmap. |filename| is loaded into DC1, and then is passed
// off to whatever transition effect, which will perform some intermediary
// steps and then render DC1 to DC0.
struct open_0 : public RLOpcode<StrConstant_T, IntConstant_T> {
  open_1 delegate_;
  explicit open_0(bool in) : delegate_(in) {}

  void operator()(RLMachine& machine, string filename, int effectNum) {
    std::vector<int> selEffect = GetSELEffect(machine, effectNum);
    delegate_(machine, filename, effectNum, selEffect[14]);
  }
};

template <typename SPACE>
struct open_3 : public RLOpcode<StrConstant_T,
                               IntConstant_T,
                               Rect_T<SPACE>,
                               Point_T,
                               IntConstant_T> {
  bool use_alpha_;
  explicit open_3(bool in) : use_alpha_(in) {}

  void operator()(RLMachine& machine,
                  string filename,
                  int effectNum,
                  Rect srcRect,
                  Point dest,
                  int opacity) {
    GraphicsSystem& graphics = machine.system().graphics();

    std::shared_ptr<Surface> before = graphics.RenderToSurface();

    // Kanon uses the recOpen('?', ...) form for rendering Last Regrets. This
    // isn't documented in the rldev manual.
    loadImageToDC1(machine, filename, srcRect, dest, opacity, use_alpha_);
    blitDC1toDC0(machine);

    std::shared_ptr<Surface> after = graphics.RenderToSurface();
    performEffect(machine, after, before, effectNum);
    performHideAllTextWindows(machine);
  }
};

// Implements op<1:Grp:00076, 1>, fun grpOpen(strC 'filename', '\#SEL',
// 'opacity').
//
// Load and display a bitmap. |filename| is loaded into DC1 with opacity
// |opacity|, and then is passed off to whatever transition effect, which will
// perform some intermediary steps and then render DC1 to DC0.
template <typename SPACE>
struct open_2
    : public RLOpcode<StrConstant_T, IntConstant_T, Rect_T<SPACE>, Point_T> {
  open_3<SPACE> delegate_;
  explicit open_2(bool in) : delegate_(in) {}

  void operator()(RLMachine& machine,
                  string filename,
                  int effectNum,
                  Rect src,
                  Point dest) {
    int opacity = GetSELEffect(machine, effectNum).at(14);
    delegate_(machine, filename, effectNum, src, dest, opacity);
  }
};

template <typename SPACE>
struct open_4 : public RLOpcode<StrConstant_T,
                                    Rect_T<SPACE>,
                                    Point_T,
                                    IntConstant_T,
                                    IntConstant_T,
                                    IntConstant_T,
                                    IntConstant_T,
                                    IntConstant_T,
                                    IntConstant_T,
                                    IntConstant_T,
                                    IntConstant_T,
                                    IntConstant_T,
                                    IntConstant_T> {
  bool use_alpha_;
  explicit open_4(bool in) : use_alpha_(in) {}

  void operator()(RLMachine& machine,
                  string fileName,
                  Rect srcRect,
                  Point dest,
                  int time,
                  int style,
                  int direction,
                  int interpolation,
                  int xsize,
                  int ysize,
                  int a,
                  int b,
                  int opacity,
                  int c) {
    GraphicsSystem& graphics = machine.system().graphics();

    std::shared_ptr<Surface> before = graphics.RenderToSurface();

    // Kanon uses the recOpen('?', ...) form for rendering Last Regrets. This
    // isn't documented in the rldev manual.
    loadImageToDC1(machine, fileName, srcRect, dest, opacity, use_alpha_);
    blitDC1toDC0(machine);

    std::shared_ptr<Surface> after = graphics.RenderToSurface();
    performEffect(machine,
                  after,
                  before,
                  time,
                  style,
                  direction,
                  interpolation,
                  xsize,
                  ysize,
                  a,
                  b,
                  c);
    performHideAllTextWindows(machine);
  }
};

struct openBg_1
    : public RLOpcode<StrConstant_T, IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine,
                  string fileName,
                  int effectNum,
                  int opacity) {
    GraphicsSystem& graphics = machine.system().graphics();
    Rect srcRect;
    Point destPoint;
    GetSELPointAndRect(machine, effectNum, srcRect, destPoint);

    OpenBgPrelude(machine, fileName);

    std::shared_ptr<Surface> before = graphics.RenderToSurface();

    loadImageToDC1(machine, fileName, srcRect, destPoint, opacity, false);
    blitDC1toDC0(machine);

    std::shared_ptr<Surface> after = graphics.RenderToSurface();
    performEffect(machine, after, before, effectNum);
    performHideAllTextWindows(machine);
  }
};

struct openBg_0 : public RLOpcode<StrConstant_T, IntConstant_T> {
  openBg_1 delegate_;

  void operator()(RLMachine& machine, string filename, int effectNum) {
    std::vector<int> selEffect = GetSELEffect(machine, effectNum);
    delegate_(machine, filename, effectNum, selEffect[14]);
  }
};

template <typename SPACE>
struct openBg_3 : public RLOpcode<StrConstant_T,
                                 IntConstant_T,
                                 Rect_T<SPACE>,
                                 Point_T,
                                 IntConstant_T> {
  bool use_alpha_;
  explicit openBg_3(bool in) : use_alpha_(in) {}

  void operator()(RLMachine& machine,
                  string fileName,
                  int effectNum,
                  Rect srcRect,
                  Point destPt,
                  int opacity) {
    GraphicsSystem& graphics = machine.system().graphics();
    OpenBgPrelude(machine, fileName);

    // Set the long operation for the correct transition long operation
    std::shared_ptr<Surface> before = graphics.RenderToSurface();

    loadImageToDC1(machine, fileName, srcRect, destPt, opacity, use_alpha_);
    blitDC1toDC0(machine);

    std::shared_ptr<Surface> after = graphics.RenderToSurface();
    performEffect(machine, after, before, effectNum);
    performHideAllTextWindows(machine);
  }
};

template <typename SPACE>
struct openBg_2
    : public RLOpcode<StrConstant_T, IntConstant_T, Rect_T<SPACE>, Point_T> {
  openBg_3<SPACE> delegate_;
  explicit openBg_2(bool in) : delegate_(in) {}

  void operator()(RLMachine& machine,
                  string fileName,
                  int effectNum,
                  Rect srcRect,
                  Point destPt) {
    std::vector<int> selEffect = GetSELEffect(machine, effectNum);
    delegate_(machine, fileName, effectNum, srcRect, destPt, selEffect[14]);
  }
};

template <typename SPACE>
struct openBg_4 : public RLOpcode<StrConstant_T,
                                      Rect_T<SPACE>,
                                      Point_T,
                                      IntConstant_T,
                                      IntConstant_T,
                                      IntConstant_T,
                                      IntConstant_T,
                                      IntConstant_T,
                                      IntConstant_T,
                                      IntConstant_T,
                                      IntConstant_T,
                                      IntConstant_T,
                                      IntConstant_T> {
  bool use_alpha_;
  explicit openBg_4(bool in) : use_alpha_(in) {}

  void operator()(RLMachine& machine,
                  string fileName,
                  Rect srcRect,
                  Point destPt,
                  int time,
                  int style,
                  int direction,
                  int interpolation,
                  int xsize,
                  int ysize,
                  int a,
                  int b,
                  int opacity,
                  int c) {
    GraphicsSystem& graphics = machine.system().graphics();
    OpenBgPrelude(machine, fileName);

    // Set the long operation for the correct transition long operation
    std::shared_ptr<Surface> before = graphics.RenderToSurface();

    loadImageToDC1(machine, fileName, srcRect, destPt, opacity, use_alpha_);
    blitDC1toDC0(machine);

    // Render the screen to a temporary
    std::shared_ptr<Surface> after = graphics.RenderToSurface();
    performEffect(machine,
                  after,
                  before,
                  time,
                  style,
                  direction,
                  interpolation,
                  xsize,
                  ysize,
                  a,
                  b,
                  c);
    performHideAllTextWindows(machine);
  }
};

// -----------------------------------------------------------------------
// {grp,rec}Copy
// -----------------------------------------------------------------------
template <typename SPACE>
struct copy_3 : public RLOpcode<Rect_T<SPACE>,
                               IntConstant_T,
                               Point_T,
                               IntConstant_T,
                               DefaultIntValue_T<255>> {
  bool use_alpha_;
  explicit copy_3(bool in) : use_alpha_(in) {}

  void operator()(RLMachine& machine,
                  Rect srcRect,
                  int src,
                  Point destPoint,
                  int dst,
                  int opacity) {
    // Copying to self is a noop
    if (src == dst)
      return;

    GraphicsSystem& graphics = machine.system().graphics();

    std::shared_ptr<Surface> sourceSurface = graphics.GetDC(src);

    if (dst != 0 && dst != 1) {
      graphics.SetMinimumSizeForDC(dst, srcRect.size());
    }

    sourceSurface->BlitToSurface(*graphics.GetDC(dst),
                                 srcRect,
                                 Rect(destPoint, srcRect.size()),
                                 opacity,
                                 use_alpha_);
  }
};

struct copy_1
    : public RLOpcode<IntConstant_T, IntConstant_T, DefaultIntValue_T<255>> {
  bool use_alpha_;
  explicit copy_1(bool in) : use_alpha_(in) {}

  void operator()(RLMachine& machine, int src, int dst, int opacity) {
    // Copying to self is a noop
    if (src == dst)
      return;

    GraphicsSystem& graphics = machine.system().graphics();

    std::shared_ptr<Surface> sourceSurface = graphics.GetDC(src);

    if (dst != 0 && dst != 1) {
      graphics.SetMinimumSizeForDC(dst, sourceSurface->GetSize());
    }

    sourceSurface->BlitToSurface(*graphics.GetDC(dst),
                                 sourceSurface->GetRect(),
                                 sourceSurface->GetRect(),
                                 opacity,
                                 use_alpha_);
  }
};

// -----------------------------------------------------------------------
// {grp,rec}Fill
// -----------------------------------------------------------------------

struct fill_0 : public RLOpcode<IntConstant_T, RGBColour_T> {
  void operator()(RLMachine& machine, int dc, RGBAColour colour) {
    // Justification: Maiden Halo uses fill(x, 0, 0, 0) as a synanom for clear
    // and since it uses haikei, the DC0 needs to be transparent.
    if (colour.r() == 0 && colour.g() == 0 && colour.b() == 0)
      colour.set_alpha(0);

    machine.system().graphics().GetDC(dc)->Fill(colour);
  }
};

struct fill_1 : public RLOpcode<IntConstant_T, RGBMaybeAColour_T> {
  void operator()(RLMachine& machine, int dc, RGBAColour colour) {
    machine.system().graphics().GetDC(dc)->Fill(colour);
  }
};

template <typename SPACE>
struct fill_3
    : public RLOpcode<Rect_T<SPACE>, IntConstant_T, RGBMaybeAColour_T> {
  void operator()(RLMachine& machine,
                  Rect destRect,
                  int dc,
                  RGBAColour colour) {
    machine.system().graphics().GetDC(dc)->Fill(colour, destRect);
  }
};

struct invert_1 : public RLOpcode<IntConstant_T> {
  void operator()(RLMachine& machine, int dc) {
    std::shared_ptr<Surface> surface = machine.system().graphics().GetDC(dc);
    surface->Invert(surface->GetRect());
  }
};

template <typename SPACE>
struct invert_3 : public RLOpcode<Rect_T<SPACE>, IntConstant_T> {
  void operator()(RLMachine& machine, Rect rect, int dc) {
    machine.system().graphics().GetDC(dc)->Invert(rect);
  }
};

struct mono_1 : public RLOpcode<IntConstant_T> {
  void operator()(RLMachine& machine, int dc) {
    std::shared_ptr<Surface> surface = machine.system().graphics().GetDC(dc);
    surface->Mono(surface->GetRect());
  }
};

template <typename SPACE>
struct mono_3 : public RLOpcode<Rect_T<SPACE>, IntConstant_T> {
  void operator()(RLMachine& machine, Rect rect, int dc) {
    machine.system().graphics().GetDC(dc)->Mono(rect);
  }
};

struct colour_1 : public RLOpcode<IntConstant_T, RGBColour_T> {
  void operator()(RLMachine& machine, int dc, RGBAColour colour) {
    std::shared_ptr<Surface> surface = machine.system().graphics().GetDC(dc);
    surface->ApplyColour(colour.rgb(), surface->GetRect());
  }
};

template <typename SPACE>
struct colour_2
    : public RLOpcode<Rect_T<SPACE>, IntConstant_T, RGBColour_T> {
  void operator()(RLMachine& machine, Rect rect, int dc, RGBAColour colour) {
    std::shared_ptr<Surface> surface = machine.system().graphics().GetDC(dc);
    surface->ApplyColour(colour.rgb(), rect);
  }
};

struct light_1 : public RLOpcode<IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, int dc, int level) {
    std::shared_ptr<Surface> surface = machine.system().graphics().GetDC(dc);
    surface->ApplyColour(RGBColour(level, level, level), surface->GetRect());
  }
};

template <typename SPACE>
struct light_2
    : public RLOpcode<Rect_T<SPACE>, IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, Rect rect, int dc, int level) {
    std::shared_ptr<Surface> surface = machine.system().graphics().GetDC(dc);
    surface->ApplyColour(RGBColour(level, level, level), rect);
  }
};

// -----------------------------------------------------------------------
// {grp,rec}Fade
// -----------------------------------------------------------------------

template <typename SPACE>
struct fade_7
    : public RLOpcode<Rect_T<SPACE>, RGBColour_T, DefaultIntValue_T<0>> {
  void operator()(RLMachine& machine, Rect rect, RGBAColour colour, int time) {
    GraphicsSystem& graphics = machine.system().graphics();
    std::shared_ptr<Surface> before = graphics.RenderToSurface();
    graphics.GetDC(0)->Fill(colour, rect);
    std::shared_ptr<Surface> after = graphics.RenderToSurface();

    if (time > 0) {
      performEffect(machine, after, before, time, 0, 0, 0, 0, 0, 0, 0, 0);
    }
  }
};

template <typename SPACE>
struct fade_5
    : public RLOpcode<Rect_T<SPACE>, IntConstant_T, DefaultIntValue_T<0>> {
  fade_7<SPACE> delegate_;

  void operator()(RLMachine& machine, Rect rect, int colour_num, int time) {
    Gameexe& gexe = machine.system().gameexe();
    const std::vector<int>& rgb = gexe("COLOR_TABLE", colour_num).ToIntVector();
    delegate_(machine, rect, RGBAColour(rgb), time);
  }
};

struct fade_3 : public RLOpcode<RGBColour_T, DefaultIntValue_T<0>> {
  fade_7<rect_impl::REC> delegate_;

  void operator()(RLMachine& machine, RGBAColour colour, int time) {
    Size screen_size = machine.system().graphics().screen_size();
    delegate_(machine, Rect(0, 0, screen_size), colour, time);
  }
};

struct fade_1 : public RLOpcode<IntConstant_T, DefaultIntValue_T<0>> {
  fade_7<rect_impl::REC> delegate_;

  void operator()(RLMachine& machine, int colour_num, int time) {
    Size screen_size = machine.system().graphics().screen_size();
    Gameexe& gexe = machine.system().gameexe();
    const std::vector<int>& rgb = gexe("COLOR_TABLE", colour_num).ToIntVector();
    delegate_(machine, Rect(0, 0, screen_size), RGBAColour(rgb), time);
  }
};

// -----------------------------------------------------------------------
// {grp,rec}StretchBlit
// -----------------------------------------------------------------------
template <typename SPACE>
struct stretchBlit_1 : public RLOpcode<Rect_T<SPACE>,
                                      IntConstant_T,
                                      Rect_T<SPACE>,
                                      IntConstant_T,
                                      DefaultIntValue_T<255>> {
  bool use_alpha_;
  explicit stretchBlit_1(bool in) : use_alpha_(in) {}

  void operator()(RLMachine& machine,
                  Rect src_rect,
                  int src,
                  Rect dst_rect,
                  int dst,
                  int opacity) {
    // Copying to self is a noop
    if (src == dst)
      return;

    GraphicsSystem& graphics = machine.system().graphics();
    std::shared_ptr<Surface> sourceSurface = graphics.GetDC(src);

    if (dst != 0 && dst != 1) {
      graphics.SetMinimumSizeForDC(dst, sourceSurface->GetSize());
    }

    sourceSurface->BlitToSurface(
        *graphics.GetDC(dst), src_rect, dst_rect, opacity, use_alpha_);
  }
};

template <typename SPACE>
struct zoom : public RLOpcode<Rect_T<SPACE>,
                             Rect_T<SPACE>,
                             IntConstant_T,
                             Rect_T<SPACE>,
                             IntConstant_T> {
  void operator()(RLMachine& machine,
                  Rect frect,
                  Rect trect,
                  int srcDC,
                  Rect drect,
                  int time) {
    GraphicsSystem& gs = machine.system().graphics();
    gs.set_graphics_background(BACKGROUND_DC0);

    LongOperation* zoomOp = new ZoomLongOperation(
        machine, gs.GetDC(0), gs.GetDC(srcDC), frect, trect, drect, time);
    BlitAfterEffectFinishes* blitOp = new BlitAfterEffectFinishes(
        zoomOp, gs.GetDC(srcDC), gs.GetDC(0), trect, drect);
    machine.PushLongOperation(blitOp);
  }
};

// -----------------------------------------------------------------------
// {grp,rec}multi
// -----------------------------------------------------------------------

// Defines the fairly complex parameter definition for the list of functions to
// call in a {grp,rec}Multi command.
typedef Argc_T<Special_T<
    DefaultSpecialMapper,
    // 0:copy(strC 'filename')
    StrConstant_T,
    // 1:copy(strC 'filename', 'effect')
    Complex_T<StrConstant_T, IntConstant_T>,
    // 2:copy(strC 'filename', 'effect', 'alpha')
    Complex_T<StrConstant_T, IntConstant_T, IntConstant_T>,
    // 3:area(strC 'filename', 'x1', 'y1', 'x2', 'y2', 'dx', 'dy')
    Complex_T<StrConstant_T,
              IntConstant_T,
              IntConstant_T,
              IntConstant_T,
              IntConstant_T,
              IntConstant_T,
              IntConstant_T>,
    // 4:area(strC 'filename', 'x1', 'y1', 'x2', 'y2', 'dx', 'dy', 'alpha')
    Complex_T<StrConstant_T,
              IntConstant_T,
              IntConstant_T,
              IntConstant_T,
              IntConstant_T,
              IntConstant_T,
              IntConstant_T,
              IntConstant_T>>> MultiCommand;

// -----------------------------------------------------------------------

// Defines the weird multi commands. I will be the first to admit that the
// following is fairly difficult to read; it comes from the quagmire of
// composing Special_T and ComplexX_T templates.
//
// In the end, this operation struct simply Dispatches the Special/Complex
// commands to functions and other operation structs that are clearer in
// purpose.

// All work is applied to DC 1.
const int MULTI_TARGET_DC = 1;

template <typename SPACE>
struct multi_command {
  void handleMultiCommands(RLMachine& machine,
                           const MultiCommand::type& commands);
};

template <typename SPACE>
void multi_command<SPACE>::handleMultiCommands(
    RLMachine& machine,
    const MultiCommand::type& commands) {
  for (MultiCommand::type::const_iterator it = commands.begin();
       it != commands.end();
       it++) {
    switch (it->type) {
      case 0:
        // 0:copy(strC 'filename')
        if (it->first != "")
          load_1(true)(machine, it->first, MULTI_TARGET_DC, 255);
        break;
      case 1: {
        // 1:copy(strC 'filename', 'effect')
        if (get<0>(it->second) != "") {
          Rect src;
          Point dest;
          GetSELPointAndRect(machine, get<1>(it->second), src, dest);

          load_3<SPACE>(true)(
              machine, get<0>(it->second), MULTI_TARGET_DC, src, dest, 255);
        }
        break;
      }
      case 2: {
        // 2:copy(strC 'filename', 'effect', 'alpha')
        if (get<0>(it->third) != "") {
          Rect src;
          Point dest;
          GetSELPointAndRect(machine, get<1>(it->third), src, dest);

          load_3<SPACE>(true)(machine,
                              get<0>(it->third),
                              MULTI_TARGET_DC,
                              src,
                              dest,
                              get<2>(it->third));
        }
        break;
      }
      case 3: {
        // 3:area(strC 'filename', 'x1', 'y1', 'x2', 'y2', 'dx', 'dy')
        if (get<0>(it->fourth) != "") {
          load_3<SPACE>(true)(machine,
                              get<0>(it->fourth),
                              MULTI_TARGET_DC,
                              SPACE::makeRect(get<1>(it->fourth),
                                              get<2>(it->fourth),
                                              get<3>(it->fourth),
                                              get<4>(it->fourth)),
                              Point(get<5>(it->fourth), get<6>(it->fourth)),
                              255);
        }
        break;
      }
      case 4: {
        // 4:area(strC 'filename', 'x1', 'y1', 'x2', 'y2', 'dx', 'dy', 'alpha')
        if (get<0>(it->fifth) != "") {
          load_3<SPACE>(true)(machine,
                              get<0>(it->fifth),
                              MULTI_TARGET_DC,
                              SPACE::makeRect(get<1>(it->fifth),
                                              get<2>(it->fifth),
                                              get<3>(it->fifth),
                                              get<4>(it->fifth)),
                              Point(get<5>(it->fifth), get<6>(it->fifth)),
                              get<7>(it->fifth));
        }
        break;
      }
    }
  }
}

// fun grpMulti <1:Grp:00075, 4> (<strC 'filename', <'effect', MultiCommand)
template <typename SPACE>
struct multi_str_1 : public RLOpcode<StrConstant_T,
                                    IntConstant_T,
                                    IntConstant_T,
                                    MultiCommand>,
                     public multi_command<SPACE> {
  void operator()(RLMachine& machine,
                  string filename,
                  int effect,
                  int alpha,
                  MultiCommand::type commands) {
    load_1(false)(machine, filename, MULTI_TARGET_DC, 255);
    multi_command<SPACE>::handleMultiCommands(machine, commands);
    display_0()(machine, MULTI_TARGET_DC, effect);
  }
};

template <typename SPACE>
struct multi_str_0
    : public RLOpcode<StrConstant_T, IntConstant_T, MultiCommand> {
  multi_str_1<SPACE> delegate_;

  void operator()(RLMachine& machine,
                  string filename,
                  int effect,
                  MultiCommand::type commands) {
    delegate_(machine, filename, effect, 255, commands);
  }
};

template <typename SPACE>
struct multi_dc_1 : public RLOpcode<IntConstant_T,
                                   IntConstant_T,
                                   IntConstant_T,
                                   MultiCommand>,
                    public multi_command<SPACE> {
  void operator()(RLMachine& machine,
                  int dc,
                  int effect,
                  int alpha,
                  MultiCommand::type commands) {
    copy_1(false)(machine, dc, MULTI_TARGET_DC, 255);
    multi_command<SPACE>::handleMultiCommands(machine, commands);
    display_0()(machine, MULTI_TARGET_DC, effect);
  }
};

template <typename SPACE>
struct multi_dc_0
    : public RLOpcode<IntConstant_T, IntConstant_T, MultiCommand> {
  multi_dc_1<SPACE> delegate_;

  void operator()(RLMachine& machine,
                  int dc,
                  int effect,
                  MultiCommand::type commands) {
    delegate_(machine, dc, effect, 255, commands);
  }
};

// Special case adapter to record every graphics command onto the "graphics
// stack"
class GrpStackAdapter : public RLOp_SpecialCase {
 public:
  explicit GrpStackAdapter(RLOperation* in) : operation(in) {}

  void operator()(RLMachine& machine, const libreallive::CommandElement& ff) {
    operation->DispatchFunction(machine, ff);

    // Record this command's reallive bytecode form onto the graphics stack.
    machine.system().graphics().AddGraphicsStackCommand(
        ff.GetSerializedCommand(machine));
  }

 private:
  std::unique_ptr<RLOperation> operation;
};

}  // namespace

RLOperation* GraphicsStackMappingFun(RLOperation* op) {
  return new GrpStackAdapter(op);
}

GrpModule::GrpModule() : MappedRLModule(GraphicsStackMappingFun, "Grp", 1, 33) {
  using rect_impl::GRP;
  using rect_impl::REC;

  AddOpcode(15, 0, "allocDC", new allocDC);
  AddOpcode(16, 0, "FreeDC", CallFunction(&GraphicsSystem::FreeDC));

  AddUnsupportedOpcode(20, 0, "grpLoadMask");
  // AddOpcode(30, 0, new grpTextout);

  AddOpcode(31, 0, "wipe", new wipe);
  AddOpcode(32, 0, "shake", new shake);

  AddOpcode(50, 0, "grpLoad", new load_1(false));
  AddOpcode(50, 1, "grpLoad", new load_1(false));
  AddOpcode(50, 2, "grpLoad", new load_3<GRP>(false));
  AddOpcode(50, 3, "grpLoad", new load_3<GRP>(false));
  AddOpcode(51, 0, "grpMaskLoad", new load_1(true));
  AddOpcode(51, 1, "grpMaskLoad", new load_1(true));
  AddOpcode(51, 2, "grpMaskLoad", new load_3<GRP>(true));
  AddOpcode(51, 3, "grpMaskLoad", new load_3<GRP>(true));

  // These are grpBuffer, which is very similar to grpLoad and Haeleth
  // doesn't know how they differ. For now, we just assume they're
  // equivalent.
  AddOpcode(70, 0, "grpBuffer", new load_1(false));
  AddOpcode(70, 1, "grpBuffer", new load_1(false));
  AddOpcode(70, 2, "grpBuffer", new load_3<GRP>(false));
  AddOpcode(70, 3, "grpBuffer", new load_3<GRP>(false));
  AddOpcode(71, 0, "grpMaskBuffer", new load_1(true));
  AddOpcode(71, 1, "grpMaskBuffer", new load_1(true));
  AddOpcode(71, 2, "grpMaskBuffer", new load_3<GRP>(true));
  AddOpcode(71, 3, "grpMaskBuffer", new load_3<GRP>(true));

  AddOpcode(72, 0, "grpDisplay", new display_0);
  AddOpcode(72, 1, "grpDisplay", new display_1);
  AddOpcode(72, 2, "grpDisplay", new display_2<GRP>());
  AddOpcode(72, 3, "grpDisplay", new display_3<GRP>());
  AddOpcode(72, 4, "grpDisplay", new display_4<GRP>());

  AddOpcode(73, 0, "grpOpenBg", new openBg_0);
  AddOpcode(73, 1, "grpOpenBg", new openBg_1);
  AddOpcode(73, 2, "grpOpenBg", new openBg_2<GRP>(false));
  AddOpcode(73, 3, "grpOpenBg", new openBg_3<GRP>(false));
  AddOpcode(73, 4, "grpOpenBg", new openBg_4<GRP>(false));

  AddOpcode(74, 0, "grpMaskOpen", new open_0(true));
  AddOpcode(74, 1, "grpMaskOpen", new open_1(true));
  AddOpcode(74, 2, "grpMaskOpen", new open_2<GRP>(true));
  AddOpcode(74, 3, "grpMaskOpen", new open_3<GRP>(true));
  AddOpcode(74, 4, "grpMaskOpen", new open_4<GRP>(true));

  AddOpcode(75, 0, "grpMulti", new multi_str_0<GRP>());
  AddOpcode(75, 1, "grpMulti", new multi_str_1<GRP>());
  AddUnsupportedOpcode(75, 2, "grpMulti");
  AddUnsupportedOpcode(75, 3, "grpMulti");
  AddUnsupportedOpcode(75, 4, "grpMulti");

  AddOpcode(76, 0, "grpOpen", new open_0(false));
  AddOpcode(76, 1, "grpOpen", new open_1(false));
  AddOpcode(76, 2, "grpOpen", new open_2<GRP>(false));
  AddOpcode(76, 3, "grpOpen", new open_3<GRP>(false));
  AddOpcode(76, 4, "grpOpen", new open_4<GRP>(false));

  AddOpcode(77, 0, "grpMulti", new multi_dc_0<GRP>());
  AddOpcode(77, 1, "grpMulti", new multi_dc_1<GRP>());
  AddUnsupportedOpcode(77, 2, "grpMulti");
  AddUnsupportedOpcode(77, 3, "grpMulti");
  AddUnsupportedOpcode(77, 4, "grpMulti");

  AddOpcode(100, 0, "grpCopy", new copy_1(false));
  AddOpcode(100, 1, "grpCopy", new copy_1(false));
  AddOpcode(100, 2, "grpCopy", new copy_3<GRP>(false));
  AddOpcode(100, 3, "grpCopy", new copy_3<GRP>(false));
  AddOpcode(101, 0, "grpMaskCopy", new copy_1(true));
  AddOpcode(101, 1, "grpMaskCopy", new copy_1(true));
  AddOpcode(101, 2, "grpMaskCopy", new copy_3<GRP>(true));
  AddOpcode(101, 3, "grpMaskCopy", new copy_3<GRP>(true));

  AddUnsupportedOpcode(120, 5, "grpCopyWithMask");
  AddUnsupportedOpcode(140, 5, "grpCopyInvMask");

  AddOpcode(201, 0, "grpFill", new fill_0);
  AddOpcode(201, 1, "grpFill", new fill_1);
  AddOpcode(201, 2, "grpFill", new fill_3<GRP>());
  AddOpcode(201, 3, "grpFill", new fill_3<GRP>());

  AddOpcode(300, 0, "grpInvert", new invert_1);
  AddUnsupportedOpcode(300, 1, "grpInvert");
  AddOpcode(300, 2, "grpInvert", new invert_3<GRP>());
  AddUnsupportedOpcode(300, 3, "grpInvert");

  AddOpcode(301, 0, "grpMono", new mono_1);
  AddUnsupportedOpcode(301, 1, "grpMono");
  AddOpcode(301, 2, "grpMono", new mono_3<GRP>());
  AddUnsupportedOpcode(301, 3, "grpMono");

  AddOpcode(302, 0, "grpColour", new colour_1);
  AddOpcode(302, 1, "grpColour", new colour_2<GRP>());

  AddOpcode(303, 0, "grpLight", new light_1);
  AddOpcode(303, 1, "grpLight", new light_2<GRP>());

  AddUnsupportedOpcode(400, 0, "grpSwap");
  AddUnsupportedOpcode(400, 1, "grpSwap");

  AddOpcode(401, 0, "grpStretchBlt", new stretchBlit_1<GRP>(false));
  AddOpcode(401, 1, "grpStretchBlt", new stretchBlit_1<GRP>(false));

  AddOpcode(402, 0, "grpZoom", new zoom<GRP>());

  AddOpcode(403, 0, "grpFade", new fade_1);
  AddOpcode(403, 1, "grpFade", new fade_1);
  AddOpcode(403, 2, "grpFade", new fade_3);
  AddOpcode(403, 3, "grpFade", new fade_3);
  AddOpcode(403, 4, "grpFade", new fade_5<GRP>());
  AddOpcode(403, 5, "grpFade", new fade_5<GRP>());
  AddOpcode(403, 6, "grpFade", new fade_7<GRP>());
  AddOpcode(403, 7, "grpFade", new fade_7<GRP>());

  AddOpcode(409, 0, "grpMaskStretchBlt", new stretchBlit_1<GRP>(true));
  AddOpcode(409, 1, "grpMaskStretchBlt", new stretchBlit_1<GRP>(true));

  AddUnsupportedOpcode(601, 0, "grpMaskAdd");
  AddUnsupportedOpcode(601, 1, "grpMaskAdd");
  AddUnsupportedOpcode(601, 2, "grpMaskAdd");
  AddUnsupportedOpcode(601, 3, "grpMaskAdd");

  // -----------------------------------------------------------------------

  AddOpcode(1050, 0, "recLoad", new load_1(false));
  AddOpcode(1050, 1, "recLoad", new load_1(false));
  AddOpcode(1050, 2, "recLoad", new load_3<REC>(false));
  AddOpcode(1050, 3, "recLoad", new load_3<REC>(false));

  AddOpcode(1051, 0, "recMaskLoad", new load_1(true));
  AddOpcode(1051, 1, "recMaskLoad", new load_1(true));
  AddOpcode(1051, 2, "recMaskLoad", new load_3<REC>(true));
  AddOpcode(1051, 3, "recMaskLoad", new load_3<REC>(true));

  AddOpcode(1052, 0, "recDisplay", new display_0);
  AddOpcode(1052, 1, "recDisplay", new display_1);
  AddOpcode(1052, 2, "recDisplay", new display_2<REC>());
  AddOpcode(1052, 3, "recDisplay", new display_3<REC>());
  AddOpcode(1052, 4, "recDisplay", new display_4<REC>());

  AddOpcode(1053, 0, "recOpenBg", new openBg_0);
  AddOpcode(1053, 1, "recOpenBg", new openBg_1);
  AddOpcode(1053, 2, "recOpenBg", new openBg_2<REC>(false));
  AddOpcode(1053, 3, "recOpenBg", new openBg_3<REC>(false));
  AddOpcode(1053, 4, "recOpenBg", new openBg_4<REC>(false));

  AddOpcode(1054, 0, "recMaskOpen", new open_0(true));
  AddOpcode(1054, 1, "recMaskOpen", new open_1(true));
  AddOpcode(1054, 2, "recMaskOpen", new open_2<REC>(true));
  AddOpcode(1054, 3, "recMaskOpen", new open_3<REC>(true));
  AddOpcode(1054, 4, "recMaskOpen", new open_4<REC>(true));

  AddOpcode(1056, 0, "recOpen", new open_0(false));
  AddOpcode(1056, 1, "recOpen", new open_1(false));
  AddOpcode(1056, 2, "recOpen", new open_2<REC>(false));
  AddOpcode(1056, 3, "recOpen", new open_3<REC>(false));
  AddOpcode(1056, 4, "recOpen", new open_4<REC>(false));

  AddOpcode(1055, 0, "recMulti", new multi_str_0<REC>());
  AddOpcode(1055, 1, "recMulti", new multi_str_1<REC>());
  AddUnsupportedOpcode(1055, 2, "recMulti");
  AddUnsupportedOpcode(1055, 3, "recMulti");
  AddUnsupportedOpcode(1055, 4, "recMulti");

  AddOpcode(1057, 0, "recMulti", new multi_dc_0<REC>());
  AddOpcode(1057, 1, "recMulti", new multi_dc_1<REC>());
  AddUnsupportedOpcode(1057, 2, "recMulti");
  AddUnsupportedOpcode(1057, 3, "recMulti");
  AddUnsupportedOpcode(1057, 4, "recMulti");

  AddOpcode(1100, 0, "recCopy", new copy_1(false));
  AddOpcode(1100, 1, "recCopy", new copy_1(false));
  AddOpcode(1100, 2, "recCopy", new copy_3<REC>(false));
  AddOpcode(1100, 3, "recCopy", new copy_3<REC>(false));
  AddOpcode(1101, 0, "recMaskCopy", new copy_1(true));
  AddOpcode(1101, 1, "recMaskCopy", new copy_1(true));
  AddOpcode(1101, 2, "recMaskCopy", new copy_3<REC>(true));
  AddOpcode(1101, 3, "recMaskCopy", new copy_3<REC>(true));

  AddOpcode(1201, 0, "recFill", new fill_0);
  AddOpcode(1201, 1, "recFill", new fill_1);
  AddOpcode(1201, 2, "recFill", new fill_3<REC>());
  AddOpcode(1201, 3, "recFill", new fill_3<REC>());

  AddOpcode(1300, 0, "recInvert", new invert_1);
  AddUnsupportedOpcode(1300, 1, "recInvert");
  AddOpcode(1300, 2, "recInvert", new invert_3<REC>());
  AddUnsupportedOpcode(1300, 3, "recInvert");

  AddOpcode(1301, 0, "recMono", new mono_1);
  AddUnsupportedOpcode(1301, 1, "recMono");
  AddOpcode(1301, 2, "recMono", new mono_3<REC>());
  AddUnsupportedOpcode(1301, 3, "recMono");

  AddOpcode(1302, 0, "recColour", new colour_1);
  AddOpcode(1302, 1, "recColour", new colour_2<REC>());

  AddOpcode(1303, 0, "recLight", new light_1);
  AddOpcode(1303, 1, "recLight", new light_2<REC>());

  AddUnsupportedOpcode(1400, 0, "recSwap");
  AddUnsupportedOpcode(1400, 1, "recSwap");

  AddOpcode(1401, 0, "recStretchBlt", new stretchBlit_1<REC>(false));
  AddOpcode(1401, 1, "recStretchBlt", new stretchBlit_1<REC>(false));

  AddOpcode(1402, 0, "recZoom", new zoom<REC>());

  AddOpcode(1403, 0, "recFade", new fade_1);
  AddOpcode(1403, 1, "recFade", new fade_1);
  AddOpcode(1403, 2, "recFade", new fade_3);
  AddOpcode(1403, 3, "recFade", new fade_3);
  AddOpcode(1403, 4, "recFade", new fade_5<REC>());
  AddOpcode(1403, 5, "recFade", new fade_5<REC>());
  AddOpcode(1403, 6, "recFade", new fade_7<REC>());
  AddOpcode(1403, 7, "recFade", new fade_7<REC>());

  AddUnsupportedOpcode(1404, 0, "recFlash");
  AddUnsupportedOpcode(1404, 1, "recFlash");
  AddUnsupportedOpcode(1404, 2, "recFlash");
  AddUnsupportedOpcode(1404, 3, "recFlash");

  AddUnsupportedOpcode(1406, 0, "recPan");
  AddUnsupportedOpcode(1407, 0, "recShift");
  AddUnsupportedOpcode(1408, 0, "recSlide");
  AddOpcode(1409, 0, "recMaskStretchBlt", new stretchBlit_1<REC>(true));
  AddOpcode(1409, 1, "recMaskStretchBlt", new stretchBlit_1<REC>(true));
}

// @}

// -----------------------------------------------------------------------

void ReplayGraphicsStackCommand(RLMachine& machine,
                                const std::deque<std::string>& stack) {
  try {
    for (auto const& command : stack) {
      if (command != "") {
        // Parse the string as a chunk of Reallive bytecode.
        libreallive::ConstructionData cdata(0, libreallive::pointer_t());
        libreallive::BytecodeElement* element =
            libreallive::BytecodeElement::Read(
                command.c_str(), command.c_str() + command.size(), cdata);
        libreallive::CommandElement* command =
            dynamic_cast<libreallive::CommandElement*>(element);
        if (command) {
          machine.ExecuteCommand(*command);
        }
      }
    }
  }
  catch (std::exception& e) {
    std::cerr << "Error while replaying graphics stack: " << e.what()
              << std::endl;
    return;
  }
}

// -----------------------------------------------------------------------

void ReplayDepricatedGraphicsStackVector(
    RLMachine& machine,
    const std::vector<GraphicsStackFrame>& gstack) {
  for (auto const& frame : gstack) {
    try {
      if (frame.name() == GRP_LOAD) {
        if (frame.hasTargetCoordinates()) {
          load_3<rect_impl::REC>(frame.mask())(machine,
                                               frame.filename(),
                                               frame.targetDC(),
                                               frame.sourceRect(),
                                               frame.targetPoint(),
                                               frame.opacity());
        } else {
          // Older versions of rlvm didn't record the mask bit, so make sure we
          // check for that since we don't want to break old save games.
          bool mask = (frame.hasMask() ? frame.mask() : true);
          load_1 loader(mask);
          loader(machine, frame.filename(), frame.targetDC(), frame.opacity());
        }
      } else if (frame.name() == GRP_OPEN) {
        // open is just a load + an animation.
        loadImageToDC1(machine,
                       frame.filename(),
                       frame.sourceRect(),
                       frame.targetPoint(),
                       frame.opacity(),
                       frame.mask());
        blitDC1toDC0(machine);
      } else if (frame.name() == GRP_COPY) {
        if (frame.hasSourceCoordinates()) {
          copy_3<rect_impl::REC>(frame.mask())(machine,
                                               frame.sourceRect(),
                                               frame.sourceDC(),
                                               frame.targetPoint(),
                                               frame.targetDC(),
                                               frame.opacity());
        } else {
          copy_1(frame.mask())(
              machine, frame.sourceDC(), frame.targetDC(), frame.opacity());
        }
      } else if (frame.name() == GRP_DISPLAY) {
        loadDCToDC1(machine,
                    frame.sourceDC(),
                    frame.sourceRect(),
                    frame.targetPoint(),
                    frame.opacity());
        blitDC1toDC0(machine);
      } else if (frame.name() == GRP_OPENBG) {
        loadImageToDC1(machine,
                       frame.filename(),
                       frame.sourceRect(),
                       frame.targetPoint(),
                       frame.opacity(),
                       false);
        blitDC1toDC0(machine);
      } else if (frame.name() == GRP_ALLOC) {
        Point target = frame.targetPoint();
        allocDC()(machine, frame.targetDC(), target.x(), target.y());
      } else if (frame.name() == GRP_WIPE) {
        wipe()(machine, frame.targetDC(), frame.r(), frame.g(), frame.b());
      }
    }
    catch (rlvm::Exception& e) {
      std::cerr << "WARNING: Error while thawing graphics stack: "
                << e.what() << std::endl;
    }
  }
}
