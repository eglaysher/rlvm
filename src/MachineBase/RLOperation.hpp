// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006, 2007 Elliot Glaysher
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
 * @ingroup RLOperationGroup
 * @brief Defines all the base RLOperations and their type checking structs.
 */

#ifndef __RLOperation__h__
#define __RLOperation__h__

#include <boost/ptr_container/ptr_vector.hpp>

#include "libReallive/bytecode_fwd.h"
#include "libReallive/expression.h"

#include "Utilities.h"

// ------------------------------------------------------------ Real

/**
 * @defgroup RLOperationGroup RLOperation and its type system
 *
 * Defines the base classes from which all of the opcode
 * implementations derive from. This heirarchy of classes works by
 * having one of your operation classes, which handles a specific
 * prototype of a specific opcode, derive from one of the subclases
 * of RLOperation, specifically RLOp_Void_* and RLOp_Store_*. The
 * template parameters of these subclasses refer to the types of the
 * parameters, some of which can be composed to represent more complex
 * parameters.
 *
 * Valid type parameters are IntConstant_T, IntReference_T,
 * StrConstant_T, StrReference_T, Argc_T< U > (takes another type as a
 * parameter). The type parameters change the arguments to the
 * implementation function.
 *
 * Let's say we want to implement an operation with the following
 * prototype: <tt>fun (store)doSomething(str, intC+)</tt>. The
 * function returns an integer value to the store register, so we want
 * to derive the implementation struct from RLOp_Store<>, which will
 * automatically place the return value in the store register. Our
 * first parameter is a reference to a piece of string memory, so our
 * first template argument is StrReference_T. We then take a variable
 * number of ints, so we compose IntConstant_T into the template
 * Argc_T for our second parameter.
 * 
 * Thus, our sample operation would be implemented with this:
 *
 * @code
 * struct Operation : public RLOp_Store_2<StrReference_T, Argc_T< IntConstant_T > > {
 *   int operator()(RLMachine& machine, StringReferenceIterator x, vector<int> y) {
 *     // Do whatever with the input parameters...
 *     return 5;
 *   }
 * }; 
 * @endcode
 *
 * For information on how to group RLOperations into modules to
 * attach to an RLMachine instance, please see @ref ModulesOpcodes
 * "Modules and Opcode Definitions".
 *
 * @{
 */

/** 
 * An RLOperation object implements an individual bytecode
 * command. All command bytecodes have a corresponding instance of a
 * subclass of RLOperation that defines it. 
 *
 * RLOperations are grouped into RLModule s, which are then added to
 * the RLMachine.
 */
struct RLOperation {
private:
  std::string m_name;

public:
  /// Default constructor
  RLOperation();

  /// Destructor
  virtual ~RLOperation();

  void setName(const std::string& name) { m_name = name; }
  const std::string& name() const { return m_name; }

  /** 
   * Check made as to whether the instruction pointer should be
   * incremented after the instruction is executed. Override this in
   * the *rare* case where an instruction messes about with the
   * instruction pointer and we don't want to mess with it afterwards.
   * The default implmentation returns true; 99% of instructions want
   * the instruction pointer to be advanced automaticly.
   */
  virtual bool advanceInstructionPointer();

  /** The dispatch function is implemented on a per type basis and is called by the
   * Module, after checking to make sure that the 
   */
  virtual void dispatch(
    RLMachine& machine, 
    const boost::ptr_vector<libReallive::ExpressionPiece>& parameters) = 0;

  /** 
   * Parses the parameters in the CommandElement passed in into an
   * output ptr_vector that contains parsed ExpressionPieces for each 
   * 
   * @param input The incoming CommandElement
   * @param output The output ptr_vector, filled with the parsed parameters
   */
  virtual void parseParameters(const std::vector<std::string>& input,
                       boost::ptr_vector<libReallive::ExpressionPiece>& output) = 0;

  /** The public interface used by the RLModule; how a method is dispatched.
   *
   * @param machine RLMachine to operate on
   * @param f CommandElement (assumed to be FunctionElement in default
   * implementation) to work on.
   */
  virtual void dispatchFunction(RLMachine& machine, 
                                const libReallive::CommandElement& f);

  /// Used for quality control. The downside of a dynamic typesystem
  /// hack is errors found at runtime instead of compiletime. *tear*
  void throw_unimplemented();
};

// -----------------------------------------------------------------------

/**
 * Type definition for a Constant integer value. 
 * 
 * This struct is used to define the parameter types of a RLOperation
 * subclass, and should not be used directly. It should only be used
 * as a template parameter to one of those classes, or of another type
 * definition struct.
 *
 * The
 */
struct IntConstant_T {
  /// The output type of this type struct
  typedef int type;

  /// Convert the incoming parameter objects into the resulting type
  static type getData(RLMachine& machine,
                      const boost::ptr_vector<libReallive::ExpressionPiece>& p,
                      unsigned int position);

  /// Parse the raw parameter string and put the results in ExpressionPiece
  static void parseParameters(unsigned int position,
                              const std::vector<std::string>& input,
                              boost::ptr_vector<libReallive::ExpressionPiece>& output);

  enum {
    isRealTypestruct = true,
    isComplex = false
  };
};

// -----------------------------------------------------------------------

/** 
 * Type definition for a constant string value. 
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
                      const boost::ptr_vector<libReallive::ExpressionPiece>& p,
                      unsigned int position);


  /// Parse the raw parameter string and put the results in ExpressionPiece
  static void parseParameters(unsigned int position,
                              const std::vector<std::string>& input,
                              boost::ptr_vector<libReallive::ExpressionPiece>& output);

  enum {
    isRealTypestruct = true,
    isComplex = false
  };
};

// -----------------------------------------------------------------------

// -----------------------------------------------------------------------

struct emptyStruct { };

/// Defines a null type for the Special parameter.
struct Empty_T {
  typedef emptyStruct type;

  /// Convert the incoming parameter objects into the resulting type.
  static type getData(RLMachine& machine, 
                      const boost::ptr_vector<libReallive::ExpressionPiece>& p,
                      unsigned int position);

  /// Parse the raw parameter string and put the results in ExpressionPiece
  static void parseParameters(unsigned int position,
                              const std::vector<std::string>& input,
                              boost::ptr_vector<libReallive::ExpressionPiece>& output);


  enum {
    isRealTypestruct = false,
    isComplex = false
  };
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
 * FunctionElements. Meaning the Gotos and Select. Also, you get to do
 * weird tricks with the 
 *
 * @todo Make this invoke parseParameters
 *
 * @see JmpModule
 */
