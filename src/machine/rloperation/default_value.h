// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2007 Elliot Glaysher
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

#ifndef SRC_MACHINE_RLOPERATION_DEFAULT_VALUE_H_
#define SRC_MACHINE_RLOPERATION_DEFAULT_VALUE_H_

#include <string>
#include <vector>

#include "libreallive/expression.h"

template <int DEFAULTVAL>
struct DefaultIntValue_T {
  // The output type of this type struct
  typedef int type;

  // Convert the incoming parameter objects into the resulting type
  static type getData(RLMachine& machine,
                      const libreallive::ExpressionPiecesVector& p,
                      unsigned int& position) {
    if (position < p.size()) {
      return IntConstant_T::getData(machine, p, position);
    } else {
      return DEFAULTVAL;
    }
  }

  static void ParseParameters(unsigned int& position,
                              const std::vector<std::string>& input,
                              libreallive::ExpressionPiecesVector& output) {
    if (position < input.size()) {
      IntConstant_T::ParseParameters(position, input, output);
    } else {
      output.emplace_back(
          libreallive::ExpressionPiece::IntConstant(DEFAULTVAL));
      position++;
    }
  }

  enum { is_complex = false };
};

// Typestruct that will return an empty string if there isn't a value.
struct DefaultStrValue_T {
  // The output type of this type struct
  typedef std::string type;

  // Convert the incoming parameter objects into the resulting type
  static type getData(RLMachine& machine,
                      const libreallive::ExpressionPiecesVector& p,
                      unsigned int& position) {
    if (position < p.size()) {
      return StrConstant_T::getData(machine, p, position);
    } else {
      return std::string();
    }
  }

  static void ParseParameters(unsigned int& position,
                              const std::vector<std::string>& input,
                              libreallive::ExpressionPiecesVector& output) {
    if (position < input.size()) {
      StrConstant_T::ParseParameters(position, input, output);
    } else {
      output.emplace_back(
          libreallive::ExpressionPiece::StrConstant(std::string()));
      position++;
    }
  }

  enum { is_complex = false };
};

#endif  // SRC_MACHINE_RLOPERATION_DEFAULT_VALUE_H_
