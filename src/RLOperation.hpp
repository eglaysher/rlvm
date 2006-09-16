// This file is part of RLVM, a RealLive virutal machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006 El Riot
//  
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
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
/** 
 * @file
 * @ingroup RLOperation
 * @brief Defines all the base RLOperations and their type checking structs.
 */

#ifndef __RLOperation__h__
#define __RLOperation__h__

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/tuple/tuple.hpp>

#include "RLMachine.hpp"
#include "defs.h"
#include "expression.h"
#include "reference.hpp"

#include <iostream>

// ------------------------------------------------------------ Real

/**
 * @defgroup RLOperation RLOperations/Type structures
 *
 * Defines the base classes from which the all of the opcode
 * implementations derive from. This heiarchy of classes works by
 * having one of your operation classes, which handles a specific
 * prototype of a specific opcode, derfive from one of the subclases
 * of RLOperation, specifically RLOP_{Void,Store}_{# of
 * parameters}. The template parameters of these subclasses refer to
 * the types of the parameters.
 *
 * Valid type parameters are IntConstant_T, IntReference_T,
 * StrConstant_T, StrReference_T, Argc_T (takes another type as a
 * parameter), 
 * 
 * @{
 */

/** 
 * @brief Root of the entire operation heiarchy.
 *
 * RLOperations are grouped into RLModule s, which are then added to
 * the RLMachine.
 */
struct RLOperation {
  /** The type checker is called by the Module to make sure the parameters read in
   * are of the expected type.
   */
  virtual bool checkTypes(RLMachine& machine, 
                          boost::ptr_vector<Reallive::ExpressionPiece>& parameters) = 0;

  /** The dispatch function is implemented on a per type basis and is called by the
   * Module, after checking to make sure that the 
   */
  virtual void dispatch(RLMachine& machine, 
                        boost::ptr_vector<Reallive::ExpressionPiece>& parameters) = 0;

  void addParameterTo(const std::string& parameter, 
                      boost::ptr_vector<Reallive::ExpressionPiece>& outputParameters);

  /** The public interface used by the RLModule; how a method is dispatched.
   *
   * @param machine RLMachine to operate on
   * @param f CommandElement (assumed to be FunctionElement in default
   * implementation) to work on.
   */
  virtual void dispatchFunction(RLMachine& machine, 
                                const Reallive::CommandElement& f);
};

// -----------------------------------------------------------------------

/**
 * @brief Type definition for a Constant integer value. 
 * 
 * This struct is used to define the parameter types of a RLOperation
 * subclass, and should not be used directly. It should only be used
 * as a template parameter to one of those classes, or of another type
 * definition struct.
 */
struct IntConstant_T {
  /// The output type of this type struct
  typedef int type;

  /// Convert the incoming parameter objects into the resulting type
  static type getData(RLMachine& machine,
                      boost::ptr_vector<Reallive::ExpressionPiece>& p,
                      int position) {
    return p[position].getIntegerValue(machine);
   }

  /// Verify that the incoming parameter objects meet the desired types
  static bool verifyType(boost::ptr_vector<Reallive::ExpressionPiece>& p, int position);
};

// -----------------------------------------------------------------------

/** 
 * @brief Type definition for a reference into the RLMachine's memory,
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

  static type getData(RLMachine& machine, 
                      boost::ptr_vector<Reallive::ExpressionPiece>& p, 
                      int position) {
    return static_cast<Reallive::MemoryReference&>(p[position]).
      getIntegerReferenceIterator(machine);
  }

  static bool verifyType(boost::ptr_vector<Reallive::ExpressionPiece>& p,
                         int position);
};

// -----------------------------------------------------------------------

/** 
 * @brief Type definition for a constant string value. 
 * 
 * This struct is used to define the parameter types of a RLOperation
 * subclass, and should not be used directly. It should only be used
 * as a template parameter to one of those classes, or of another type
 * definition struct.
 */
struct StrConstant_T {
  /// The output type of this type struct
  typedef std::string type;

  /// Convert the incoming parameter objects into the resulting type
  static type getData(RLMachine& machine, 
                      boost::ptr_vector<Reallive::ExpressionPiece>& p,
                      int position) { 
    return p[position].getStringValue(machine);
  }

  /// Verify that the incoming parameter objects meet the desired types
  static bool verifyType(boost::ptr_vector<Reallive::ExpressionPiece>& p,
                         int position);
};

// -----------------------------------------------------------------------

