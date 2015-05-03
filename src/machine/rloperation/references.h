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

#ifndef SRC_MACHINE_RLOPERATION_REFERENCES_H_
#define SRC_MACHINE_RLOPERATION_REFERENCES_H_

#include <string>
#include <vector>

#include "machine/reference.h"
#include "libreallive/expression.h"

// Type definition for a reference into the RLMachine's memory,
// referencing an integer value.
//
// This struct is used to define the parameter types of a RLOperation
// subclass, and should not be used directly. It should only be used
// as a template parameter to one of those classes, or of another type
// definition struct.
struct IntReference_T {
  // We pass iterators to the
  typedef IntReferenceIterator type;

  // Convert the incoming parameter objects into the resulting type
  static type getData(RLMachine& machine,
                      const libreallive::ExpressionPiecesVector& p,
                      unsigned int& position);

  // Parse the raw parameter string and put the results in ExpressionPiece
  static void ParseParameters(unsigned int& position,
                              const std::vector<std::string>& input,
                              libreallive::ExpressionPiecesVector& output);

  enum { is_complex = false };
};

// Type struct for a reference into the RLMachine's memory,
// referencing a string value.
//
// This struct is used to define the parameter types of a RLOperation
// subclass, and should not be used directly. It should only be used
// as a template parameter to one of those classes, or of another type
// definition struct.
struct StrReference_T {
  // The output type of this type struct
  typedef StringReferenceIterator type;

  // Convert the incoming parameter objects into the resulting type
  static type getData(RLMachine& machine,
                      const libreallive::ExpressionPiecesVector& p,
                      unsigned int& position);

  // Parse the raw parameter string and put the results in ExpressionPiece
  static void ParseParameters(unsigned int& position,
                              const std::vector<std::string>& input,
                              libreallive::ExpressionPiecesVector& output);

  enum { is_complex = false };
};

#endif  // SRC_MACHINE_RLOPERATION_REFERENCES_H_
