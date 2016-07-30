// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of libreallive, a dependency of RLVM.
//
// -----------------------------------------------------------------------
//
// Copyright (c) 2006 Peter Jolly
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

#ifndef SRC_LIBREALLIVE_EXPRESSION_H_
#define SRC_LIBREALLIVE_EXPRESSION_H_

#include <memory>
#include <string>
#include <vector>

#include "machine/reference.h"

class RLMachine;

namespace libreallive {

// Size of expression functions
size_t NextToken(const char* src);
size_t NextExpression(const char* src);
size_t NextString(const char* src);
size_t NextData(const char* src);

// Parse expression functions
class ExpressionPiece;
ExpressionPiece GetExpressionToken(const char*& src);
ExpressionPiece GetExpressionTerm(const char*& src);
ExpressionPiece GetExpressionArithmatic(const char*& src);
ExpressionPiece GetExpressionCondition(const char*& src);
ExpressionPiece GetExpressionBoolean(const char*& src);
ExpressionPiece GetExpression(const char*& src);
ExpressionPiece GetAssignment(const char*& src);
ExpressionPiece GetData(const char*& src);
ExpressionPiece GetComplexParam(const char*& src);

std::string EvaluatePRINT(RLMachine& machine, const std::string& in);

// Converts a parameter string (as read from the binary SEEN.TXT file)
// into a human readable (and printable) format.
std::string ParsableToPrintableString(const std::string& src);

// Converts a printable string (i.e., "$ 05 [ $ FF EE 03 00 00 ]")
// into one that can be parsed by all the get_expr family of functions.
std::string PrintableToParsableString(const std::string& src);

enum ExpressionValueType {
  ValueTypeInteger,
  ValueTypeString
};

enum ExpressionPieceType {
  TYPE_STORE_REGISTER,
  TYPE_INT_CONSTANT,
  TYPE_STRING_CONSTANT,
  TYPE_MEMORY_REFERENCE,
  TYPE_SIMPLE_MEMORY_REFERENCE,
  TYPE_UNIARY_EXPRESSION,
  TYPE_BINARY_EXPRESSION,
  TYPE_SIMPLE_ASSIGNMENT,
  TYPE_COMPLEX_EXPRESSION,
  TYPE_SPECIAL_EXPRESSION,
  TYPE_INVALID
};

struct invalid_expression_piece_t {};

class ExpressionPiece {
 public:
  static ExpressionPiece StoreRegister();
  static ExpressionPiece IntConstant(const int constant);
  static ExpressionPiece StrConstant(const std::string constant);
  static ExpressionPiece MemoryReference(const int type,
                                         ExpressionPiece location);
  static ExpressionPiece UniaryExpression(const char operation,
                                          ExpressionPiece operand);
  static ExpressionPiece BinaryExpression(const char operation,
                                          ExpressionPiece lhs,
                                          ExpressionPiece rhs);
  static ExpressionPiece ComplexExpression();
  static ExpressionPiece SpecialExpression(const int tag);

  ExpressionPiece(invalid_expression_piece_t);
  ExpressionPiece(const ExpressionPiece& rhs);
  ExpressionPiece(ExpressionPiece&& rhs);
  ~ExpressionPiece();

  ExpressionPiece& operator=(const ExpressionPiece& rhs);
  ExpressionPiece& operator=(ExpressionPiece&& rhs);

  bool is_valid() const { return piece_type != TYPE_INVALID; }

  // Capability method; returns false by default. Override when
  // ExpressionPiece subclass accesses a piece of memory.
  bool IsMemoryReference() const;

  // Capability method; returns false by default. Override only in
  // classes that represent a complex parameter to the type system.
  // @see Complex_T
  bool IsComplexParameter() const;

  // Capability method; returns false by default. Override only in
  // classes that represent a special parameter to the type system.
  // @see Special_T
  bool IsSpecialParameter() const;

  // Returns the value type of this expression (i.e. string or
  // integer)
  ExpressionValueType GetExpressionValueType() const;

  // Assigns the value into the memory location represented by the
  // current expression. Not all ExpressionPieces can do this, so
  // there is a default implementation which does nothing.
  void SetIntegerValue(RLMachine& machine, int rvalue);

  // Returns the integer value of this expression; this can either be
  // a memory access or a calculation based on some subexpressions.
  int GetIntegerValue(RLMachine& machine) const;

  void SetStringValue(RLMachine& machine, const std::string& rvalue);
  const std::string& GetStringValue(RLMachine& machine) const;

  // I used to be able to just static cast any ExpressionPiece to a
  // MemoryReference if I wanted/needed a corresponding iterator. Haeleth's
  // rlBabel library instead uses the store register as an argument to a
  // function that takes a integer reference. So this needs to be here now.
  IntReferenceIterator GetIntegerReferenceIterator(RLMachine& machine) const;
  StringReferenceIterator GetStringReferenceIterator(RLMachine& machine) const;

  // A persistable version of this value. This method should return RealLive
  // bytecode equal to this ExpressionPiece with all references returned.
  std::string GetSerializedExpression(RLMachine& machine) const;

  // A printable representation of the expression itself. Used to dump our
  // parsing of the bytecode to the console.
  std::string GetDebugString() const;

  // In the case of Complex and Special types, adds an expression piece to the
  // list.
  void AddContainedPiece(ExpressionPiece piece);

  const std::vector<ExpressionPiece>& GetContainedPieces() const;

  int GetOverloadTag() const;

 private:
  ExpressionPiece();

  // Frees all possible memory and sets |piece_type| to TYPE_INVALID.
  void Invalidate();

  // Implementations of some of the public interface where they aren't one
  // liners.
  std::string GetComplexSerializedExpression(RLMachine& machine) const;
  std::string GetSpecialSerializedExpression(RLMachine& machine) const;

  std::string GetMemoryDebugString(int type,
                                   const std::string& location) const;
  std::string GetUniaryDebugString() const;
  std::string GetBinaryDebugString(char operation,
                                   const std::string& lhs,
                                   const std::string& rhs) const;
  std::string GetComplexDebugString() const;
  std::string GetSpecialDebugString() const;

  int PerformUniaryOperationOn(int int_operand) const;
  static int PerformBinaryOperationOn(char operand, int lhs, int rhs);

  ExpressionPieceType piece_type;

  union {
    // TYPE_INT_CONSTANT
    int int_constant;

    // TYPE_STRING_CONSTANT
    std::string str_constant;

    // TYPE_MEMORY_REFERENCE
    struct {
      int type;
      ExpressionPiece* location;
    } mem_reference;

    // TYPE_SIMPLE_MEMORY_REFERENCE
    struct {
      int type;
      int location;
    } simple_mem_reference;

    // TYPE_UNIARY_EXPRESSION
    struct {
      char operation;
      ExpressionPiece* operand;
    } uniary_expression;

    // TYPE_BINARY_EXPRESSION
    struct {
      char operation;
      ExpressionPiece* left_operand;
      ExpressionPiece* right_operand;
    } binary_expression;

    // TYPE_SIMPLE_ASSIGNMENT
    struct {
      int type;
      int location;
      int value;
    } simple_assignment;

    // TYPE_COMPLEX_EXPRESSION
    std::vector<ExpressionPiece> complex_expression;

    // TYPE_SPECIAL_EXPRESSION
    struct {
      int overload_tag;
      std::vector<ExpressionPiece> pieces;
    } special_expression;
  };
};

typedef std::vector<libreallive::ExpressionPiece> ExpressionPiecesVector;

}  // namespace libreallive

#endif  // SRC_LIBREALLIVE_EXPRESSION_H_
