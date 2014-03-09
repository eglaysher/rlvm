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

#ifndef SRC_EFFECTS_BLIND_EFFECT_H_
#define SRC_EFFECTS_BLIND_EFFECT_H_

#include "effects/effect.h"

// Base class for implementing \#SEL transition style \#10, Blind.
class BlindEffect : public Effect {
 public:
  BlindEffect(RLMachine& machine,
              boost::shared_ptr<Surface> src,
              boost::shared_ptr<Surface> dst,
              const Size& screenSize,
              int time,
              int blindSize);

  ~BlindEffect();

 protected:
  const int blindSize() const { return blind_size_; }

  virtual void computeGrowing(RLMachine& machine, int maxSize, int currentTime);
  virtual void computeDecreasing(RLMachine& machine,
                                 int maxSize,
                                 int currentTime);

  virtual void renderPolygon(int polyStart, int polyEnd) = 0;

 private:
  virtual bool blitOriginalImage() const;

  int blind_size_;
};

class BlindTopToBottomEffect : public BlindEffect {
 public:
  BlindTopToBottomEffect(RLMachine& machine,
                         boost::shared_ptr<Surface> src,
                         boost::shared_ptr<Surface> dst,
                         const Size& screenSize,
                         int time,
                         int blindSize);

 protected:
  virtual void performEffectForTime(RLMachine& machine, int currentTime);
  virtual void renderPolygon(int polyStart, int polyEnd);
};

class BlindBottomToTopEffect : public BlindEffect {
 public:
  BlindBottomToTopEffect(RLMachine& machine,
                         boost::shared_ptr<Surface> src,
                         boost::shared_ptr<Surface> dst,
                         const Size& screenSize,
                         int time,
                         int blindSize);

 protected:
  virtual void performEffectForTime(RLMachine& machine, int currentTime);
  virtual void renderPolygon(int polyStart, int polyEnd);
};

class BlindLeftToRightEffect : public BlindEffect {
 public:
  BlindLeftToRightEffect(RLMachine& machine,
                         boost::shared_ptr<Surface> src,
                         boost::shared_ptr<Surface> dst,
                         const Size& screenSize,
                         int time,
                         int blindSize);

 protected:
  virtual void performEffectForTime(RLMachine& machine, int currentTime);
  virtual void renderPolygon(int polyStart, int polyEnd);
};

class BlindRightToLeftEffect : public BlindEffect {
 public:
  BlindRightToLeftEffect(RLMachine& machine,
                         boost::shared_ptr<Surface> src,
                         boost::shared_ptr<Surface> dst,
                         const Size& screenSize,
                         int time,
                         int blindSize);

 protected:
  virtual void performEffectForTime(RLMachine& machine, int currentTime);
  virtual void renderPolygon(int polyStart, int polyEnd);
};

#endif  // SRC_EFFECTS_BLIND_EFFECT_H_
