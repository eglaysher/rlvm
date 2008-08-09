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

#ifndef __Complex_T_hpp__
#define __Complex_T_hpp__

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
                      unsigned int position) {
    const libReallive::ComplexExpressionPiece& sp =
      static_cast<const libReallive::ComplexExpressionPiece&>(p[position]);
    return boost::tuple<typename A::type, typename B::type>(
      A::getData(machine, sp.getContainedPieces(), 0),
      B::getData(machine, sp.getContainedPieces(), 1));
  }

  /// Takes a type and makes sure that
/*
  static bool verifyType(const boost::ptr_vector<libReallive::ExpressionPiece>& p,
                         unsigned int position)
  {
    // Verify the size of the vector, that we have a special parameter, and then
    // make sure all the
    bool typeOK = position < p.size();
    typeOK = typeOK && p[position].isComplexParameter();
    if(typeOK) {
      const libReallive::ComplexExpressionPiece& sp =
        static_cast<const libReallive::ComplexExpressionPiece&>(p[position]);
      typeOK = typeOK && A::verifyType(sp.getContainedPieces(), 0);
      typeOK = typeOK && B::verifyType(sp.getContainedPieces(), 1);
    }
    return typeOK;
  }
*/

  static void parseParameters(unsigned int position,
                              const std::vector<std::string>& input,
                              boost::ptr_vector<libReallive::ExpressionPiece>& output)
  {
  const char* data = input.at(position).c_str();
  std::auto_ptr<libReallive::ExpressionPiece> ep(libReallive::get_complex_param(data));

//   if(ep->expressionValueType() != libReallive::ValueTypeString)
//   {
//     throw rlvm::Exception("StrConstant_T parse err.");
//   }

  output.push_back(ep.release());

//    throw rlvm::Exception("Unimplemented");
  }


  enum {
    isRealTypestruct = true,
    isComplex = true
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
                      unsigned int position)
  {
    const libReallive::ComplexExpressionPiece& sp =
      static_cast<const libReallive::ComplexExpressionPiece&>(p[position]);

    return type(
      A::getData(machine, sp.getContainedPieces(), 0),
      B::getData(machine, sp.getContainedPieces(), 1),
      C::getData(machine, sp.getContainedPieces(), 2));
  }

  /// Takes a type and makes sure that
/*
  static bool verifyType(const boost::ptr_vector<libReallive::ExpressionPiece>& p,
                         unsigned int position)
  {
    // Verify the size of the vector, that we have a special parameter, and then
    // make sure all the
    bool typeOK = position < p.size();
    typeOK = typeOK && p[position].isComplexParameter();
    if(typeOK) {
      const libReallive::ComplexExpressionPiece& sp =
        static_cast<const libReallive::ComplexExpressionPiece&>(p[position]);
      typeOK = typeOK && A::verifyType(sp.getContainedPieces(), 0);
      typeOK = typeOK && B::verifyType(sp.getContainedPieces(), 1);
      typeOK = typeOK && C::verifyType(sp.getContainedPieces(), 2);
    }
    return typeOK;
  }
*/

  static void parseParameters(unsigned int position,
                              const std::vector<std::string>& input,
                              boost::ptr_vector<libReallive::ExpressionPiece>& output)
  {
  const char* data = input.at(position).c_str();
  std::auto_ptr<libReallive::ExpressionPiece> ep(libReallive::get_complex_param(data));

//   if(ep->expressionValueType() != libReallive::ValueTypeString)
//   {
//     throw rlvm::Exception("StrConstant_T parse err.");
//   }

  output.push_back(ep.release());

//    throw rlvm::Exception("Unimplemented");
  }


  enum {
    isRealTypestruct = true,
    isComplex = true
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
                      unsigned int position)
  {
    const libReallive::ComplexExpressionPiece& sp =
      static_cast<const libReallive::ComplexExpressionPiece&>(p[position]);
    return type(
      A::getData(machine, sp.getContainedPieces(), 0),
      B::getData(machine, sp.getContainedPieces(), 1),
      C::getData(machine, sp.getContainedPieces(), 2),
      D::getData(machine, sp.getContainedPieces(), 3));
  }

/*
  /// Takes a type and makes sure that
  static bool verifyType(const boost::ptr_vector<libReallive::ExpressionPiece>& p,
                         unsigned int position)
  {
    // Verify the size of the vector, that we have a special parameter, and then
    // make sure all the
    bool typeOK = position < p.size();
    typeOK = typeOK && p[position].isComplexParameter();
    if(typeOK) {
      const libReallive::ComplexExpressionPiece& sp =
        static_cast<const libReallive::ComplexExpressionPiece&>(p[position]);
      typeOK = typeOK && A::verifyType(sp.getContainedPieces(), 0);
      typeOK = typeOK && B::verifyType(sp.getContainedPieces(), 1);
      typeOK = typeOK && C::verifyType(sp.getContainedPieces(), 2);
      typeOK = typeOK && D::verifyType(sp.getContainedPieces(), 3);
    }
    return typeOK;
  }
*/

  static void parseParameters(unsigned int position,
                              const std::vector<std::string>& input,
                              boost::ptr_vector<libReallive::ExpressionPiece>& output)
  {
    const char* data = input.at(position).c_str();
  std::auto_ptr<libReallive::ExpressionPiece> ep(libReallive::get_complex_param(data));

//   if(ep->expressionValueType() != libReallive::ValueTypeString)
//   {
//     throw rlvm::Exception("StrConstant_T parse err.");
//   }

  output.push_back(ep.release());

//  throw rlvm::Exception("Unimplemented");
  }


  enum {
    isRealTypestruct = true,
    isComplex = true
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
                      unsigned int position)
  {
    const libReallive::ComplexExpressionPiece& sp =
      static_cast<const libReallive::ComplexExpressionPiece&>(p[position]);
    return boost::tuple<typename A::type, typename B::type>(
      A::getData(machine, sp.getContainedPieces(), 0),
      B::getData(machine, sp.getContainedPieces(), 1),
      C::getData(machine, sp.getContainedPieces(), 2),
      D::getData(machine, sp.getContainedPieces(), 3),
      E::getData(machine, sp.getContainedPieces(), 4),
      F::getData(machine, sp.getContainedPieces(), 5),
      G::getData(machine, sp.getContainedPieces(), 6));
  }

