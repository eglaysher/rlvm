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
  StoreRegisterExpressionPiece();
  virtual ~StoreRegisterExpressionPiece();

  // Overridden from ExpressionPiece:
  virtual bool IsMemoryReference() const final;
  virtual void SetIntegerValue(RLMachine& machine, int rvalue) final;
  virtual int GetIntegerValue(RLMachine& machine) const final;
  virtual std::string GetSerializedExpression(
      RLMachine& machine) const final;
  virtual std::string GetDebugString() const final;
  // Never seen in any commercial games, but needed for rlBabel support.
  virtual IntReferenceIterator GetIntegerReferenceIterator(
      RLMachine& machine) const final;
  virtual std::unique_ptr<ExpressionPiece> Clone() const final;
};

// Represents a constant integer in an Expression.
class IntegerConstant : public ExpressionPiece {
 public:
  explicit IntegerConstant(const int in);
  virtual ~IntegerConstant();

  // Overridden from ExpressionPiece:
  virtual int GetIntegerValue(RLMachine& machine) const final;
  virtual std::string GetSerializedExpression(
      RLMachine& machine) const final;
  virtual std::string GetDebugString() const final;
  virtual std::unique_ptr<ExpressionPiece> Clone() const final;

 private:
  // The value of this constant
  int constant;
};

// -----------------------------------------------------------------------

class StringConstant : public ExpressionPiece {
 public:
  explicit StringConstant(const std::string& inStr);
  virtual ~StringConstant();

  // Overridden from ExpressionPiece:
  virtual ExpressionValueType GetExpressionValueType() const final;
  virtual const std::string& GetStringValue(RLMachine& machine) const final;
  virtual std::string GetSerializedExpression(
      RLMachine& machine) const final;
  virtual std::string GetDebugString() const final;
  virtual std::unique_ptr<ExpressionPiece> Clone() const final;

 private:
  std::string constant;
};

// -----------------------------------------------------------------------

// Reference to a piece of memory in an RLMachine. Noe that this
class MemoryReference : public ExpressionPiece {
 public:
  MemoryReference(int type, std::unique_ptr<ExpressionPiece> inLoc);
  virtual ~MemoryReference();

  // Overridden from ExpressionPiece:
  virtual bool IsMemoryReference() const final;
  virtual ExpressionValueType GetExpressionValueType() const final;

  virtual void SetIntegerValue(RLMachine& machine, int rvalue) final;
  virtual int GetIntegerValue(RLMachine& machine) const final;

  virtual void SetStringValue(RLMachine& machine,
                              const std::string& rvalue) final;
  virtual const std::string& GetStringValue(RLMachine& machine) const final;
  virtual std::string GetSerializedExpression(
      RLMachine& machine) const final;
  virtual std::string GetDebugString() const final;
  virtual IntReferenceIterator GetIntegerReferenceIterator(
      RLMachine& machine) const final;
  virtual StringReferenceIterator GetStringReferenceIterator(
      RLMachine& machine) const final;
  virtual std::unique_ptr<ExpressionPiece> Clone() const final;

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
  virtual ~UniaryExpressionOperator();

  // Overridden from ExpressionPiece:
  virtual int GetIntegerValue(RLMachine& machine) const final;
  virtual std::string GetSerializedExpression(
      RLMachine& machine) const final;
  virtual std::string GetDebugString() const final;
  virtual std::unique_ptr<ExpressionPiece> Clone() const final;

 private:
  // Performs operation on the passed in parameter, and returns the
  // value.
  int PerformOperationOn(int x) const;

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
  virtual ~BinaryExpressionOperator();

  // Overridden from ExpressionPiece:
  virtual int GetIntegerValue(RLMachine& machine) const override;
  virtual std::string GetSerializedExpression(
      RLMachine& machine) const final;
  virtual std::string GetDebugString() const final;
  virtual std::unique_ptr<ExpressionPiece> Clone() const override;

 protected:
  // Performs operation on the two passed in operands.
  int PerformOperationOn(int lhs, int rhs) const;

  // The operation to perform
  char operation;

  // The left operand for this expression
  std::unique_ptr<ExpressionPiece> leftOperand;

  // The right operand for this expression
  std::unique_ptr<ExpressionPiece> rightOperand;
};

// ----------------------------------------------------------------------

// Operation that modies a given memory location, sucha as +=, -=, /=,
// et cetera.
class AssignmentExpressionOperator : public BinaryExpressionOperator {
 public:
  AssignmentExpressionOperator(char operation,
                               std::unique_ptr<ExpressionPiece> lhs,
                               std::unique_ptr<ExpressionPiece> rhs);
  virtual ~AssignmentExpressionOperator();

  virtual bool IsAssignment() const final;

  // For the entire assignment operator hiearchy, we use GetIntegerValue,
  // since it acts as the execute.
  virtual int GetIntegerValue(RLMachine& machine) const final;

  // Deliberately has no GetSerializedExpression() implementation; uses
  // BinaryExpressionOperator's.

  virtual std::unique_ptr<ExpressionPiece> Clone() const final;
};


// -----------------------------------------------------------------------

// Represents a Complex parameter to a function. This ExpressionPiece
// contains multiple ExpressionPieces.
class ComplexExpressionPiece : public ExpressionPiece {
 public:
  ComplexExpressionPiece();
  virtual ~ComplexExpressionPiece();

  // Adds an ExpressionPiece to this complex expressionPiece. This
  // instance of ComplexExpressionPiece takes ownership of any
  // ExpressionPiece passed in this way.
  void AddContainedPiece(std::unique_ptr<ExpressionPiece> piece);

  const std::vector<std::unique_ptr<ExpressionPiece>>& contained_pieces() const
  { return contained_pieces_; }

  // Overridden from ExpressionPiece:
  virtual bool IsComplexParameter() const override;
  virtual std::string GetSerializedExpression(
      RLMachine& machine) const override;
  virtual std::string GetDebugString() const override;
  virtual std::unique_ptr<ExpressionPiece> Clone() const override;

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
  virtual ~SpecialExpressionPiece();

  int overload_tag() const { return overload_tag_; }

  // Overridden from ExpressionPiece:
  virtual bool IsSpecialParameter() const final;
  virtual std::string GetSerializedExpression(
      RLMachine& machine) const final;
  virtual std::string GetDebugString() const final;
  virtual std::unique_ptr<ExpressionPiece> Clone() const final;

 private:
  int overload_tag_;
};

}  // namespace libreallive

#endif  // SRC_LIBREALLIVE_EXPRESSION_PIECES_H_