/** 
 * @brief Type struct for a reference into the RLMachine's memory,
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

  /// 
  static type getData(RLMachine& machine, 
                      boost::ptr_vector<Reallive::ExpressionPiece>& p, 
                      int position) {
    return static_cast<Reallive::MemoryReference&>(p[position]).
      getStringReferenceIterator(machine);
  }

  static bool verifyType(boost::ptr_vector<Reallive::ExpressionPiece>& p,
                         int position);
};

// -----------------------------------------------------------------------

/** 
 * @brief Type struct that implements the argc concept. 
 *
 * This type struct can only be used as the last element in a type
 * definition. (This is not checked for at runtime; I'm not even sure
 * how I'd check this concept.) This type struct takes a type struct
 * as its parameter type, and then will accept a variable number of
 * items of that type.
 */
template<typename CON>
struct Argc_T {
  /// The output type of this type struct
  typedef std::vector<typename CON::type> type;

  /** Convert the incoming parameter objects into the resulting type.
   * Passes each parameter down to 
   */
  static type getData(RLMachine& machine, 
                      boost::ptr_vector<Reallive::ExpressionPiece>& p,
                      int position);

  /** Verify that the incoming parameter objects meet the desired types,
   * by passing each object from its spot in the parameters on to 
   * the template argument type.
   *
   * @return true if all parameters are of the correct type.
   */
  static bool verifyType(boost::ptr_vector<Reallive::ExpressionPiece>& p, 
                         int position);
};

// -----------------------------------------------------------------------

/** 
 * @brief Type definition that implements the complex parameter concept.
 *
 * It really should have been called tuple, but the name's stuck
 * now. Takes two other type structs as template parameters.
 */
template<typename A, typename B>
struct Complex2_T {
  /// The output type of this type struct
  typedef boost::tuple<typename A::type, typename B::type> type;

  /// Convert the incoming parameter objects into the resulting type.
  static type getData(RLMachine& machine, boost::ptr_vector<Reallive::ExpressionPiece>& p,
                      int position) {
    Reallive::ComplexExpressionPiece& sp = 
      static_cast<Reallive::ComplexExpressionPiece&>(p[position]);
    return boost::tuple<typename A::type, typename B::type>(
      A::getData(machine, sp.getContainedPieces(), 0), 
      B::getData(machine, sp.getContainedPieces(), 1));
  }

  /// Takes a type and makes sure that 
  static bool verifyType(boost::ptr_vector<Reallive::ExpressionPiece>& p, int position) {
    // Verify the size of the vector, that we have a special parameter, and then
    // make sure all the 
    bool typeOK = position < p.size();
    typeOK = typeOK && p[position].isComplexParameter(); 
    if(typeOK) {
      Reallive::ComplexExpressionPiece& sp = 
        static_cast<Reallive::ComplexExpressionPiece&>(p[position]);
      typeOK = typeOK && A::verifyType(sp.getContainedPieces(), 0);
      typeOK = typeOK && A::verifyType(sp.getContainedPieces(), 1);
    }
    return typeOK;
  }
};

// ----------------------------------------------------------------------

/**
 * Implements a special case operation. This should be used with
 * things that don't follow the usually function syntax in the
 * bytecode, such as weird gotos, et cetera.
 *
 * RLOp_SpecialCase gives you complete control of the dispatch,
 * performing no type checking, no parameter conversion, and no
 * implicit instruction pointer advancement. 
 * 
 * @warning This is almost certainly not what you want. This is only
 * used to define handlers for CommandElements that aren't
 * FunctionElements. Meaning the Gotos and Select.
 * @see JmpModule
 */
struct RLOp_SpecialCase : public RLOperation {
  /** 
   * Empty function defined simply to obey the interface
   */
  bool checkTypes(RLMachine& machine, 
                  boost::ptr_vector<Reallive::ExpressionPiece>& parameters);

  /** 
   * Empty function defined simply to obey the interface
   */
  void dispatch(RLMachine& machine, 
                boost::ptr_vector<Reallive::ExpressionPiece>& parameters);

  void dispatchFunction(RLMachine& machine, 
                        const Reallive::CommandElement& f);

  virtual void operator()(RLMachine&, const Reallive::CommandElement&) = 0;
};

// ----------------------------------------------------------------------

/** RLOp that doesn't expect a return value, and doesn't take any
 * parameters.
 */
struct RLOp_Void_Void : public RLOperation {
  bool checkTypes(RLMachine& machine, 
                  boost::ptr_vector<Reallive::ExpressionPiece>& parameters) {
    return parameters.size() == 0;
  }

  void dispatch(RLMachine& machine, 
                boost::ptr_vector<Reallive::ExpressionPiece>& parameters) {
    operator()(machine);
  }

  virtual void operator()(RLMachine&) = 0;
};

// -----------------------------------------------------------------------

/** RLOp that stores the return value in the store register, and which
 * takes no parameters.
 */
