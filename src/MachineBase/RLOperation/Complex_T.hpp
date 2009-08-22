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

#ifndef SRC_MACHINEBASE_RLOPERATION_COMPLEX_T_HPP_
#define SRC_MACHINEBASE_RLOPERATION_COMPLEX_T_HPP_

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/tuple/tuple.hpp>

#include "libReallive/bytecode_fwd.h"
#include "libReallive/expression.h"
#include "libReallive/expression_pieces.h"

/**
 * Type definition that implements the complex parameter concept.
 *
 * It really should have been called tuple, but the name's stuck
 * now. Takes two other type structs as template parameters.
 */
template<typename A, typename B>
struct Complex2_T {
  /// The output type of this type struct
  typedef boost::tuple<typename A::type, typename B::type> type;

  /// Convert the incoming parameter objects into the resulting type.
  static type getData(RLMachine& machine,
                      const boost::ptr_vector<libReallive::ExpressionPiece>& p,
                      unsigned int& position) {
    unsigned int pos_in_expression = 0;
    const libReallive::ComplexExpressionPiece& sp =
      static_cast<const libReallive::ComplexExpressionPiece&>(p[position++]);
    typename A::type a = A::getData(machine, sp.getContainedPieces(), pos_in_expression);
    typename B::type b = B::getData(machine, sp.getContainedPieces(), pos_in_expression);
    return type(a, b);
  }

  static void parseParameters(unsigned int& position,
                              const std::vector<std::string>& input,
                              boost::ptr_vector<libReallive::ExpressionPiece>& output) {
    const char* data = input.at(position).c_str();
    std::auto_ptr<libReallive::ExpressionPiece> ep(libReallive::get_complex_param(data));
    output.push_back(ep.release());
    position++;
  }

  enum {
    is_real_typestruct = true,
    is_complex = true
  };
};

// -----------------------------------------------------------------------

/**
 * Type definition that implements the complex parameter concept.
 *
 * It really should have been called tuple, but the name's stuck
 * now. Takes two other type structs as template parameters.
 */
template<typename A, typename B, typename C>
struct Complex3_T {
  /// The output type of this type struct
  typedef boost::tuple<typename A::type, typename B::type,
                       typename C::type> type;

  /// Convert the incoming parameter objects into the resulting type.
  static type getData(RLMachine& machine,
                      const boost::ptr_vector<libReallive::ExpressionPiece>& p,
                      unsigned int& position) {
    unsigned int pos_in_expression = 0;
    const libReallive::ComplexExpressionPiece& sp =
      static_cast<const libReallive::ComplexExpressionPiece&>(p[position++]);
    typename A::type a = A::getData(machine, sp.getContainedPieces(), pos_in_expression);
    typename B::type b = B::getData(machine, sp.getContainedPieces(), pos_in_expression);
    typename C::type c = C::getData(machine, sp.getContainedPieces(), pos_in_expression);
    return type(a, b, c);
  }

  static void parseParameters(unsigned int& position,
                              const std::vector<std::string>& input,
                              boost::ptr_vector<libReallive::ExpressionPiece>& output) {
    const char* data = input.at(position).c_str();
    std::auto_ptr<libReallive::ExpressionPiece> ep(libReallive::get_complex_param(data));
    output.push_back(ep.release());
    position++;
  }

  enum {
    is_real_typestruct = true,
    is_complex = true
  };
};

// -----------------------------------------------------------------------

/**
 * Type definition that implements the complex parameter concept.
 *
 * It really should have been called tuple, but the name's stuck
 * now. Takes two other type structs as template parameters.
 */
template<typename A, typename B, typename C, typename D>
struct Complex4_T {
  /// The output type of this type struct
  typedef boost::tuple<typename A::type, typename B::type,
                       typename C::type, typename D::type> type;

  /// Convert the incoming parameter objects into the resulting type.
  static type getData(RLMachine& machine,
                      const boost::ptr_vector<libReallive::ExpressionPiece>& p,
                      unsigned int& position) {
    unsigned int pos_in_expression = 0;

    const libReallive::ComplexExpressionPiece& sp =
      static_cast<const libReallive::ComplexExpressionPiece&>(p[position++]);
    typename A::type a = A::getData(machine, sp.getContainedPieces(), pos_in_expression);
    typename B::type b = B::getData(machine, sp.getContainedPieces(), pos_in_expression);
    typename C::type c = C::getData(machine, sp.getContainedPieces(), pos_in_expression);
    typename D::type d = D::getData(machine, sp.getContainedPieces(), pos_in_expression);
    return type(a, b, c, d);
  }

