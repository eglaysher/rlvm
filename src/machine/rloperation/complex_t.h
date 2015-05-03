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

#ifndef SRC_MACHINE_RLOPERATION_COMPLEX_T_H_
#define SRC_MACHINE_RLOPERATION_COMPLEX_T_H_

#include <string>
#include <tuple>
#include <vector>

#include "libreallive/bytecode_fwd.h"
#include "libreallive/expression.h"
#include "machine/rloperation.h"
#include "machine/rloperation/references.h"

// Type definition that implements the complex parameter concept.
//
// It really should have been called tuple, but the name's stuck
// now. Takes (n) type structs as template parameters.
template <typename... Args>
struct Complex_T {
  // The output type of this type struct
  typedef std::tuple<typename Args::type...> type;

  // Convert the incoming parameter objects into the resulting type.
  static type getData(RLMachine& machine,
                      const libreallive::ExpressionPiecesVector& p,
                      unsigned int& position);

  static void ParseParameters(unsigned int& position,
                              const std::vector<std::string>& input,
                              libreallive::ExpressionPiecesVector& output);

  enum { is_complex = true };
};

// static
template <typename... Args>
typename Complex_T<Args...>::type
Complex_T<Args...>::getData(RLMachine& machine,
                            const libreallive::ExpressionPiecesVector& p,
                            unsigned int& position) {
  unsigned int pos_in_expression = 0;
  const libreallive::ExpressionPiecesVector& pieces =
      p[position++].GetContainedPieces();
  return type { Args::getData(machine, pieces, pos_in_expression)... };
}

// static
template <typename... Args>
void Complex_T<Args...>::ParseParameters(
    unsigned int& position,
    const std::vector<std::string>& input,
    libreallive::ExpressionPiecesVector& output) {
  const char* data = input.at(position).c_str();
  libreallive::ExpressionPiece ep(libreallive::GetComplexParam(data));
  output.push_back(std::move(ep));
  position++;
}

extern template struct Complex_T<IntConstant_T, IntConstant_T>;
extern template struct Complex_T<IntConstant_T, IntReference_T>;
extern template struct Complex_T<IntReference_T, IntReference_T>;
extern template struct Complex_T<StrConstant_T, IntConstant_T, IntConstant_T>;
extern template struct Complex_T<StrConstant_T, IntConstant_T>;

#endif  // SRC_MACHINE_RLOPERATION_COMPLEX_T_H_