struct RLOp_SpecialCase : public RLOperation {

  /** 
   * Empty function defined simply to obey the interface
   */
  void dispatch(RLMachine& machine, 
                const boost::ptr_vector<libReallive::ExpressionPiece>& parameters);

  void dispatchFunction(RLMachine& machine, 
                        const libReallive::CommandElement& f);

  /// Default implementation that simply parses everything as data;
  /// doesn't work in the case of complex expressions.
  void parseParameters(const std::vector<std::string>& input,
                       boost::ptr_vector<libReallive::ExpressionPiece>& output);

  /// Method that is overridden by all subclasses to implement the
  /// function of this opcode
  virtual void operator()(RLMachine&, const libReallive::CommandElement&) = 0;
};

// -----------------------------------------------------------------------

/** 
 * Base class for all the normal operations; This is the third
 * revision of this part of the type system. It was revised into two
 * monolithic classes, RLOp_Void<> and RLOp_Store<>, but that got a
 * bit wacky with what was a big set of if/else if statements
 * dispatching to every possilbe operator(). It also suffered the
 * weakeness that each derived struct had a vtable with 26*4 entries
 * for all the possible number of operations, most of which have a
 * default implementation that just throws an exception; it's possible
 * to have a mismatch between the stated function signature and the
 * overloaded method.
 *
 * This implementation doesn't suffer that. Parameter mismatch between
 * the type definition and the operator()() implementation is a
 * compile time error now. (While porting from the old system to the
 * new, I discovered and fixed several of these errors.) The vtable
 * isn't hueg liek xbox anymore.
 */
template<typename A = Empty_T, typename B = Empty_T, typename C = Empty_T, 
         typename D = Empty_T, typename E = Empty_T, typename F = Empty_T,
         typename G = Empty_T, typename H = Empty_T, typename I = Empty_T,
         typename J = Empty_T, typename K = Empty_T, typename L = Empty_T,
         typename M = Empty_T, typename N = Empty_T, typename O = Empty_T,
         typename P = Empty_T, typename Q = Empty_T, typename R = Empty_T,
         typename S = Empty_T, typename T = Empty_T, typename U = Empty_T,
         typename V = Empty_T, typename W = Empty_T, typename X = Empty_T,
         typename Y = Empty_T, typename Z = Empty_T>
struct RLOp_NormalOperation : public RLOperation {
private:
  template<typename TYPE>
  void addTypeTo(int position, const std::vector<std::string>& input,
                 boost::ptr_vector<libReallive::ExpressionPiece>& output)
  {
    if(TYPE::isRealTypestruct)
    {
      TYPE::parseParameters(position, input, output);
    }
  }

public:
  void parseParameters(const std::vector<std::string>& input,
                       boost::ptr_vector<libReallive::ExpressionPiece>& output)
  {
    addTypeTo<A>(0, input, output);
    addTypeTo<B>(1, input, output);
    addTypeTo<C>(2, input, output);
    addTypeTo<D>(3, input, output);
    addTypeTo<E>(4, input, output);
    addTypeTo<F>(5, input, output);
    addTypeTo<G>(6, input, output);
    addTypeTo<H>(7, input, output);
    addTypeTo<I>(8, input, output);
    addTypeTo<J>(9, input, output);
    addTypeTo<K>(10, input, output);
    addTypeTo<L>(11, input, output);
    addTypeTo<M>(12, input, output);
    addTypeTo<N>(13, input, output);
    addTypeTo<O>(14, input, output);
    addTypeTo<P>(15, input, output);
    addTypeTo<Q>(16, input, output);
    addTypeTo<R>(17, input, output);
    addTypeTo<S>(18, input, output);
    addTypeTo<T>(19, input, output);
    addTypeTo<U>(20, input, output);
    addTypeTo<V>(21, input, output);
    addTypeTo<W>(22, input, output);
    addTypeTo<X>(23, input, output);
    addTypeTo<Y>(24, input, output);
    addTypeTo<Z>(25, input, output);
  }
};

// Partial specialization for RLOp_Normal::checkTypes for when
// everything is empty (aka an operation that takes no parameters)
template<>
inline void RLOp_NormalOperation<
  Empty_T, Empty_T, Empty_T, Empty_T, Empty_T, Empty_T, Empty_T, 
  Empty_T, Empty_T, Empty_T, Empty_T, Empty_T, Empty_T, Empty_T, 
  Empty_T, Empty_T, Empty_T, Empty_T, Empty_T, Empty_T, Empty_T,
  Empty_T, Empty_T, Empty_T, Empty_T, Empty_T>::
parseParameters(const std::vector<std::string>& input,
                boost::ptr_vector<libReallive::ExpressionPiece>& output)
{
}

// -----------------------------------------------------------------------

struct RLOp_Void_Void : public RLOp_NormalOperation<>
{
  void dispatch(RLMachine& machine, 
                const boost::ptr_vector<libReallive::ExpressionPiece>& parameters) 
  {
    operator()(machine);
  }

  virtual void operator()(RLMachine&) = 0;
};

// -----------------------------------------------------------------------

template<typename A>
struct RLOp_Void_1 : public RLOp_NormalOperation<A>
{
  void dispatch(RLMachine& machine, 
                const boost::ptr_vector<libReallive::ExpressionPiece>& parameters) 
  { 
    operator()(machine, A::getData(machine, parameters, 0));
  }

  virtual void operator()(RLMachine&, typename A::type) = 0;
};