struct RLOp_Store_Void : public RLOperation {
  bool checkTypes(RLMachine& machine, 
                  boost::ptr_vector<Reallive::ExpressionPiece>& parameters) {
    return parameters.size() == 0;
  }

  void dispatch(RLMachine& machine, 
                boost::ptr_vector<Reallive::ExpressionPiece>& parameters) {
    int store = operator()(machine);
    machine.setStoreRegister(store);
  }

  virtual int operator()(RLMachine&) = 0;
};

// -----------------------------------------------------------------------

/** RLOp that doesn't expect a return value, and which takes a single
 * parameter.
 * 
 * @param A Type struct representing type to check for.
 */
template<typename A>
struct RLOp_Void_1 : public RLOperation {
  typedef typename A::type firstType;

  bool checkTypes(RLMachine& machine, boost::ptr_vector<Reallive::ExpressionPiece>& parameters) {
    return A::verifyType(parameters, 0);
  }

  void dispatch(RLMachine& machine, boost::ptr_vector<Reallive::ExpressionPiece>& parameters) {
    operator()(machine, A::getData(machine, parameters, 0));
  }

  virtual void operator()(RLMachine&, firstType) = 0;
};

// -----------------------------------------------------------------------

/** RLOp that stores the return in the store register, and which takes
 * a single parameter.
 * 
 * @param A Type struct representing type to check for.
 */
template<typename A>
struct RLOp_Store_1 : public RLOperation {
  typedef typename A::type firstType;

  bool checkTypes(RLMachine& machine, boost::ptr_vector<Reallive::ExpressionPiece>& parameters) {
    return A::verifyType(parameters, 0);
  }

  void dispatch(RLMachine& machine, boost::ptr_vector<Reallive::ExpressionPiece>& parameters) {
    int store = operator()(machine, A::getData(machine, parameters, 0));
    machine.setStoreRegister(store);
  }

  virtual int operator()(RLMachine&, firstType) = 0;
};

// -----------------------------------------------------------------------

/** RLOp that doesn't expect a return value, and which takes
 * two parameters.
 * 
 * @param A Type struct representing type to check for.
 * @param B Type struct representing type to check for.
 */
template<typename A, typename B>
struct RLOp_Void_2 : public RLOperation {
  typedef typename A::type firstType;
  typedef typename B::type secondType;

  bool checkTypes(RLMachine& machine, boost::ptr_vector<Reallive::ExpressionPiece>& parameters) {
    return A::verifyType(parameters, 0) &&
      B::verifyType(parameters, 1);
  }

  void dispatch(RLMachine& machine, boost::ptr_vector<Reallive::ExpressionPiece>& parameters) {
    operator()(machine, A::getData(machine, parameters, 0),
               B::getData(machine, parameters, 1));
  }

  virtual void operator()(RLMachine&, firstType, secondType) = 0;
};

// -----------------------------------------------------------------------

/** RLOp that stores the return in the store register, and which takes
 * two parameters.
 * 
 * @param A Type struct representing type to check for.
 * @param B Type struct representing type to check for.
 */
template<typename A, typename B>
struct RLOp_Store_2 : public RLOperation {
  typedef typename A::type firstType;
  typedef typename B::type secondType;

  bool checkTypes(RLMachine& machine, boost::ptr_vector<Reallive::ExpressionPiece>& parameters) {
    return A::verifyType(parameters, 0) &&
      B::verifyType(parameters, 1);
  }

  void dispatch(RLMachine& machine, boost::ptr_vector<Reallive::ExpressionPiece>& parameters) {
    int store = operator()(machine, A::getData(machine, parameters, 0),
                           B::getData(machine, parameters, 1));
    machine.setStoreRegister(store);
  }

  virtual int operator()(RLMachine&, firstType, secondType) = 0;
};

// -----------------------------------------------------------------------

/** RLOp that doesn't expect a return value, and which takes
 * three parameters.
 * 
 * @param A Type struct representing type to check for.
 * @param B Type struct representing type to check for.
 * @param C Type struct representing type to check for.
 */
template<typename A, typename B, typename C>
struct RLOp_Void_3 : public RLOperation {
  typedef typename A::type firstType;
  typedef typename B::type secondType;
  typedef typename C::type thirdType;

  bool checkTypes(RLMachine& machine, boost::ptr_vector<Reallive::ExpressionPiece>& parameters) {
    return A::verifyType(parameters, 0) &&
      B::verifyType(parameters, 1) &&
      C::verifyType(parameters, 2);
  }

  void dispatch(RLMachine& machine, boost::ptr_vector<Reallive::ExpressionPiece>& parameters) {
    operator()(machine, A::getData(machine, parameters, 0),
               B::getData(machine, parameters, 1), 
               C::getData(machine, parameters, 2));
  }