  static void parseParameters(unsigned int& position,
                              const std::vector<std::string>& input,
                              boost::ptr_vector<libReallive::ExpressionPiece>& output) {
    const char* data = input.at(position).c_str();
    std::auto_ptr<libReallive::ExpressionPiece> ep(libReallive::get_complex_param(data));
    output.push_back(ep.release());
    position++;
  }

  enum {
    is_real_typestruct = true,
    is_complex = true
  };
};


// -----------------------------------------------------------------------

/**
 * Type definition that implements the complex parameter concept.
 *
 * It really should have been called tuple, but the name's stuck
 * now. Takes two other type structs as template parameters.
 */
template<typename A, typename B, typename C, typename D, typename E,
         typename F, typename G>
struct Complex7_T {
  /// The output type of this type struct
  typedef boost::tuple<typename A::type, typename B::type,
                       typename C::type, typename D::type,
                       typename E::type, typename F::type,
                       typename G::type> type;

  /// Convert the incoming parameter objects into the resulting type.
  static type getData(RLMachine& machine,
                      const boost::ptr_vector<libReallive::ExpressionPiece>& p,
                      unsigned int& position) {
    unsigned int pos_in_expression = 0;

    const libReallive::ComplexExpressionPiece& sp =
      static_cast<const libReallive::ComplexExpressionPiece&>(p[position++]);
    typename A::type a = A::getData(machine, sp.getContainedPieces(), pos_in_expression);
    typename B::type b = B::getData(machine, sp.getContainedPieces(), pos_in_expression);
    typename C::type c = C::getData(machine, sp.getContainedPieces(), pos_in_expression);
    typename D::type d = D::getData(machine, sp.getContainedPieces(), pos_in_expression);
    typename E::type e = E::getData(machine, sp.getContainedPieces(), pos_in_expression);
    typename F::type f = F::getData(machine, sp.getContainedPieces(), pos_in_expression);
    typename G::type g = G::getData(machine, sp.getContainedPieces(), pos_in_expression);
    return type(a, b, c, d, e, f, g);
  }

  static void parseParameters(unsigned int& position,
                              const std::vector<std::string>& input,
                              boost::ptr_vector<libReallive::ExpressionPiece>& output) {
    const char* data = input.at(position).c_str();
    std::auto_ptr<libReallive::ExpressionPiece> ep(libReallive::get_complex_param(data));
    output.push_back(ep.release());
    position++;
  }

  enum {
    is_real_typestruct = true,
    is_complex = true
  };
};

// -----------------------------------------------------------------------

/**
 * Type definition that implements the complex parameter concept.
 *
 * It really should have been called tuple, but the name's stuck
 * now. Takes two other type structs as template parameters.
 */
template<typename A, typename B, typename C, typename D, typename E,
         typename F, typename G, typename H>
struct Complex8_T {
  /// The output type of this type struct
  typedef boost::tuple<typename A::type, typename B::type,
                       typename C::type, typename D::type,
                       typename E::type, typename F::type,
                       typename G::type, typename H::type> type;

  /// Convert the incoming parameter objects into the resulting type.
  static type getData(RLMachine& machine,
                      const boost::ptr_vector<libReallive::ExpressionPiece>& p,
                      unsigned int& position) {
    unsigned int pos_in_expression = 0;
    const libReallive::ComplexExpressionPiece& sp =
      static_cast<const libReallive::ComplexExpressionPiece&>(p[position++]);
    typename A::type a = A::getData(machine, sp.getContainedPieces(), pos_in_expression);
    typename B::type b = B::getData(machine, sp.getContainedPieces(), pos_in_expression);
    typename C::type c = C::getData(machine, sp.getContainedPieces(), pos_in_expression);
    typename D::type d = D::getData(machine, sp.getContainedPieces(), pos_in_expression);
    typename E::type e = E::getData(machine, sp.getContainedPieces(), pos_in_expression);
    typename F::type f = F::getData(machine, sp.getContainedPieces(), pos_in_expression);
    typename G::type g = G::getData(machine, sp.getContainedPieces(), pos_in_expression);
    typename H::type h = H::getData(machine, sp.getContainedPieces(), pos_in_expression);
    return type(a, b, c, d, e, f, g, h);
  }

  static void parseParameters(unsigned int& position,
                              const std::vector<std::string>& input,
                              boost::ptr_vector<libReallive::ExpressionPiece>& output) {
    const char* data = input.at(position).c_str();
    std::auto_ptr<libReallive::ExpressionPiece> ep(libReallive::get_complex_param(data));
    output.push_back(ep.release());
    position++;
  }

  enum {
    is_real_typestruct = true,
    is_complex = true
  };
};

#endif  // SRC_MACHINEBASE_RLOPERATION_COMPLEX_T_HPP_
