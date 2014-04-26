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
std::unique_ptr<ExpressionPiece> GetExpressionToken(const char*& src);
std::unique_ptr<ExpressionPiece> GetExpressionTerm(const char*& src);
std::unique_ptr<ExpressionPiece> GetExpressionArithmatic(const char*& src);
std::unique_ptr<ExpressionPiece> GetExpressionCondition(const char*& src);
std::unique_ptr<ExpressionPiece> GetExpressionBoolean(const char*& src);
std::unique_ptr<ExpressionPiece> GetExpression(const char*& src);
std::unique_ptr<ExpressionPiece> GetAssignment(const char*& src);
std::unique_ptr<ExpressionPiece> GetData(const char*& src);
std::unique_ptr<ExpressionPiece> GetComplexParam(const char*& src);

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

class ExpressionPiece {
 public:
  virtual ~ExpressionPiece();

  // Capability method; returns false by default. Override when
  // ExpressionPiece subclass accesses a piece of memory.
  virtual bool IsMemoryReference() const;

  // Capability method; returns false by default. Override when
  // ExpressionPiece subclass is an operation on one or more other
  // ExpressionPieces.
  virtual bool IsOperator() const;

  // Used only to add a '=' in debug strings.
  virtual bool IsAssignment() const;

  // Capability method; returns false by default. Override only in
  // classes that represent a complex parameter to the type system.
  // @see Complex2_T
  virtual bool IsComplexParameter() const;

  // Capability method; returns false by default. Override only in
  // classes that represent a special parameter to the type system.
  // @see Special_T
  virtual bool IsSpecialParameter() const;

  // Returns the value type of this expression (i.e. string or
  // integer)
  virtual ExpressionValueType GetExpressionValueType() const;

  // Assigns the value into the memory location represented by the
  // current expression. Not all ExpressionPieces can do this, so
  // there is a default implementation which does nothing.
  virtual void SetIntegerValue(RLMachine& machine, int rvalue);

  // Returns the integer value of this expression; this can either be
  // a memory access or a calculation based on some subexpressions.
  virtual int GetIntegerValue(RLMachine& machine) const;

  virtual void SetStringValue(RLMachine& machine,
                                 const std::string& rvalue);
  virtual const std::string& GetStringValue(RLMachine& machine) const;

  // A persistable version of this value. This method should return RealLive
  // bytecode equal to this ExpressionPiece with all references returned.
  virtual std::string GetSerializedExpression(RLMachine& machine) const = 0;

  // A printable representation of the expression itself. Used to dump our
  // parsing of the bytecode to the console.
  virtual std::string GetDebugString() const = 0;

  // I used to be able to just static cast any ExpressionPiece to a
  // MemoryReference if I wanted/needed a corresponding iterator. Haeleth's
  // rlBabel library instead uses the store register as an argument to a
  // function that takes a integer reference. So this needs to be here now.
  virtual IntReferenceIterator GetIntegerReferenceIterator(
      RLMachine& machine) const;
  virtual StringReferenceIterator GetStringReferenceIterator(
      RLMachine& machine) const;

  // Builds a copy of this Expression.
  virtual std::unique_ptr<ExpressionPiece> Clone() const = 0;
};

typedef std::vector<std::unique_ptr<libreallive::ExpressionPiece> >
    ExpressionPiecesVector;

}  // namespace libreallive

#endif  // SRC_LIBREALLIVE_EXPRESSION_H_
