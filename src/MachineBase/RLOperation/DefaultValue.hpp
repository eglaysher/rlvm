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

#ifndef SRC_MACHINEBASE_RLOPERATION_DEFAULTVALUE_HPP_
#define SRC_MACHINEBASE_RLOPERATION_DEFAULTVALUE_HPP_

#include <boost/ptr_container/ptr_vector.hpp>
#include "libReallive/expression_pieces.h"


template<int DEFAULTVAL>
struct DefaultIntValue_T {
  /// The output type of this type struct
  typedef int type;

  /// Convert the incoming parameter objects into the resulting type
  static type getData(RLMachine& machine,
                      const boost::ptr_vector<libReallive::ExpressionPiece>& p,
                      unsigned int& position) {
    if(position < p.size()) {
      return IntConstant_T::getData(machine, p, position);
    } else {
      return DEFAULTVAL;
    }
  }

  static void parseParameters(unsigned int& position,
                              const std::vector<std::string>& input,
                              boost::ptr_vector<libReallive::ExpressionPiece>& output) {
    if(position < input.size()) {
      IntConstant_T::parseParameters(position, input, output);
    } else {
      output.push_back(new libReallive::IntegerConstant(DEFAULTVAL));
      position++;
    }
  }

  enum {
    is_real_typestruct = true,
    is_complex = false
  };
};

// -----------------------------------------------------------------------

/**
 * Typestruct that will return an empty string if there isn't a value.
 */
struct DefaultStrValue_T {
  /// The output type of this type struct
  typedef std::string type;

  /// Convert the incoming parameter objects into the resulting type
  static type getData(RLMachine& machine,
                      const boost::ptr_vector<libReallive::ExpressionPiece>& p,
                      unsigned int& position) {
    if(position < p.size()) {
      return StrConstant_T::getData(machine, p, position);
    } else {
      return std::string();
    }
  }

  static void parseParameters(unsigned int& position,
                              const std::vector<std::string>& input,
                              boost::ptr_vector<libReallive::ExpressionPiece>& output) {
    if(position < input.size()) {
      StrConstant_T::parseParameters(position, input, output);
    } else {
      output.push_back(new libReallive::StringConstant(std::string()));
      position++;
    }
  }

  enum {
    is_real_typestruct = true,
    is_complex = false
  };
};

#endif  // SRC_MACHINEBASE_RLOPERATION_DEFAULTVALUE_HPP_