// -----------------------------------------------------------------------

template<typename A, typename B>
struct RLOp_Void_2 : public RLOp_NormalOperation<A, B>
{
  void dispatch(RLMachine& machine, 
                const boost::ptr_vector<libReallive::ExpressionPiece>& parameters) 
  {
    operator()(machine, A::getData(machine, parameters, 0),
               B::getData(machine, parameters, 1));
  }

  virtual void operator()(RLMachine&, typename A::type, typename B::type) = 0;  
};

// -----------------------------------------------------------------------

template<typename A, typename B, typename C>
struct RLOp_Void_3 : public RLOp_NormalOperation<A, B, C>
{
  void dispatch(RLMachine& machine, 
                const boost::ptr_vector<libReallive::ExpressionPiece>& parameters) 
  {
      operator()(machine, A::getData(machine, parameters, 0),
                 B::getData(machine, parameters, 1),
                 C::getData(machine, parameters, 2));
  }

  virtual void operator()(RLMachine&, typename A::type, typename B::type, typename C::type) = 0;
};

// -----------------------------------------------------------------------

template<typename A, typename B, typename C, typename D>
struct RLOp_Void_4 : public RLOp_NormalOperation<A, B, C, D>
{
  void dispatch(RLMachine& machine, 
                const boost::ptr_vector<libReallive::ExpressionPiece>& parameters) 
  {
      operator()(machine, A::getData(machine, parameters, 0),
                 B::getData(machine, parameters, 1),
                 C::getData(machine, parameters, 2),
                 D::getData(machine, parameters, 3));
  }

  virtual void operator()(RLMachine&, typename A::type,typename B::type, typename C::type, 
                          typename D::type) = 0;
};

// -----------------------------------------------------------------------


template<typename A, typename B, typename C, typename D, typename E>
struct RLOp_Void_5 : public RLOp_NormalOperation<A, B, C, D, E>
{
  void dispatch(RLMachine& machine, 
                const boost::ptr_vector<libReallive::ExpressionPiece>& parameters) 
  {
      operator()(machine, A::getData(machine, parameters, 0),
                 B::getData(machine, parameters, 1),
                 C::getData(machine, parameters, 2),
                 D::getData(machine, parameters, 3),
                 E::getData(machine, parameters, 4));
  }

  virtual void operator()(RLMachine&, typename A::type,typename B::type, typename C::type, 
                          typename D::type, typename E::type) = 0;
};

// -----------------------------------------------------------------------


template<typename A, typename B, typename C, typename D, typename E, 
         typename F>
struct RLOp_Void_6 : public RLOp_NormalOperation<A, B, C, D, E, F>
{
  void dispatch(RLMachine& machine, 
                const boost::ptr_vector<libReallive::ExpressionPiece>& parameters) 
  {
      operator()(machine, A::getData(machine, parameters, 0),
                 B::getData(machine, parameters, 1),
                 C::getData(machine, parameters, 2),
                 D::getData(machine, parameters, 3),
                 E::getData(machine, parameters, 4),
                 F::getData(machine, parameters, 5));
  }

  virtual void operator()(RLMachine&, typename A::type,typename B::type, typename C::type, 
                          typename D::type, typename E::type, typename F::type) = 0;
};

// -----------------------------------------------------------------------

template<typename A, typename B, typename C, typename D, typename E, 
         typename F, typename G>
struct RLOp_Void_7 : public RLOp_NormalOperation<A, B, C, D, E, F, G>
{
  void dispatch(RLMachine& machine, 
                const boost::ptr_vector<libReallive::ExpressionPiece>& parameters) 
  {
      operator()(machine, A::getData(machine, parameters, 0),
                 B::getData(machine, parameters, 1),
                 C::getData(machine, parameters, 2),
                 D::getData(machine, parameters, 3),
                 E::getData(machine, parameters, 4),
                 F::getData(machine, parameters, 5),
                 G::getData(machine, parameters, 6));
  }

  virtual void operator()(RLMachine&, typename A::type,typename B::type, typename C::type, 
                          typename D::type, typename E::type, typename F::type, typename G::type) = 0;
};

// -----------------------------------------------------------------------

template<typename A, typename B, typename C, typename D, typename E, 
         typename F, typename G, typename H>
struct RLOp_Void_8 : public RLOp_NormalOperation<A, B, C, D, E, F, G, H>
{
  void dispatch(RLMachine& machine, 
                const boost::ptr_vector<libReallive::ExpressionPiece>& parameters) 
  {
      operator()(machine, A::getData(machine, parameters, 0),
                 B::getData(machine, parameters, 1),
                 C::getData(machine, parameters, 2),
                 D::getData(machine, parameters, 3),
                 E::getData(machine, parameters, 4),
                 F::getData(machine, parameters, 5),
                 G::getData(machine, parameters, 6),
                 H::getData(machine, parameters, 7));
  }

  virtual void operator()(RLMachine&, typename A::type,typename B::type, typename C::type, 
                          typename D::type, typename E::type, typename F::type, typename G::type,
                          typename H::type) = 0;
};

// -----------------------------------------------------------------------

template<typename A, typename B, typename C, typename D, typename E, 
         typename F, typename G, typename H, typename I>
struct RLOp_Void_9 : public RLOp_NormalOperation<A, B, C, D, E, F, G, H, I>
{
  void dispatch(RLMachine& machine, 
                const boost::ptr_vector<libReallive::ExpressionPiece>& parameters) 
  {
      operator()(machine, A::getData(machine, parameters, 0),
                 B::getData(machine, parameters, 1),
                 C::getData(machine, parameters, 2),
                 D::getData(machine, parameters, 3),
                 E::getData(machine, parameters, 4),
                 F::getData(machine, parameters, 5),
                 G::getData(machine, parameters, 6),
                 H::getData(machine, parameters, 7),
                 I::getData(machine, parameters, 8));
  }

  virtual void operator()(RLMachine&, typename A::type,typename B::type, typename C::type, 
                          typename D::type, typename E::type, typename F::type, typename G::type,
                          typename H::type, typename I::type) = 0;
};

