// This file is part of libreallive, a dependency of RLVM.
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

#ifndef SRC_LIBREALLIVE_EXPRESSION_PIECES_H_
#define SRC_LIBREALLIVE_EXPRESSION_PIECES_H_

#include <string>
#include <vector>

#include "libreallive/expression.h"
#include "machine/reference.h"

namespace libreallive {

// Represents the store register in an Expression (both as an lvalue
// and rvlaue).
class StoreRegisterExpressionPiece : public ExpressionPiece {
 public:
  // StoreRegisterExpressionPiece is a memory reference; returns true
  virtual bool isMemoryReference() const;

  // Assign the incoming value to the store register of the passed in
  // machine.
  virtual void assignIntValue(RLMachine& machine, int rvalue);

  // Returns the store register value of the passed in machine
  virtual int integerValue(RLMachine& machine) const;

  virtual std::string serializedValue(RLMachine& machine) const;
  virtual std::string getDebugValue(RLMachine& machine) const;
  virtual std::string getDebugString() const;

  // Never seen in any commercial games, but needed for rlBabel support.
  virtual IntReferenceIterator getIntegerReferenceIterator(
      RLMachine& machine) const;

  virtual std::unique_ptr<ExpressionPiece> clone() const;
};

// Represents a constant integer in an Expression.
class IntegerConstant : public ExpressionPiece {
 public:
  explicit IntegerConstant(const int in);
  ~IntegerConstant();

  // Returns the constant value
  virtual int integerValue(RLMachine& machine) const;
  virtual std::string serializedValue(RLMachine& machine) const;
  virtual std::string getDebugValue(RLMachine& machine) const;
  virtual std::string getDebugString() const;
  virtual std::unique_ptr<ExpressionPiece> clone() const;

 private:
  // The value of this constant
  int constant;
};

// -----------------------------------------------------------------------

class StringConstant : public ExpressionPiece {
 public:
  explicit StringConstant(const std::string& inStr);

  virtual ExpressionValueType expressionValueType() const;
  virtual const std::string& getStringValue(RLMachine& machine) const;
  virtual std::string serializedValue(RLMachine& machine) const;
  virtual std::string getDebugValue(RLMachine& machine) const;
  virtual std::string getDebugString() const;

  virtual std::unique_ptr<ExpressionPiece> clone() const;

 private:
  std::string constant;
};

// -----------------------------------------------------------------------

// Reference to a piece of memory in an RLMachine. Noe that this
class MemoryReference : public ExpressionPiece {
 public:
  MemoryReference(int type, std::unique_ptr<ExpressionPiece> inLoc);
  ~MemoryReference();

  virtual bool isMemoryReference() const;
  virtual ExpressionValueType expressionValueType() const;

  virtual void assignIntValue(RLMachine& machine, int rvalue);
  virtual int integerValue(RLMachine& machine) const;

  virtual void assignStringValue(RLMachine& machine, const std::string& rvalue);
  virtual const std::string& getStringValue(RLMachine& machine) const;
  virtual std::string serializedValue(RLMachine& machine) const;
  virtual std::string getDebugValue(RLMachine& machine) const;
  virtual std::string getDebugString() const;

  virtual IntReferenceIterator getIntegerReferenceIterator(
      RLMachine& machine) const;
  virtual StringReferenceIterator getStringReferenceIterator(
      RLMachine& machine) const;

  virtual std::unique_ptr<ExpressionPiece> clone() const;

 private:
  // The type of an memory reference refers to both the memory
  // bank we're accessing, and how we're addressing it.
  int type;

  // Arbitrarily complex expression to evaluate at runtime to
  // determine the index of the location we want to address.
  std::unique_ptr<ExpressionPiece> location;
};

// ----------------------------------------------------------------------

// Represents an operation on one ExpressionPiece, i.e. unary
// negative, et cetera.
class UniaryExpressionOperator : public ExpressionPiece {
 public:
  UniaryExpressionOperator(char inOperation,
                           std::unique_ptr<ExpressionPiece> inOperand);
  ~UniaryExpressionOperator();
  virtual int integerValue(RLMachine& machine) const;
  virtual std::string serializedValue(RLMachine& machine) const;
  virtual std::string getDebugValue(RLMachine& machine) const;
  virtual std::string getDebugString() const;
  virtual std::unique_ptr<ExpressionPiece> clone() const;

