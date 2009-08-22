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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// -----------------------------------------------------------------------

#ifndef SRC_MACHINEBASE_RLOPERATION_REFERENCES_HPP_
#define SRC_MACHINEBASE_RLOPERATION_REFERENCES_HPP_

#include "MachineBase/reference.hpp"
#include "libReallive/expression.h"
#include "libReallive/expression_pieces.h"

// -----------------------------------------------------------------------

/**
 * Type definition for a reference into the RLMachine's memory,
 * referencing an integer value.
 *
 * This struct is used to define the parameter types of a RLOperation
 * subclass, and should not be used directly. It should only be used
 * as a template parameter to one of those classes, or of another type
 * definition struct.
 */
struct IntReference_T {
  /// We pass iterators to the
  typedef IntReferenceIterator type;

  /// Convert the incoming parameter objects into the resulting type
  static type getData(RLMachine& machine,
                      const boost::ptr_vector<libReallive::ExpressionPiece>& p,
                      unsigned int& position);

  /// Parse the raw parameter string and put the results in ExpressionPiece
  static void parseParameters(unsigned int& position,
                              const std::vector<std::string>& input,
                              boost::ptr_vector<libReallive::ExpressionPiece>& output);

  enum {
    is_real_typestruct = true,
    is_complex = false
  };
};

// -----------------------------------------------------------------------


/**
 * Type struct for a reference into the RLMachine's memory,
 * referencing a string value.
 *
 * This struct is used to define the parameter types of a RLOperation
 * subclass, and should not be used directly. It should only be used
 * as a template parameter to one of those classes, or of another type
 * definition struct.
 */
struct StrReference_T {
  /// The output type of this type struct
  typedef StringReferenceIterator type;

  /// Convert the incoming parameter objects into the resulting type
  static type getData(RLMachine& machine,
                      const boost::ptr_vector<libReallive::ExpressionPiece>& p,
                      unsigned int& position);

  /// Parse the raw parameter string and put the results in ExpressionPiece
  static void parseParameters(unsigned int& position,
                              const std::vector<std::string>& input,
                              boost::ptr_vector<libReallive::ExpressionPiece>& output);

  enum {
    is_real_typestruct = true,
    is_complex = false
  };
};

// -----------------------------------------------------------------------

#endif  // SRC_MACHINEBASE_RLOPERATION_REFERENCES_HPP_