// -----------------------------------------------------------------------

template<typename A, typename B, typename C, typename D, typename E, 
         typename F, typename G, typename H, typename I, typename J>
struct RLOp_Void_10 : public RLOp_NormalOperation<A, B, C, D, E, F, G, H, I, J>
{
  void dispatch(RLMachine& machine, 
                const boost::ptr_vector<libReallive::ExpressionPiece>& parameters) 
  {
      operator()(machine, A::getData(machine, parameters, 0),
                 B::getData(machine, parameters, 1),
                 C::getData(machine, parameters, 2),
                 D::getData(machine, parameters, 3),
                 E::getData(machine, parameters, 4),
                 F::getData(machine, parameters, 5),
                 G::getData(machine, parameters, 6),
                 H::getData(machine, parameters, 7),
                 I::getData(machine, parameters, 8),
                 J::getData(machine, parameters, 9));
  }

  virtual void operator()(RLMachine&, typename A::type,typename B::type, typename C::type, 
                          typename D::type, typename E::type, typename F::type, typename G::type,
                          typename H::type, typename I::type, typename J::type) = 0;
};

// -----------------------------------------------------------------------

template<typename A, typename B, typename C, typename D, typename E, 
         typename F, typename G, typename H, typename I, typename J,
         typename K>
struct RLOp_Void_11 : public RLOp_NormalOperation<A, B, C, D, E, F, G, H, I, J, K>
{
  void dispatch(RLMachine& machine, 
                const boost::ptr_vector<libReallive::ExpressionPiece>& parameters) 
  {
      operator()(machine, A::getData(machine, parameters, 0),
                 B::getData(machine, parameters, 1),
                 C::getData(machine, parameters, 2),
                 D::getData(machine, parameters, 3),
                 E::getData(machine, parameters, 4),
                 F::getData(machine, parameters, 5),
                 G::getData(machine, parameters, 6),
                 H::getData(machine, parameters, 7),
                 I::getData(machine, parameters, 8),
                 J::getData(machine, parameters, 9),
                 K::getData(machine, parameters, 10));
  }

  virtual void operator()(RLMachine&, typename A::type,typename B::type, typename C::type, 
                          typename D::type, typename E::type, typename F::type, typename G::type,
                          typename H::type, typename I::type, typename J::type, typename K::type) = 0;
};

// -----------------------------------------------------------------------

template<typename A, typename B, typename C, typename D, typename E, 
         typename F, typename G, typename H, typename I, typename J,
         typename K, typename L>
struct RLOp_Void_12 : public RLOp_NormalOperation<A, B, C, D, E, F, G, H, I, J, K, L>
{
  void dispatch(RLMachine& machine, 
                const boost::ptr_vector<libReallive::ExpressionPiece>& parameters) 
  {
      operator()(machine, A::getData(machine, parameters, 0),
                 B::getData(machine, parameters, 1),
                 C::getData(machine, parameters, 2),
                 D::getData(machine, parameters, 3),
                 E::getData(machine, parameters, 4),
                 F::getData(machine, parameters, 5),
                 G::getData(machine, parameters, 6),
                 H::getData(machine, parameters, 7),
                 I::getData(machine, parameters, 8),
                 J::getData(machine, parameters, 9),
                 K::getData(machine, parameters, 10),
                 L::getData(machine, parameters, 11));
  }

  virtual void operator()(RLMachine&, typename A::type,typename B::type, typename C::type, 
                          typename D::type, typename E::type, typename F::type, typename G::type,
                          typename H::type, typename I::type, typename J::type, typename K::type,
                          typename L::type) = 0;
};

// -----------------------------------------------------------------------

template<typename A, typename B, typename C, typename D, typename E, 
         typename F, typename G, typename H, typename I, typename J,
         typename K, typename L, typename M>
struct RLOp_Void_13 : public RLOp_NormalOperation<A, B, C, D, E, F, G, H, I, J, K, L, M>
{
  void dispatch(RLMachine& machine, 
                const boost::ptr_vector<libReallive::ExpressionPiece>& parameters) 
  {
      operator()(machine, A::getData(machine, parameters, 0),
                 B::getData(machine, parameters, 1),
                 C::getData(machine, parameters, 2),
                 D::getData(machine, parameters, 3),
                 E::getData(machine, parameters, 4),
                 F::getData(machine, parameters, 5),
                 G::getData(machine, parameters, 6),
                 H::getData(machine, parameters, 7),
                 I::getData(machine, parameters, 8),
                 J::getData(machine, parameters, 9),
                 K::getData(machine, parameters, 10),
                 L::getData(machine, parameters, 11),
                 M::getData(machine, parameters, 12));
  }

  virtual void operator()(RLMachine&, typename A::type,typename B::type, typename C::type, 
                          typename D::type, typename E::type, typename F::type, typename G::type,
                          typename H::type, typename I::type, typename J::type, typename K::type,
                          typename L::type, typename M::type) = 0;
};

// -----------------------------------------------------------------------

template<typename A, typename B, typename C, typename D, typename E, 
         typename F, typename G, typename H, typename I, typename J,
         typename K, typename L, typename M, typename N>
struct RLOp_Void_14 : public RLOp_NormalOperation<A, B, C, D, E, F, G, H, I, J, K,
                                                  L, M, N>
{
  void dispatch(RLMachine& machine, 
                const boost::ptr_vector<libReallive::ExpressionPiece>& parameters) 
  {
      operator()(machine, A::getData(machine, parameters, 0),
                 B::getData(machine, parameters, 1),
                 C::getData(machine, parameters, 2),
                 D::getData(machine, parameters, 3),
                 E::getData(machine, parameters, 4),
                 F::getData(machine, parameters, 5),
                 G::getData(machine, parameters, 6),
                 H::getData(machine, parameters, 7),
                 I::getData(machine, parameters, 8),
                 J::getData(machine, parameters, 9),
                 K::getData(machine, parameters, 10),
                 L::getData(machine, parameters, 11),
                 M::getData(machine, parameters, 12),
                 N::getData(machine, parameters, 13));
  }

  virtual void operator()(RLMachine&, typename A::type,typename B::type, typename C::type, 
                          typename D::type, typename E::type, typename F::type, typename G::type,
                          typename H::type, typename I::type, typename J::type, typename K::type,
                          typename L::type, typename M::type, typename N::type) = 0;
};