 private:
  // Performs operation on the passed in parameter, and returns the
  // value.
  int performOperationOn(int x) const;

  // The sub-Expression to operate on
  std::unique_ptr<ExpressionPiece> operand;

  // Which operation we are to perform.
  char operation;
};

// ----------------------------------------------------------------------

class BinaryExpressionOperator : public ExpressionPiece {
 public:
  BinaryExpressionOperator(char inOperation,
                           std::unique_ptr<ExpressionPiece> lhs,
                           std::unique_ptr<ExpressionPiece> rhs);
  ~BinaryExpressionOperator();
  virtual int integerValue(RLMachine& machine) const;
  virtual std::string serializedValue(RLMachine& machine) const;
  virtual std::string getDebugValue(RLMachine& machine) const;
  virtual std::string getDebugString() const;

  virtual std::unique_ptr<ExpressionPiece> clone() const;

 protected:
  // The operation to perform
  char operation;

  // The left operand for this expression
  std::unique_ptr<ExpressionPiece> leftOperand;

  // The right operand for this expression
  std::unique_ptr<ExpressionPiece> rightOperand;

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
};

// ----------------------------------------------------------------------

// Operation that modies a given memory location, sucha as +=, -=, /=,
// et cetera.
class AssignmentExpressionOperator : public BinaryExpressionOperator {
 public:
  AssignmentExpressionOperator(char operation,
                               std::unique_ptr<ExpressionPiece> lhs,
                               std::unique_ptr<ExpressionPiece> rhs);
  ~AssignmentExpressionOperator();

  virtual bool isAssignment() const;

  // For the entire assignment operator hiearchy, we use integerValue,
  // since it acts as the execute.
  virtual int integerValue(RLMachine& machine) const;

  // Deliberately has no serializedValue() implementation; uses
  // BinaryExpressionOperator's.
  virtual std::string getDebugValue(RLMachine& machine) const;

  virtual std::unique_ptr<ExpressionPiece> clone() const;
};


// -----------------------------------------------------------------------

// Represents a Complex parameter to a function. This ExpressionPiece
// contains multiple ExpressionPieces.
class ComplexExpressionPiece : public ExpressionPiece {
 public:
  virtual bool isComplexParameter() const;

  // Adds an ExpressionPiece to this complex expressionPiece. This
  // instance of ComplexExpressionPiece takes ownership of any
  // ExpressionPiece passed in this way.
  void addContainedPiece(std::unique_ptr<ExpressionPiece> piece);

  const std::vector<std::unique_ptr<ExpressionPiece>>&
  getContainedPieces() const
    { return contained_pieces_; }
  virtual std::string serializedValue(RLMachine& machine) const;
  virtual std::string getDebugValue(RLMachine& machine) const;
  virtual std::string getDebugString() const;
  virtual std::unique_ptr<ExpressionPiece> clone() const;

 protected:
  // Poiter owning container of all the ExpressionPieces we compose
  std::vector<std::unique_ptr<ExpressionPiece>> contained_pieces_;
};

// -----------------------------------------------------------------------

// Represents a Special parameter; a type that can be multiple types.
//
// In both Haeleth's code and my study of the bytecode, Special
// parameters seem to be types of Complex expressions. Even if they
// weren't, we need to maintain this relationships to make Special_T
// work.
class SpecialExpressionPiece : public ComplexExpressionPiece {
 public:
  explicit SpecialExpressionPiece(int tag);
  virtual bool isSpecialParamater() const;

  int getOverloadTag() const { return overloadTag; }
  virtual std::string serializedValue(RLMachine& machine) const;
  virtual std::string getDebugValue(RLMachine& machine) const;
  virtual std::string getDebugString() const;
  virtual std::unique_ptr<ExpressionPiece> clone() const;

 private:
  int overloadTag;
};

}  // namespace libreallive

#endif  // SRC_LIBREALLIVE_EXPRESSION_PIECES_H_
