// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of libReallive, a dependency of RLVM.
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

#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <boost/scoped_ptr.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

class RLMachine;

namespace libReallive {

// Size of expression functions
size_t next_token(const char* src);
size_t next_expr(const char* src);
size_t next_string(const char* src);
size_t next_data(const char* src);

// Parse expression functions
class ExpressionPiece;
ExpressionPiece* get_expr_token(const char*& src);
ExpressionPiece* get_expr_term(const char*& src);
ExpressionPiece* get_expr_arith(const char*& src);
ExpressionPiece* get_expr_cond(const char*& src);
ExpressionPiece* get_expr_bool(const char*& src);
ExpressionPiece* get_expression(const char*& src);
ExpressionPiece* get_assignment(const char*& src);
ExpressionPiece* get_data(const char*& src);
ExpressionPiece* get_complex_param(const char*& src);

std::string parsableToPrintableString(const std::string& src);
std::string printableToParsableString(const std::string& src);

enum ExpressionValueType {
  ValueTypeInteger,
  ValueTypeString
};

class ExpressionPiece {
public:
  virtual ~ExpressionPiece();

  /// Capability method; returns false by default. Override when
  /// ExpressionPiece subclass accesses a piece of memory.
  virtual bool isMemoryReference() const;

  /// Capability method; returns false by default. Override when
  /// ExpressionPiece subclass is an operation on one or more other
  /// ExpressionPieces.
  virtual bool isOperator() const;

  /// Capability method; returns false by default. Override only in
  /// classes that represent a complex parameter to the type system.
  /// @see Complex2_T
  virtual bool isComplexParameter() const;

  /// Capability method; returns false by default. Override only in
  /// classes that represent a special parameter to the type system.
  /// @see Special_T
  virtual bool isSpecialParamater() const;

  /// Returns the value type of this expression (i.e. string or
  /// integer)
  virtual ExpressionValueType expressionValueType() const;

  /// Assigns the value into the memory location represented by the
  /// current expression. Not all ExpressionPieces can do this, so
  /// there is a default implementation which does nothing.
  virtual void assignIntValue(RLMachine& machine, int rvalue);

  /// Returns the integer value of this expression; this can either be
  /// a memory access or a calculation based on some subexpressions.
  virtual int integerValue(RLMachine& machine) const;

  virtual void assignStringValue(RLMachine& machine);
  virtual const std::string& getStringValue(RLMachine& machine) const;

  virtual ExpressionPiece* clone() const = 0;
};

// Boost helper
inline ExpressionPiece* new_clone( const ExpressionPiece& a )
{
    return a.clone();
}

// -----------------------------------------------------------------------

}

#endif