// -----------------------------------------------------------------------


template<typename A, typename B, typename C, typename D, typename E, 
         typename F, typename G, typename H, typename I, typename J,
         typename K, typename L, typename M, typename N, typename O>
struct RLOp_Void_15 : public RLOp_NormalOperation<A, B, C, D, E, F, G, H, I, J, K,
                                                  L, M, N, O>
{
  void dispatch(RLMachine& machine, 
                const boost::ptr_vector<libReallive::ExpressionPiece>& parameters) 
  {
      operator()(machine, A::getData(machine, parameters, 0),
                 B::getData(machine, parameters, 1),
                 C::getData(machine, parameters, 2),
                 D::getData(machine, parameters, 3),
                 E::getData(machine, parameters, 4),
                 F::getData(machine, parameters, 5),
                 G::getData(machine, parameters, 6),
                 H::getData(machine, parameters, 7),
                 I::getData(machine, parameters, 8),
                 J::getData(machine, parameters, 9),
                 K::getData(machine, parameters, 10),
                 L::getData(machine, parameters, 11),
                 M::getData(machine, parameters, 12),
                 N::getData(machine, parameters, 13),
                 O::getData(machine, parameters, 14));
  }

  virtual void operator()(RLMachine&, typename A::type,typename B::type, typename C::type, 
                          typename D::type, typename E::type, typename F::type, typename G::type,
                          typename H::type, typename I::type, typename J::type, typename K::type,
                          typename L::type, typename M::type, typename N::type,
                          typename O::type) = 0;
};

// -----------------------------------------------------------------------

template<typename A, typename B, typename C, typename D, typename E, 
         typename F, typename G, typename H, typename I, typename J,
         typename K, typename L, typename M, typename N, typename O,
         typename P>
struct RLOp_Void_16 : public RLOp_NormalOperation<A, B, C, D, E, F, G, H, I, J, K,
                                                  L, M, N, O, P>
{
  void dispatch(RLMachine& machine, 
                const boost::ptr_vector<libReallive::ExpressionPiece>& parameters) 
  {
      operator()(machine, A::getData(machine, parameters, 0),
                 B::getData(machine, parameters, 1),
                 C::getData(machine, parameters, 2),
                 D::getData(machine, parameters, 3),
                 E::getData(machine, parameters, 4),
                 F::getData(machine, parameters, 5),
                 G::getData(machine, parameters, 6),
                 H::getData(machine, parameters, 7),
                 I::getData(machine, parameters, 8),
                 J::getData(machine, parameters, 9),
                 K::getData(machine, parameters, 10),
                 L::getData(machine, parameters, 11),
                 M::getData(machine, parameters, 12),
                 N::getData(machine, parameters, 13),
                 O::getData(machine, parameters, 14),
                 P::getData(machine, parameters, 15));
  }

  virtual void operator()(RLMachine&, typename A::type,typename B::type, typename C::type, 
                          typename D::type, typename E::type, typename F::type, typename G::type,
                          typename H::type, typename I::type, typename J::type, typename K::type,
                          typename L::type, typename M::type, typename N::type,
                          typename O::type, typename P::type) = 0;
};

// -----------------------------------------------------------------------


template<typename A, typename B, typename C, typename D, typename E, 
         typename F, typename G, typename H, typename I, typename J,
         typename K, typename L, typename M, typename N, typename O,
         typename P, typename Q>
struct RLOp_Void_17 : public RLOp_NormalOperation<A, B, C, D, E, F, G, H, I, J, K,
                                                  L, M, N, O, P, Q>
{
  void dispatch(RLMachine& machine, 
                const boost::ptr_vector<libReallive::ExpressionPiece>& parameters) 
  {
      operator()(machine, A::getData(machine, parameters, 0),
                 B::getData(machine, parameters, 1),
                 C::getData(machine, parameters, 2),
                 D::getData(machine, parameters, 3),
                 E::getData(machine, parameters, 4),
                 F::getData(machine, parameters, 5),
                 G::getData(machine, parameters, 6),
                 H::getData(machine, parameters, 7),
                 I::getData(machine, parameters, 8),
                 J::getData(machine, parameters, 9),
                 K::getData(machine, parameters, 10),
                 L::getData(machine, parameters, 11),
                 M::getData(machine, parameters, 12),
                 N::getData(machine, parameters, 13),
                 O::getData(machine, parameters, 14),
                 P::getData(machine, parameters, 15),
                 Q::getData(machine, parameters, 16));
  }

  virtual void operator()(RLMachine&, typename A::type,typename B::type, typename C::type, 
                          typename D::type, typename E::type, typename F::type, typename G::type,
                          typename H::type, typename I::type, typename J::type, typename K::type,
                          typename L::type, typename M::type, typename N::type,
                          typename O::type, typename P::type, typename Q::type) = 0;
};

// -----------------------------------------------------------------------

template<typename A, typename B, typename C, typename D, typename E, 
         typename F, typename G, typename H, typename I, typename J,
         typename K, typename L, typename M, typename N, typename O,
         typename P, typename Q, typename R>
