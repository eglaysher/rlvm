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

#ifndef SRC_EFFECTS_WIPE_EFFECT_H_
#define SRC_EFFECTS_WIPE_EFFECT_H_

#include <memory>

#include "effects/effect.h"

class GraphicsSystem;

// Base class for the four classess that implement \#SEL transition
// style #10, Wipe. There are four direct subclasses from WipeEffect
// that implement the individual directions that we wipe in.
class WipeEffect : public Effect {
 public:
  WipeEffect(RLMachine& machine,
             std::shared_ptr<Surface> src,
             std::shared_ptr<Surface> dst,
             const Size& screen_size,
             int time,
             int interpolation);
  virtual ~WipeEffect();

 protected:
  void CalculateSizes(int currentTime,
                      int& sizeOfInterpolation,
                      int& sizeOfMainPolygon,
                      int sizeOfScreen);

 private:
  virtual bool BlitOriginalImage() const final;

  int interpolation_;
  int interpolation_in_pixels_;
};

// Implements SEL #10, Wipe, with direction 0, top to bottom.
class WipeTopToBottomEffect : public WipeEffect {
 public:
  WipeTopToBottomEffect(RLMachine& machine,
                        std::shared_ptr<Surface> src,
                        std::shared_ptr<Surface> dst,
                        const Size& screen_size,
                        int time,
                        int interpolation);
  virtual ~WipeTopToBottomEffect();

 protected:
  virtual void PerformEffectForTime(RLMachine& machine,
                                    int currentTime) final;
};

// Implements SEL #10, Wipe, with direction 1, bottom to top.
class WipeBottomToTopEffect : public WipeEffect {
 public:
  WipeBottomToTopEffect(RLMachine& machine,
                        std::shared_ptr<Surface> src,
                        std::shared_ptr<Surface> dst,
                        const Size& screen_size,
                        int time,
                        int interpolation);
  virtual ~WipeBottomToTopEffect();

 protected:
  virtual void PerformEffectForTime(RLMachine& machine,
                                    int currentTime) final;
};

// Implements SEL #10, Wipe, with direction 2, left to right.
class WipeLeftToRightEffect : public WipeEffect {
 public:
  WipeLeftToRightEffect(RLMachine& machine,
                        std::shared_ptr<Surface> src,
                        std::shared_ptr<Surface> dst,
                        const Size& screen_size,
                        int time,
                        int interpolation);
  virtual ~WipeLeftToRightEffect();

 protected:
  virtual void PerformEffectForTime(RLMachine& machine,
                                    int currentTime) final;
};

// Implements SEL #10, Wipe, with direction 3, right to left.
class WipeRightToLeftEffect : public WipeEffect {
 public:
  WipeRightToLeftEffect(RLMachine& machine,
                        std::shared_ptr<Surface> src,
                        std::shared_ptr<Surface> dst,
                        const Size& screen_size,
                        int time,
                        int interpolation);
  virtual ~WipeRightToLeftEffect();

 protected:
  virtual void PerformEffectForTime(RLMachine& machine,
                                    int currentTime) final;
};

#endif  // SRC_EFFECTS_WIPE_EFFECT_H_
