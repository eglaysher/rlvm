// This file is part of libReallive, a dependency of RLVM.
//
// -----------------------------------------------------------------------
//
// Copyright (c) 2006, 2007 Peter Jolly
// Copyright (c) 2007 Elliot Glaysher
//
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use, copy,
// modify, merge, publish, distribute, sublicense, and/or sell copies
// of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
// BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// -----------------------------------------------------------------------

#ifndef EXPRESSION_PIECES_H
#define EXPRESSION_PIECES_H

#include "libReallive/expression.h"
#include "MachineBase/reference.hpp"

namespace libReallive {

/**
 * Represents the store register in an Expression (both as an lvalue
 * and rvlaue).
 */
class StoreRegisterExpressionPiece : public ExpressionPiece {
public:
  /// StoreRegisterExpressionPiece is a memory reference; returns true
  virtual bool isMemoryReference() const;

  /// Assign the incoming value to the store register of the passed in
  /// machine.
  virtual void assignIntValue(RLMachine& machine, int rvalue);

  /// Returns the store register value of the passed in machine
  virtual int integerValue(RLMachine& machine) const;

  virtual ExpressionPiece* clone() const;
};

/**
 * Represents a constant integer in an Expression.
 */
class IntegerConstant : public ExpressionPiece {
private:
  /// The value of this constant
  int constant;
public:
  IntegerConstant(const int in);
  ~IntegerConstant();

  /// Returns the constant value
  virtual int integerValue(RLMachine& machine) const;

  virtual ExpressionPiece* clone() const;
};

// -----------------------------------------------------------------------

class StringConstant : public ExpressionPiece {
private:
  std::string constant;
public:
  StringConstant(const std::string& inStr);

  virtual ExpressionValueType expressionValueType() const;
  virtual const std::string& getStringValue(RLMachine& machine) const;
  virtual ExpressionPiece* clone() const;
};

// -----------------------------------------------------------------------

/**
 * Reference to a piece of memory in an RLMachine. Noe that this
 */
class MemoryReference : public ExpressionPiece {
private:
  /* The type of an memory reference refers to both the memory
   * bank we're accessing, and how we're addressing it.
   * @see RLMachine::getIntValue
   * @see RLMachine::getStringValue
   */
  int type;

  /** Arbitrarily complex expression to evaluate at runtime to
   * determine the index of the location we want to address.
   *
   * @see RLMachine::getIntValue
   * @see RLMachine::getStringValue
   */
  boost::scoped_ptr<ExpressionPiece> location;

public:
  MemoryReference(int type, ExpressionPiece* inLoc);
  ~MemoryReference();

  virtual bool isMemoryReference() const;
  virtual ExpressionValueType expressionValueType() const;

  virtual void assignIntValue(RLMachine& machine, int rvalue);
  virtual int integerValue(RLMachine& machine) const;

  virtual void assignStringValue(RLMachine& machine, const std::string& rvalue);
  virtual const std::string& getStringValue(RLMachine& machine) const;

  IntReferenceIterator getIntegerReferenceIterator(RLMachine& machine) const;
  StringReferenceIterator getStringReferenceIterator(RLMachine& machine) const;

  virtual ExpressionPiece* clone() const;
};

// ----------------------------------------------------------------------

/**
 * Represents an operation on one ExpressionPiece, i.e. unary
 * negative, et cetera.
 */
class UniaryExpressionOperator : public ExpressionPiece {
private:
  /// The sub-Expression to operate on
  boost::scoped_ptr<ExpressionPiece> operand;

  /// Which operation we are to perform.
  char operation;

  /**
   * Performs operation on the passed in parameter, and returns the
   * value.
   *
   * @param x Number to operate on.
   * @return The result of the operation
   */
  int performOperationOn(int x) const;

public:
  UniaryExpressionOperator(char inOperation, ExpressionPiece* inOperand);
  ~UniaryExpressionOperator();
  virtual int integerValue(RLMachine& machine) const;

  virtual ExpressionPiece* clone() const;
};

// ----------------------------------------------------------------------

class BinaryExpressionOperator : public ExpressionPiece {
protected:
  /// The operation to perform
  char operation;

  /// The left operand for this expression
  boost::scoped_ptr<ExpressionPiece> leftOperand;
  /// The right operand for this expression
  boost::scoped_ptr<ExpressionPiece> rightOperand;

  /**
   * Performs operation on the two passed in operands.
   *
   * @param lhs The left operand
   * @param rhs The right operand
   * @return The result of this operation.
   *
   * @note To anyone who says that creating a full object hiearchy of
   * all the operations would be more OO, I tried, and it's 20x more
   * misserable.
   */
  int performOperationOn(int lhs, int rhs) const;

public:
  BinaryExpressionOperator(char inOperation, ExpressionPiece* lhs,
                           ExpressionPiece* rhs);
  ~BinaryExpressionOperator();
  virtual int integerValue(RLMachine& machine) const;

  virtual ExpressionPiece* clone() const;
};

// ----------------------------------------------------------------------

/**
 * Operation that modies a given memory location, sucha as +=, -=, /=,
 * et cetera.
 */
class AssignmentExpressionOperator : public BinaryExpressionOperator {
public:
  AssignmentExpressionOperator(char operation, ExpressionPiece* lhs,
                               ExpressionPiece* rhs);
  ~AssignmentExpressionOperator();

  /** For the entire assignment operator hiearchy, we use integerValue,
   * since it acts as the execute.
   */
  virtual int integerValue(RLMachine& machine) const;

  virtual ExpressionPiece* clone() const;
};


// -----------------------------------------------------------------------

/**
 * Represents a Complex parameter to a function. This ExpressionPiece
 * contains multiple ExpressionPieces.
 */
class ComplexExpressionPiece : public ExpressionPiece {
protected:
  /// Poiter owning container of all the ExpressionPieces we compose
  boost::ptr_vector<ExpressionPiece> containedPieces;

public:
  virtual bool isComplexParameter() const;

  void addContainedPiece(ExpressionPiece* piece);

  const boost::ptr_vector<ExpressionPiece>& getContainedPieces() const
    { return containedPieces; }

  virtual ExpressionPiece* clone() const;
};

// -----------------------------------------------------------------------

/**
 * Represents a Special parameter; a type that can be multiple types.
 *
 * In both Haeleth's code and my study of the bytecode, Special
 * parameters seem to be types of Complex expressions. Even if they
 * weren't, we need to maintain this relationships to make Special_T
 * work.
 */
class SpecialExpressionPiece : public ComplexExpressionPiece {
private:
  int overloadTag;

public:
  SpecialExpressionPiece(int tag);
  virtual bool isSpecialParamater() const;

  int getOverloadTag() const { return overloadTag; }

  virtual ExpressionPiece* clone() const;
};

}

#endif