struct RLOp_Void_18 : public RLOp_NormalOperation<A, B, C, D, E, F, G, H, I, J, K,
                                                  L, M, N, O, P, Q, R>
{
  void dispatch(RLMachine& machine, 
                const boost::ptr_vector<libReallive::ExpressionPiece>& parameters) 
  {
      operator()(machine, A::getData(machine, parameters, 0),
                 B::getData(machine, parameters, 1),
                 C::getData(machine, parameters, 2),
                 D::getData(machine, parameters, 3),
                 E::getData(machine, parameters, 4),
                 F::getData(machine, parameters, 5),
                 G::getData(machine, parameters, 6),
                 H::getData(machine, parameters, 7),
                 I::getData(machine, parameters, 8),
                 J::getData(machine, parameters, 9),
                 K::getData(machine, parameters, 10),
                 L::getData(machine, parameters, 11),
                 M::getData(machine, parameters, 12),
                 N::getData(machine, parameters, 13),
                 O::getData(machine, parameters, 14),
                 P::getData(machine, parameters, 15),
                 Q::getData(machine, parameters, 16),
                 R::getData(machine, parameters, 17));
  }

  virtual void operator()(RLMachine&, typename A::type,typename B::type, typename C::type, 
                          typename D::type, typename E::type, typename F::type, typename G::type,
                          typename H::type, typename I::type, typename J::type, typename K::type,
                          typename L::type, typename M::type, typename N::type,
                          typename O::type, typename P::type, typename Q::type,
                          typename R::type) = 0;
};

// -----------------------------------------------------------------------

template<typename A, typename B, typename C, typename D, typename E, 
         typename F, typename G, typename H, typename I, typename J,
         typename K, typename L, typename M, typename N, typename O,
         typename P, typename Q, typename R, typename S>
struct RLOp_Void_19 : public RLOp_NormalOperation<A, B, C, D, E, F, G, H, I, J, K,
                                                  L, M, N, O, P, Q, R, S>
{
  void dispatch(RLMachine& machine, 
                const boost::ptr_vector<libReallive::ExpressionPiece>& parameters) 
  {
      operator()(machine, A::getData(machine, parameters, 0),
                 B::getData(machine, parameters, 1),
                 C::getData(machine, parameters, 2),
                 D::getData(machine, parameters, 3),
                 E::getData(machine, parameters, 4),
                 F::getData(machine, parameters, 5),
                 G::getData(machine, parameters, 6),
                 H::getData(machine, parameters, 7),
                 I::getData(machine, parameters, 8),
                 J::getData(machine, parameters, 9),
                 K::getData(machine, parameters, 10),
                 L::getData(machine, parameters, 11),
                 M::getData(machine, parameters, 12),
                 N::getData(machine, parameters, 13),
                 O::getData(machine, parameters, 14),
                 P::getData(machine, parameters, 15),
                 Q::getData(machine, parameters, 16),
                 R::getData(machine, parameters, 17),
                 S::getData(machine, parameters, 18));
  }

  virtual void operator()(RLMachine&, typename A::type,typename B::type, typename C::type, 
                          typename D::type, typename E::type, typename F::type, typename G::type,
                          typename H::type, typename I::type, typename J::type, typename K::type,
                          typename L::type, typename M::type, typename N::type,
                          typename O::type, typename P::type, typename Q::type,
                          typename R::type, typename S::type) = 0;
};

// -----------------------------------------------------------------------

template<typename A, typename B, typename C, typename D, typename E, 
         typename F, typename G, typename H, typename I, typename J,
         typename K, typename L, typename M, typename N, typename O,
         typename P, typename Q, typename R, typename S, typename T>
struct RLOp_Void_20 : public RLOp_NormalOperation<A, B, C, D, E, F, G, H, I, J, K,
                                                  L, M, N, O, P, Q, R, S, T>
{
  void dispatch(RLMachine& machine, 
                const boost::ptr_vector<libReallive::ExpressionPiece>& parameters) 
  {
      operator()(machine, A::getData(machine, parameters, 0),
                 B::getData(machine, parameters, 1),
                 C::getData(machine, parameters, 2),
                 D::getData(machine, parameters, 3),
                 E::getData(machine, parameters, 4),
                 F::getData(machine, parameters, 5),
                 G::getData(machine, parameters, 6),
                 H::getData(machine, parameters, 7),
                 I::getData(machine, parameters, 8),
                 J::getData(machine, parameters, 9),
                 K::getData(machine, parameters, 10),
                 L::getData(machine, parameters, 11),
                 M::getData(machine, parameters, 12),
                 N::getData(machine, parameters, 13),
                 O::getData(machine, parameters, 14),
                 P::getData(machine, parameters, 15),
                 Q::getData(machine, parameters, 16),
                 R::getData(machine, parameters, 17),
                 S::getData(machine, parameters, 18),
                 T::getData(machine, parameters, 19));
  }

  virtual void operator()(RLMachine&, typename A::type,typename B::type, typename C::type, 
                          typename D::type, typename E::type, typename F::type, typename G::type,
                          typename H::type, typename I::type, typename J::type, typename K::type,
                          typename L::type, typename M::type, typename N::type,
                          typename O::type, typename P::type, typename Q::type,
                          typename R::type, typename S::type, typename T::type) = 0;
};

// -----------------------------------------------------------------------

template<typename A, typename B, typename C, typename D, typename E, 
         typename F, typename G, typename H, typename I, typename J,
         typename K, typename L, typename M, typename N, typename O,
         typename P, typename Q, typename R, typename S, typename T,
         typename U>
struct RLOp_Void_21 : public RLOp_NormalOperation<A, B, C, D, E, F, G, H, I, J, K,
                                                  L, M, N, O, P, Q, R, S, T, U>
{
  void dispatch(RLMachine& machine, 
                const boost::ptr_vector<libReallive::ExpressionPiece>& parameters) 
  {
      operator()(machine, A::getData(machine, parameters, 0),
                 B::getData(machine, parameters, 1),
                 C::getData(machine, parameters, 2),
                 D::getData(machine, parameters, 3),
                 E::getData(machine, parameters, 4),
                 F::getData(machine, parameters, 5),
                 G::getData(machine, parameters, 6),
                 H::getData(machine, parameters, 7),
                 I::getData(machine, parameters, 8),
                 J::getData(machine, parameters, 9),
                 K::getData(machine, parameters, 10),
                 L::getData(machine, parameters, 11),
                 M::getData(machine, parameters, 12),
                 N::getData(machine, parameters, 13),
                 O::getData(machine, parameters, 14),
                 P::getData(machine, parameters, 15),
                 Q::getData(machine, parameters, 16),
                 R::getData(machine, parameters, 17),
                 S::getData(machine, parameters, 18),
                 T::getData(machine, parameters, 19),
                 U::getData(machine, parameters, 20));
  }

  virtual void operator()(RLMachine&, typename A::type,typename B::type, typename C::type, 
                          typename D::type, typename E::type, typename F::type, typename G::type,
                          typename H::type, typename I::type, typename J::type, typename K::type,
                          typename L::type, typename M::type, typename N::type,
                          typename O::type, typename P::type, typename Q::type,
                          typename R::type, typename S::type, typename T::type,
                          typename U::type) = 0;
};

