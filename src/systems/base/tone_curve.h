// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2008 Elliot Glaysher
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

#ifndef SRC_SYSTEMS_BASE_TONE_CURVE_H_
#define SRC_SYSTEMS_BASE_TONE_CURVE_H_

#include <array>
#include <vector>

class Gameexe;
class RLMachine;

typedef std::array<unsigned char, 256> ToneCurveColorMap;
typedef std::array<ToneCurveColorMap, 3> ToneCurveRGBMap;
typedef std::vector<ToneCurveRGBMap> ToneCurveEffects;

// Manages tone curve effects
//
// The tcc file is a set of mapping between R, G, and B color values and their
// corresponding values after the tone curve is applied, where
// tcc_effect[2][1][200]
// is the corresponding green value for a green value of 200 in the original
// image
// when the tone curve with the "index" of 2 is applied.
// ToneCurve class is responsible for loading the tcc data and providing an
// interface for applying tone curve effects.
class ToneCurve {
 public:
  // Initializes an empty tone curve set (for games that don't use this
  // feature).
  ToneCurve();

  // Initializes the CG table with the TCC data file specified in the
  // #TONECURVE_FILENAME gameexe key.
  explicit ToneCurve(Gameexe& gameexe);
  ~ToneCurve();

  // Returns the total number of tone curve effects available in the tone curve
  // file
  int GetEffectCount() const;

  // Return the effect at the given index (used by Surface in tone_curve()).  The
  // effects are indexed from 0 to effect_count - 1
  ToneCurveRGBMap GetEffect(int index);

 private:
  // Array of tone curve effects
  ToneCurveEffects tcc_info_;
  int effect_count_;
};  // end of class ToneCurve

#endif  // SRC_SYSTEMS_BASE_TONE_CURVE_H_