/*
  /// Takes a type and makes sure that
  static bool verifyType(const boost::ptr_vector<libReallive::ExpressionPiece>& p,
                         unsigned int position)
  {
    // Verify the size of the vector, that we have a special parameter, and then
    // make sure all the
    bool typeOK = position < p.size();
    typeOK = typeOK && p[position].isComplexParameter();
    if(typeOK) {
      const libReallive::ComplexExpressionPiece& sp =
        static_cast<const libReallive::ComplexExpressionPiece&>(p[position]);
      typeOK = typeOK && A::verifyType(sp.getContainedPieces(), 0);
      typeOK = typeOK && B::verifyType(sp.getContainedPieces(), 1);
      typeOK = typeOK && C::verifyType(sp.getContainedPieces(), 2);
      typeOK = typeOK && D::verifyType(sp.getContainedPieces(), 3);
      typeOK = typeOK && E::verifyType(sp.getContainedPieces(), 4);
      typeOK = typeOK && F::verifyType(sp.getContainedPieces(), 5);
      typeOK = typeOK && G::verifyType(sp.getContainedPieces(), 6);
    }
    return typeOK;
  }
*/

  static void parseParameters(unsigned int position,
                              const std::vector<std::string>& input,
                              boost::ptr_vector<libReallive::ExpressionPiece>& output)
  {
  const char* data = input.at(position).c_str();
  std::auto_ptr<libReallive::ExpressionPiece> ep(libReallive::get_complex_param(data));

//   if(ep->expressionValueType() != libReallive::ValueTypeString)
//   {
//     throw rlvm::Exception("StrConstant_T parse err.");
//   }

  output.push_back(ep.release());


//    throw rlvm::Exception("Unimplemented");
  }


  enum {
    isRealTypestruct = true,
    isComplex = true
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
                      unsigned int position)
  {
    const libReallive::ComplexExpressionPiece& sp =
      static_cast<const libReallive::ComplexExpressionPiece&>(p[position]);
    return boost::tuple<typename A::type, typename B::type>(
      A::getData(machine, sp.getContainedPieces(), 0),
      B::getData(machine, sp.getContainedPieces(), 1),
      C::getData(machine, sp.getContainedPieces(), 2),
      D::getData(machine, sp.getContainedPieces(), 3),
      E::getData(machine, sp.getContainedPieces(), 4),
      F::getData(machine, sp.getContainedPieces(), 5),
      G::getData(machine, sp.getContainedPieces(), 6),
      H::getData(machine, sp.getContainedPieces(), 7));
  }

  static void parseParameters(unsigned int position,
                              const std::vector<std::string>& input,
                              boost::ptr_vector<libReallive::ExpressionPiece>& output)
  {
  const char* data = input.at(position).c_str();
  std::auto_ptr<libReallive::ExpressionPiece> ep(libReallive::get_complex_param(data));

//   if(ep->expressionValueType() != libReallive::ValueTypeString)
//   {
//     throw rlvm::Exception("StrConstant_T parse err.");
//   }

  output.push_back(ep.release());


//    throw rlvm::Exception("Unimplemented");
  }

  /// Takes a type and makes sure that
/*
  static bool verifyType(const boost::ptr_vector<libReallive::ExpressionPiece>& p,
                         unsigned int position)
  {
    // Verify the size of the vector, that we have a special parameter, and then
    // make sure all the
    bool typeOK = position < p.size();
    typeOK = typeOK && p[position].isComplexParameter();
    if(typeOK) {
      const libReallive::ComplexExpressionPiece& sp =
        static_cast<const libReallive::ComplexExpressionPiece&>(p[position]);
      typeOK = typeOK && A::verifyType(sp.getContainedPieces(), 0);
      typeOK = typeOK && B::verifyType(sp.getContainedPieces(), 1);
      typeOK = typeOK && C::verifyType(sp.getContainedPieces(), 2);
      typeOK = typeOK && D::verifyType(sp.getContainedPieces(), 3);
      typeOK = typeOK && E::verifyType(sp.getContainedPieces(), 4);
      typeOK = typeOK && F::verifyType(sp.getContainedPieces(), 5);
      typeOK = typeOK && G::verifyType(sp.getContainedPieces(), 6);
      typeOK = typeOK && H::verifyType(sp.getContainedPieces(), 7);
    }
    return typeOK;
  }
*/

  enum {
    isRealTypestruct = true,
    isComplex = true
  };
};

#endif