// -----------------------------------------------------------------------

template<typename A, typename B, typename C, typename D, typename E, 
         typename F, typename G, typename H, typename I, typename J,
         typename K, typename L, typename M, typename N, typename O,
         typename P, typename Q, typename R, typename S, typename T,
         typename U, typename V>
struct RLOp_Void_22 : public RLOp_NormalOperation<A, B, C, D, E, F, G, H, I, J, K,
                                                  L, M, N, O, P, Q, R, S, T, U, V>
{
  void dispatch(RLMachine& machine, 
                const boost::ptr_vector<libReallive::ExpressionPiece>& parameters) 
  {
      operator()(machine, A::getData(machine, parameters, 0),
                 B::getData(machine, parameters, 1),
                 C::getData(machine, parameters, 2),
                 D::getData(machine, parameters, 3),
                 E::getData(machine, parameters, 4),
                 F::getData(machine, parameters, 5),
                 G::getData(machine, parameters, 6),
                 H::getData(machine, parameters, 7),
                 I::getData(machine, parameters, 8),
                 J::getData(machine, parameters, 9),
                 K::getData(machine, parameters, 10),
                 L::getData(machine, parameters, 11),
                 M::getData(machine, parameters, 12),
                 N::getData(machine, parameters, 13),
                 O::getData(machine, parameters, 14),
                 P::getData(machine, parameters, 15),
                 Q::getData(machine, parameters, 16),
                 R::getData(machine, parameters, 17),
                 S::getData(machine, parameters, 18),
                 T::getData(machine, parameters, 19),
                 U::getData(machine, parameters, 20),
                 V::getData(machine, parameters, 21));
  }

  virtual void operator()(RLMachine&, typename A::type,typename B::type, typename C::type, 
                          typename D::type, typename E::type, typename F::type, typename G::type,
                          typename H::type, typename I::type, typename J::type, typename K::type,
                          typename L::type, typename M::type, typename N::type,
                          typename O::type, typename P::type, typename Q::type,
                          typename R::type, typename S::type, typename T::type,
                          typename U::type, typename V::type) = 0;
};

// -----------------------------------------------------------------------

template<typename A, typename B, typename C, typename D, typename E, 
         typename F, typename G, typename H, typename I, typename J,
         typename K, typename L, typename M, typename N, typename O,
         typename P, typename Q, typename R, typename S, typename T,
         typename U, typename V, typename W>
struct RLOp_Void_23 : public RLOp_NormalOperation<A, B, C, D, E, F, G, H, I, J, K,
                                                  L, M, N, O, P, Q, R, S, T, U, V,
                                                  W>
{
  void dispatch(RLMachine& machine, 
                const boost::ptr_vector<libReallive::ExpressionPiece>& parameters) 
  {
      operator()(machine, A::getData(machine, parameters, 0),
                 B::getData(machine, parameters, 1),
                 C::getData(machine, parameters, 2),
                 D::getData(machine, parameters, 3),
                 E::getData(machine, parameters, 4),
                 F::getData(machine, parameters, 5),
                 G::getData(machine, parameters, 6),
                 H::getData(machine, parameters, 7),
                 I::getData(machine, parameters, 8),
                 J::getData(machine, parameters, 9),
                 K::getData(machine, parameters, 10),
                 L::getData(machine, parameters, 11),
                 M::getData(machine, parameters, 12),
                 N::getData(machine, parameters, 13),
                 O::getData(machine, parameters, 14),
                 P::getData(machine, parameters, 15),
                 Q::getData(machine, parameters, 16),
                 R::getData(machine, parameters, 17),
                 S::getData(machine, parameters, 18),
                 T::getData(machine, parameters, 19),
                 U::getData(machine, parameters, 20),
                 V::getData(machine, parameters, 21),
                 W::getData(machine, parameters, 22));
  }

  virtual void operator()(RLMachine&, typename A::type,typename B::type, typename C::type, 
                          typename D::type, typename E::type, typename F::type, typename G::type,
                          typename H::type, typename I::type, typename J::type, typename K::type,
                          typename L::type, typename M::type, typename N::type,
                          typename O::type, typename P::type, typename Q::type,
                          typename R::type, typename S::type, typename T::type,
                          typename U::type, typename V::type, 
                          typename W::type) = 0;
};

// -----------------------------------------------------------------------

template<typename A, typename B, typename C, typename D, typename E, 
         typename F, typename G, typename H, typename I, typename J,
         typename K, typename L, typename M, typename N, typename O,
         typename P, typename Q, typename R, typename S, typename T,
         typename U, typename V, typename W, typename X>