  virtual void operator()(RLMachine&, firstType, secondType, thirdType) = 0;
};

// -----------------------------------------------------------------------

/** RLOp that stores the return in the store register, and which takes
 * three parameters.
 * 
 * @param A Type struct representing type to check for.
 * @param B Type struct representing type to check for.
 * @param C Type struct representing type to check for.
 */
template<typename A, typename B, typename C>
struct RLOp_Store_3 : public RLOperation {
  typedef typename A::type firstType;
  typedef typename B::type secondType;
  typedef typename C::type thirdType;

  bool checkTypes(RLMachine& machine, boost::ptr_vector<Reallive::ExpressionPiece>& parameters) {
    return A::verifyType(parameters, 0) &&
      B::verifyType(parameters, 1) &&
      C::verifyType(parameters, 2);
  }

  void dispatch(RLMachine& machine, boost::ptr_vector<Reallive::ExpressionPiece>& parameters) {
    int store = operator()(machine, A::getData(machine, parameters, 0),
                           B::getData(machine, parameters, 1), 
                           C::getData(machine, parameters, 2));
    machine.setStoreRegister(store);
  }

  virtual int operator()(RLMachine&, firstType, secondType, thirdType) = 0;
};

// -----------------------------------------------------------------------

/** RLOp that doesn't expect a return value, and which takes
 * three parameters.
 * 
 * @param A Type struct representing type to check for.
 * @param B Type struct representing type to check for.
 * @param C Type struct representing type to check for.
 * @param D Type struct representing type to check for.
 */
template<typename A, typename B, typename C, typename D>
struct RLOp_Void_4 : public RLOperation {
  typedef typename A::type firstType;
  typedef typename B::type secondType;
  typedef typename C::type thirdType;
  typedef typename D::type fourthType;

  bool checkTypes(RLMachine& machine, boost::ptr_vector<Reallive::ExpressionPiece>& parameters) {
    return A::verifyType(parameters, 0) &&
      B::verifyType(parameters, 1) &&
      C::verifyType(parameters, 2) &&
      D::verifyType(parameters, 3);
  }

  void dispatch(RLMachine& machine, boost::ptr_vector<Reallive::ExpressionPiece>& parameters) {
    operator()(machine, A::getData(machine, parameters, 0),
               B::getData(machine, parameters, 1), 
               C::getData(machine, parameters, 2),
               D::getData(machine, parameters, 3));
  }

  virtual void operator()(RLMachine&, firstType, secondType, thirdType, fourthType) = 0;
};

// -----------------------------------------------------------------------

/** RLOp that stores the return in the store register, and which takes
 * three parameters.
 * 
 * @param A Type struct representing type to check for.
 * @param B Type struct representing type to check for.
 * @param C Type struct representing type to check for.
 * @param D Type struct representing type to check for.
 */
template<typename A, typename B, typename C, typename D>
struct RLOp_Store_4 : public RLOperation {
  typedef typename A::type firstType;
  typedef typename B::type secondType;
  typedef typename C::type thirdType;
  typedef typename D::type fourthType;

  bool checkTypes(RLMachine& machine, boost::ptr_vector<Reallive::ExpressionPiece>& parameters) {
    return A::verifyType(parameters, 0) &&
      B::verifyType(parameters, 1) &&
      C::verifyType(parameters, 2) &&
      D::verifyType(parameters, 3);
  }

  void dispatch(RLMachine& machine, boost::ptr_vector<Reallive::ExpressionPiece>& parameters) {
    int store = operator()(machine, A::getData(machine, parameters, 0),
                           B::getData(machine, parameters, 1), 
                           C::getData(machine, parameters, 2),
                           D::getData(machine, parameters, 3));
    machine.setStoreRegister(store);
  }

  virtual int operator()(RLMachine&, firstType, secondType, thirdType, fourthType) = 0;
};

// @}

// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// IMPLEMENTATION OF NON-INLINE FUNCTIONS FROM ABOVE DECLARATIONS
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------

template<typename CON>
struct Argc_T<CON>::type Argc_T<CON>::getData(RLMachine& machine, 
                     boost::ptr_vector<Reallive::ExpressionPiece>& p,
                     int position) {
  type returnVector;
  for(int i = position; i < p.size(); ++i)
    returnVector.push_back(CON::getData(machine, p, i));

  return returnVector;
}

// -----------------------------------------------------------------------

template<typename CON>
bool Argc_T<CON>::verifyType(boost::ptr_vector<Reallive::ExpressionPiece>& p,
                             int position) {
  for(int i = position; i < p.size(); ++i) {
    if(!CON::verifyType(p, i)) {
      return false;
    } 
  }

  return true;
}

#endif
