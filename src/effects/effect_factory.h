// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006 Elliot Glaysher
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

#ifndef SRC_EFFECTS_EFFECT_FACTORY_H_
#define SRC_EFFECTS_EFFECT_FACTORY_H_

#include <memory>

class RLMachine;

class Size;
class Surface;
class Effect;
class ScrollSquashSlideDrawer;
class ScrollSquashSlideEffectTypeBase;

// Factory that creates all Effects. This factory is called with
// either a Gameexe and the \#SEL or \#SELR number, or it is passed the
// equivalent parameters.
class EffectFactory {
 public:
  // Builds an Effect based off the \#SEL.selnum line in the
  // Gameexe.ini file. The coordinates, which are in grp* format (x1,
  // y1, x2, y2), are converted to rec* format and then based to the
  // build() method.
  static Effect* BuildFromSEL(RLMachine& machine,
                              std::shared_ptr<Surface> src,
                              std::shared_ptr<Surface> dst,
                              int selnum);

  // Returns a constructed LongOperation with the following properties
  // to perform a transition. Note: Effect's external interface work on
  // the rec* coordinate system (x, y, width, height) instead of the
  // grp* coordinate system (x1, y1, x2, y2) .
  static Effect* Build(RLMachine& machine,
                       std::shared_ptr<Surface> src,
                       std::shared_ptr<Surface> dst,
                       int time,
                       int style,
                       int direction,
                       int interpolation,
                       int xsize,
                       int ysize,
                       int a,
                       int b,
                       int c);

 private:
  // Creates a specific subclass of WipeEffect for \#SEL #10, Wipe.
  static Effect* BuildWipeEffect(RLMachine& machine,
                                 std::shared_ptr<Surface> src,
                                 std::shared_ptr<Surface> dst,
                                 const Size& screen_size,
                                 int time,
                                 int direction,
                                 int interpolation);

  // Creates a specific subclass of BlindEffect for \#SEL #120, Blind.
  static Effect* BuildBlindEffect(RLMachine& machine,
                                  std::shared_ptr<Surface> src,
                                  std::shared_ptr<Surface> dst,
                                  const Size& screen_size,
                                  int time,
                                  int direction,
                                  int xsize,
                                  int ysize);

  static ScrollSquashSlideDrawer* BuildScrollSquashSlideDrawer(int drawerType);

  static ScrollSquashSlideEffectTypeBase* BuildScrollSquashSlideTypeBase(
      int style);
};

#endif  // SRC_EFFECTS_EFFECT_FACTORY_H_