struct RLOp_Void_24 : public RLOp_NormalOperation<A, B, C, D, E, F, G, H, I, J, K,
                                                  L, M, N, O, P, Q, R, S, T, U, V,
                                                  W, X>
{
  void dispatch(RLMachine& machine, 
                const boost::ptr_vector<libReallive::ExpressionPiece>& parameters) 
  {
      operator()(machine, A::getData(machine, parameters, 0),
                 B::getData(machine, parameters, 1),
                 C::getData(machine, parameters, 2),
                 D::getData(machine, parameters, 3),
                 E::getData(machine, parameters, 4),
                 F::getData(machine, parameters, 5),
                 G::getData(machine, parameters, 6),
                 H::getData(machine, parameters, 7),
                 I::getData(machine, parameters, 8),
                 J::getData(machine, parameters, 9),
                 K::getData(machine, parameters, 10),
                 L::getData(machine, parameters, 11),
                 M::getData(machine, parameters, 12),
                 N::getData(machine, parameters, 13),
                 O::getData(machine, parameters, 14),
                 P::getData(machine, parameters, 15),
                 Q::getData(machine, parameters, 16),
                 R::getData(machine, parameters, 17),
                 S::getData(machine, parameters, 18),
                 T::getData(machine, parameters, 19),
                 U::getData(machine, parameters, 20),
                 V::getData(machine, parameters, 21),
                 W::getData(machine, parameters, 22),
                 X::getData(machine, parameters, 23));
  }

  virtual void operator()(RLMachine&, typename A::type,typename B::type, typename C::type, 
                          typename D::type, typename E::type, typename F::type, typename G::type,
                          typename H::type, typename I::type, typename J::type, typename K::type,
                          typename L::type, typename M::type, typename N::type,
                          typename O::type, typename P::type, typename Q::type,
                          typename R::type, typename S::type, typename T::type,
                          typename U::type, typename V::type, 
                          typename W::type, typename X::type) = 0;
};

// -----------------------------------------------------------------------

template<typename A, typename B, typename C, typename D, typename E, 
         typename F, typename G, typename H, typename I, typename J,
         typename K, typename L, typename M, typename N, typename O,
         typename P, typename Q, typename R, typename S, typename T,
         typename U, typename V, typename W, typename X, typename Y>
struct RLOp_Void_25 : public RLOp_NormalOperation<A, B, C, D, E, F, G, H, I, J, K,
                                                  L, M, N, O, P, Q, R, S, T, U, V,
                                                  W, X, Y>
{
  void dispatch(RLMachine& machine, 
                const boost::ptr_vector<libReallive::ExpressionPiece>& parameters) 
  {
      operator()(machine, A::getData(machine, parameters, 0),
                 B::getData(machine, parameters, 1),
                 C::getData(machine, parameters, 2),
                 D::getData(machine, parameters, 3),
                 E::getData(machine, parameters, 4),
                 F::getData(machine, parameters, 5),
                 G::getData(machine, parameters, 6),
                 H::getData(machine, parameters, 7),
                 I::getData(machine, parameters, 8),
                 J::getData(machine, parameters, 9),
                 K::getData(machine, parameters, 10),
                 L::getData(machine, parameters, 11),
                 M::getData(machine, parameters, 12),
                 N::getData(machine, parameters, 13),
                 O::getData(machine, parameters, 14),
                 P::getData(machine, parameters, 15),
                 Q::getData(machine, parameters, 16),
                 R::getData(machine, parameters, 17),
                 S::getData(machine, parameters, 18),
                 T::getData(machine, parameters, 19),
                 U::getData(machine, parameters, 20),
                 V::getData(machine, parameters, 21),
                 W::getData(machine, parameters, 22),
                 X::getData(machine, parameters, 23),
                 Y::getData(machine, parameters, 24));
  }

  virtual void operator()(RLMachine&, typename A::type,typename B::type, typename C::type, 
                          typename D::type, typename E::type, typename F::type, typename G::type,
                          typename H::type, typename I::type, typename J::type, typename K::type,
                          typename L::type, typename M::type, typename N::type,
                          typename O::type, typename P::type, typename Q::type,
                          typename R::type, typename S::type, typename T::type,
                          typename U::type, typename V::type, 
                          typename W::type, typename X::type,
                          typename Y::type) = 0;
};

// -----------------------------------------------------------------------

template<typename A, typename B, typename C, typename D, typename E, 
         typename F, typename G, typename H, typename I, typename J,
         typename K, typename L, typename M, typename N, typename O,
         typename P, typename Q, typename R, typename S, typename T,
         typename U, typename V, typename W, typename X, typename Y,
         typename Z>
struct RLOp_Void_26 : public RLOp_NormalOperation<A, B, C, D, E, F, G, H, I, J, K,
                                                  L, M, N, O, P, Q, R, S, T, U, V,
                                                  W, X, Y, Z>
{
  void dispatch(RLMachine& machine, 
                const boost::ptr_vector<libReallive::ExpressionPiece>& parameters) 
  {
      operator()(machine, A::getData(machine, parameters, 0),
                 B::getData(machine, parameters, 1),
                 C::getData(machine, parameters, 2),
                 D::getData(machine, parameters, 3),
                 E::getData(machine, parameters, 4),
                 F::getData(machine, parameters, 5),
                 G::getData(machine, parameters, 6),
                 H::getData(machine, parameters, 7),
                 I::getData(machine, parameters, 8),
                 J::getData(machine, parameters, 9),
                 K::getData(machine, parameters, 10),
                 L::getData(machine, parameters, 11),
                 M::getData(machine, parameters, 12),
                 N::getData(machine, parameters, 13),
                 O::getData(machine, parameters, 14),
                 P::getData(machine, parameters, 15),
                 Q::getData(machine, parameters, 16),
                 R::getData(machine, parameters, 17),
                 S::getData(machine, parameters, 18),
                 T::getData(machine, parameters, 19),
                 U::getData(machine, parameters, 20),
                 V::getData(machine, parameters, 21),
                 W::getData(machine, parameters, 22),
                 X::getData(machine, parameters, 23),
                 Y::getData(machine, parameters, 24),
                 Z::getData(machine, parameters, 25));
  }

  virtual void operator()(RLMachine&, typename A::type,typename B::type, typename C::type, 
                          typename D::type, typename E::type, typename F::type, typename G::type,
                          typename H::type, typename I::type, typename J::type, typename K::type,
                          typename L::type, typename M::type, typename N::type,
                          typename O::type, typename P::type, typename Q::type,
                          typename R::type, typename S::type, typename T::type,
                          typename U::type, typename V::type, 
                          typename W::type, typename X::type,
                          typename Y::type, typename Z::type) = 0;
};

// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------



// -----------------------------------------------------------------------
// @}

#endif
