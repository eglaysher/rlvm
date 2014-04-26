// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2009 Elliot Glaysher
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
// -----------------------------------------------------------------------

#ifndef SRC_MACHINE_RLOPERATION_RECT_T_H_
#define SRC_MACHINE_RLOPERATION_RECT_T_H_

#include <string>
#include <vector>

#include "machine/rloperation.h"
#include "systems/base/rect.h"

struct Point_T {
  typedef Point type;

  static type getData(RLMachine& machine,
                      const libreallive::ExpressionPiecesVector& p,
                      unsigned int& position) {
    int x = IntConstant_T::getData(machine, p, position);
    int y = IntConstant_T::getData(machine, p, position);
    return Point(x, y);
  }

  static void ParseParameters(unsigned int& position,
                              const std::vector<std::string>& input,
                              libreallive::ExpressionPiecesVector& output) {
    IntConstant_T::ParseParameters(position, input, output);
    IntConstant_T::ParseParameters(position, input, output);
  }

  enum { is_complex = false };
};

// Tags used to specify how we should construct a Rect.
namespace rect_impl {
struct GRP {
  static Rect makeRect(int one, int two, int three, int four) {
    return Rect::GRP(one, two, three, four);
  }
};

struct REC {
  static Rect makeRect(int one, int two, int three, int four) {
    return Rect::REC(one, two, three, four);
  }
};

}  // namespace rect_impl

template <typename T>
struct Rect_T {
  typedef Rect type;

  // Convert the incoming parameter objects into the resulting type.
  static type getData(RLMachine& machine,
                      const libreallive::ExpressionPiecesVector& p,
                      unsigned int& position) {
    int one = IntConstant_T::getData(machine, p, position);
    int two = IntConstant_T::getData(machine, p, position);
    int three = IntConstant_T::getData(machine, p, position);
    int four = IntConstant_T::getData(machine, p, position);
    return T::makeRect(one, two, three, four);
  }

  // Parse the raw parameter string and put the results in ExpressionPiece
  static void ParseParameters(unsigned int& position,
                              const std::vector<std::string>& input,
                              libreallive::ExpressionPiecesVector& output) {
    IntConstant_T::ParseParameters(position, input, output);
    IntConstant_T::ParseParameters(position, input, output);
    IntConstant_T::ParseParameters(position, input, output);
    IntConstant_T::ParseParameters(position, input, output);
  }

  enum { is_complex = false };
};

#endif  // SRC_MACHINE_RLOPERATION_RECT_T_H_
