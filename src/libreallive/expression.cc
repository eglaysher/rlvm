// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
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

#include "libreallive/expression.h"

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/tokenizer.hpp>

#include <iomanip>
#include <sstream>
#include <string>

#include "libreallive/defs.h"
#include "libreallive/intmemref.h"
#include "machine/reference.h"
#include "machine/rlmachine.h"

namespace {

std::string IntToBytecode(int val) {
  std::string prefix("$\xFF");
  libreallive::append_i32(prefix, val);
  return prefix;
}

bool IsUnescapedQuotationMark(const char* src, const char* current) {
  if (src == current)
    return *current == '"';
  else
    return *current == '"' && *(current - 1) != '\\';
}

}  // namespace

namespace libreallive {

// Expression Tokenization
//
// Functions that tokenize expression data while parsing the bytecode
// to create the BytecodeElements. These functions simply tokenize and
// mark boundaries; they do not perform any parsing.

size_t NextToken(const char* src) {
  if (*src++ != '$')
    return 0;
  if (*src++ == 0xff)
    return 6;
  if (*src++ != '[')
    return 2;
  return 4 + NextExpression(src);
}

static size_t NextTerm(const char* src) {
  if (*src == '(')
    return 2 + NextExpression(src + 1);
  if (*src == '\\')
    return 2 + NextTerm(src + 2);
  return NextToken(src);
}

static size_t NextArithmatic(const char* src) {
  size_t lhs = NextTerm(src);
  return (src[lhs] == '\\') ? lhs + 2 + NextArithmatic(src + lhs + 2) : lhs;
}

static size_t NextCondition(const char* src) {
  size_t lhs = NextArithmatic(src);
  return (src[lhs] == '\\' && src[lhs + 1] >= 0x28 && src[lhs + 1] <= 0x2d)
             ? lhs + 2 + NextArithmatic(src + lhs + 2)
             : lhs;
}

static size_t NextAnd(const char* src) {
  size_t lhs = NextCondition(src);
  return (src[lhs] == '\\' && src[lhs + 1] == '<')
             ? lhs + 2 + NextAnd(src + lhs + 2)
             : lhs;
}

size_t NextExpression(const char* src) {
  size_t lhs = NextAnd(src);
  return (src[lhs] == '\\' && src[lhs + 1] == '=')
             ? lhs + 2 + NextExpression(src + lhs + 2)
             : lhs;
}

size_t NextString(const char* src) {
  bool quoted = false;
  const char* end = src;

  while (true) {
    if (quoted) {
      quoted = !IsUnescapedQuotationMark(src, end);
      if (!quoted && *(end - 1) != '\\') {
        end++;  // consume the final quote
        break;
      }
    } else {
      quoted = IsUnescapedQuotationMark(src, end);
      if (strncmp(end, "###PRINT(", 9) == 0) {
        end += 9;
        end += 1 + NextExpression(end);
        continue;
      }
      if (!((*end >= 0x81 && *end <= 0x9f) || (*end >= 0xe0 && *end <= 0xef) ||
            (*end >= 'a' && *end <= 'z') ||
            (*end >= 'A' && *end <= 'Z') || (*end >= '0' && *end <= '9') ||
            *end == ' ' || *end == '?' || *end == '_' || *end == '"' ||
            *end == '\\')) {
        break;
      }
    }
    if ((*end >= 0x81 && *end <= 0x9f) || (*end >= 0xe0 && *end <= 0xef))
      end += 2;
    else
      ++end;
  }

  return end - src;
}

size_t NextData(const char* src) {
  if (*src == ',')
    return 1 + NextData(src + 1);
  if (*src == '\n')
    return 3 + NextData(src + 3);
  if ((*src >= 0x81 && *src <= 0x9f) || (*src >= 0xe0 && *src <= 0xef) ||
      (*src >= 'A' && *src <= 'Z') || (*src >= '0' && *src <= '9') ||
      *src == ' ' || *src == '?' || *src == '_' || *src == '"' ||
      strcmp(src, "###PRINT(") == 0)
    return NextString(src);
  if (*src == 'a' || *src == '(') {
    const char* end = src;
    if (*end++ == 'a') {
      ++end;

      // Some special cases have multiple tags.
      if (*end == 'a')
        end += 2;

      if (*end != '(') {
        end += NextData(end);
        return end - src;
      } else {
        end++;
      }
    }

    while (*end != ')')
      end += NextData(end);
    end++;
    if (*end == '\\')
      end += NextExpression(end);
    return end - src;
  } else {
    return NextExpression(src);
  }
}

// -----------------------------------------------------------------------

// Expression Parsing
//
// Functions used at runtime to parse expressions, both as
// ExpressionPieces, parameters in function calls, and other uses in
// some special cases. These functions form a recursive descent parser
// that parses expressions and parameters in Reallive byte code into
// ExpressionPieces, which are executed with the current RLMachine.
//
// These functions were translated from the O'Caml implementation in
// dissassembler.ml in RLDev, so really, while I coded this, Haeleth
// really gets all the credit.

ExpressionPiece GetExpressionToken(const char*& src) {
  if (src[0] == 0xff) {
    src++;
    int value = read_i32(src);
    src += 4;
    return ExpressionPiece::IntConstant(value);
  } else if (src[0] == 0xc8) {
    src++;
    return ExpressionPiece::StoreRegister();
  } else if ((src[0] != 0xc8 && src[0] != 0xff) && src[1] == '[') {
    int type = src[0];
    src += 2;
    ExpressionPiece location = GetExpression(src);

    if (src[0] != ']') {
      std::ostringstream ss;
      ss << "Unexpected character '" << src[0] << "' in GetExpressionToken"
         << " (']' expected)";
      throw Error(ss.str());
    }
    src++;

    return ExpressionPiece::MemoryReference(type, std::move(location));
  } else if (src[0] == 0) {
    throw Error("Unexpected end of buffer in GetExpressionToken");
  } else {
    std::ostringstream err;
    err << "Unknown toke type 0x" << std::hex << (short)src[0]
        << " in GetExpressionToken" << std::endl;
    throw Error(err.str());
  }
}

ExpressionPiece GetExpressionTerm(const char*& src) {
  if (src[0] == '$') {
    src++;
    return GetExpressionToken(src);
  } else if (src[0] == '\\' && src[1] == 0x00) {
    src += 2;
    return GetExpressionTerm(src);
  } else if (src[0] == '\\' && src[1] == 0x01) {
    // Uniary -
    src += 2;
    return ExpressionPiece::UniaryExpression(0x01, GetExpressionTerm(src));
  } else if (src[0] == '(') {
    src++;
    ExpressionPiece p = GetExpressionBoolean(src);
    if (src[0] != ')') {
      std::ostringstream ss;
      ss << "Unexpected character '" << src[0] << "' in GetExpressionTerm"
         << " (')' expected)";
      throw Error(ss.str());
    }
    src++;
    return p;
  } else if (src[0] == 0) {
    throw Error("Unexpected end of buffer in GetExpressionTerm");
  } else {
    std::ostringstream err;
    err << "Unknown token type 0x" << std::hex << (short)src[0]
        << " in GetExpressionTerm";
    throw Error(err.str());
  }
}

static ExpressionPiece GetExpressionArithmaticLoopHiPrec(
    const char*& src,
    ExpressionPiece tok) {
  if (src[0] == '\\' && src[1] >= 0x02 && src[1] <= 0x09) {
    char op = src[1];
    // Advance past this operator
    src += 2;
    ExpressionPiece new_piece = ExpressionPiece::BinaryExpression(
        op, std::move(tok), GetExpressionTerm(src));
    return GetExpressionArithmaticLoopHiPrec(src, std::move(new_piece));
  } else {
    // We don't consume anything and just return our input token.
    return tok;
  }
}

static ExpressionPiece GetExpressionArithmaticLoop(
    const char*& src,
    ExpressionPiece tok) {
  if (src[0] == '\\' && (src[1] == 0x00 || src[1] == 0x01)) {
    char op = src[1];
    src += 2;
    ExpressionPiece other = GetExpressionTerm(src);
    ExpressionPiece rhs =
        GetExpressionArithmaticLoopHiPrec(src, std::move(other));
    ExpressionPiece new_piece =
        ExpressionPiece::BinaryExpression(op, std::move(tok), std::move(rhs));
    return GetExpressionArithmaticLoop(src, std::move(new_piece));
  } else {
    return tok;
  }
}

ExpressionPiece GetExpressionArithmatic(const char*& src) {
  return GetExpressionArithmaticLoop(
      src, GetExpressionArithmaticLoopHiPrec(src, GetExpressionTerm(src)));
}

static ExpressionPiece GetExpressionConditionLoop(
    const char*& src,
    ExpressionPiece tok) {
  if (src[0] == '\\' && (src[1] >= 0x28 && src[1] <= 0x2d)) {
    char op = src[1];
    src += 2;
    ExpressionPiece rhs = GetExpressionArithmatic(src);
    ExpressionPiece new_piece =
        ExpressionPiece::BinaryExpression(op, std::move(tok), std::move(rhs));
    return GetExpressionConditionLoop(src, std::move(new_piece));
  } else {
    return tok;
  }
}

ExpressionPiece GetExpressionCondition(const char*& src) {
  return GetExpressionConditionLoop(src, GetExpressionArithmatic(src));
}

static ExpressionPiece GetExpressionBooleanLoopAnd(
    const char*& src,
    ExpressionPiece tok) {
  if (src[0] == '\\' && src[1] == '<') {
    src += 2;
    ExpressionPiece rhs = GetExpressionCondition(src);
    return GetExpressionBooleanLoopAnd(
        src,
        ExpressionPiece::BinaryExpression(
            0x3c, std::move(tok), std::move(rhs)));
  } else {
    return tok;
  }
}

static ExpressionPiece GetExpressionBooleanLoopOr(
    const char*& src,
    ExpressionPiece tok) {
  if (src[0] == '\\' && src[1] == '=') {
    src += 2;
    ExpressionPiece innerTerm = GetExpressionCondition(src);
    ExpressionPiece rhs =
        GetExpressionBooleanLoopAnd(src, std::move(innerTerm));
    return GetExpressionBooleanLoopOr(
        src,
        ExpressionPiece::BinaryExpression(
            0x3d, std::move(tok), std::move(rhs)));
  } else {
    return tok;
  }
}

ExpressionPiece GetExpressionBoolean(const char*& src) {
  return GetExpressionBooleanLoopOr(
      src,
      GetExpressionBooleanLoopAnd(src, GetExpressionCondition(src)));
}

ExpressionPiece GetExpression(const char*& src) {
  return GetExpressionBoolean(src);
}

// Parses an expression of the form [dest] = [source expression];
ExpressionPiece GetAssignment(const char*& src) {
  ExpressionPiece itok(GetExpressionTerm(src));
  int op = src[1];
  src += 2;
  ExpressionPiece etok(GetExpression(src));
  if (op >= 0x14 && op <= 0x24) {
    return ExpressionPiece::BinaryExpression(
        op, std::move(itok), std::move(etok));
  } else {
    throw Error("Undefined assignment in GetAssignment");
  }
}

// Parses a string in the parameter list.
static ExpressionPiece GetString(const char*& src) {
  // Get the length of this string in the bytecode:
  size_t length = NextString(src);

  string s;
  // Check to see if the string is quoted;
  if (src[0] == '"')
    s = string(src + 1, src + length - 1);
  else
    s = string(src, src + length);

  // Increment the source by that many characters
  src += length;

  // Unquote the internal quotations.
  boost::replace_all(s, "\\\"", "\"");

  return ExpressionPiece::StrConstant(s);
}

// Parses a parameter in the parameter list. This is the only method
// of all the get_*(const char*& src) functions that can parse
// strings. It also deals with things like special and complex
// parameters.
ExpressionPiece GetData(const char*& src) {
  if (*src == ',') {
    ++src;
    return GetData(src);
  } else if (*src == '\n') {
    src += 3;
    return GetData(src);
  } else if ((*src >= 0x81 && *src <= 0x9f) || (*src >= 0xe0 && *src <= 0xef) ||
             (*src >= 'A' && *src <= 'Z') || (*src >= '0' && *src <= '9') ||
             *src == ' ' || *src == '?' || *src == '_' || *src == '"' ||
             strcmp(src, "###PRINT(") == 0) {
    return GetString(src);
  } else if (*src == 'a') {
    // TODO(erg): Cleanup below.
    const char* end = src;

    ExpressionPiece cep = ExpressionPiece::ComplexExpression();

    if (*end++ == 'a') {
      int tag = *end++;

      // Some special cases have multiple tags.
      if (*end == 'a') {
        end++;
        int second = *end++;
        tag = (second << 16) | tag;
      }

      cep = ExpressionPiece::SpecialExpression(tag);

      if (*end != '(') {
        // We have a single parameter in this special expression;
        cep.AddContainedPiece(GetData(end));
        return cep;
      } else {
        end++;
      }
    } else {
      cep = ExpressionPiece::ComplexExpression();
    }

    while (*end != ')') {
      cep.AddContainedPiece(GetData(end));
    }

    return cep;
  } else {
    return GetExpression(src);
  }
}

ExpressionPiece GetComplexParam(const char*& src) {
  if (*src == ',') {
    ++src;
    return GetData(src);
  } else if (*src == '(') {
    ++src;
    ExpressionPiece cep = ExpressionPiece::ComplexExpression();

    while (*src != ')')
      cep.AddContainedPiece(GetData(src));

    return cep;
  } else {
    return GetExpression(src);
  }
}

std::string EvaluatePRINT(RLMachine& machine, const std::string& in) {
  // Currently, this doesn't evaluate the # commands inline. See 5.12.11 of the
  // rldev manual.
  if (boost::starts_with(in, "###PRINT(")) {
    const char* expression_start = in.c_str() + 9;
    ExpressionPiece piece(GetExpression(expression_start));

    if (*expression_start != ')') {
      std::ostringstream ss;
      ss << "Unexpected character '" << *expression_start
         << "' in evaluatePRINT (')' expected)";
      throw Error(ss.str());
    }

    return piece.GetStringValue(machine);
  } else {
    // Just a normal string we can ignore
    return in;
  }
}

std::string ParsableToPrintableString(const std::string& src) {
  string output;

  bool firstToken = true;
  for (char tok : src) {
    if (firstToken) {
      firstToken = false;
    } else {
      output += " ";
    }

    if (tok == '(' || tok == ')' || tok == '$' || tok == '[' || tok == ']') {
      output.push_back(tok);
    } else {
      std::ostringstream ss;
      ss << std::hex << std::setw(2) << std::setfill('0') << int(tok);
      output += ss.str();
    }
  }

  return output;
}

// -----------------------------------------------------------------------

std::string PrintableToParsableString(const std::string& src) {
  typedef boost::tokenizer<boost::char_separator<char>> ttokenizer;

  std::string output;

  boost::char_separator<char> sep(" ");
  ttokenizer tokens(src, sep);
  for (string const& tok : tokens) {
    if (tok.size() > 2)
      throw libreallive::Error(
          "Invalid string given to printableToParsableString");

    if (tok == "(" || tok == ")" || tok == "$" || tok == "[" || tok == "]") {
      output.push_back(tok[0]);
    } else {
      int charToAdd;
      std::istringstream ss(tok);
      ss >> std::hex >> charToAdd;
      output.push_back((char)charToAdd);
    }
  }

  return output;
}

// ----------------------------------------------------------------------

// OK: Here's the current things I need to do more:
//
// - I've written a move operator= (I've written the move ctor).
// - I need to write both the copy ctor and the copy operator=.
// - Lots of integration work still.


// static
ExpressionPiece ExpressionPiece::StoreRegister() {
  ExpressionPiece piece;
  piece.piece_type = TYPE_STORE_REGISTER;
  return piece;
}

// static
ExpressionPiece ExpressionPiece::IntConstant(const int constant) {
  ExpressionPiece piece;
  piece.piece_type = TYPE_INT_CONSTANT;
  piece.int_constant = constant;
  return piece;
}

// static
ExpressionPiece ExpressionPiece::StrConstant(const std::string constant) {
  ExpressionPiece piece;
  piece.piece_type = TYPE_STRING_CONSTANT;
  new (&piece.str_constant) std::string(constant);
  return piece;
}

// static
ExpressionPiece ExpressionPiece::MemoryReference(const int type,
                                                 ExpressionPiece location) {
  ExpressionPiece piece;
  if (location.piece_type == TYPE_INT_CONSTANT) {
    piece.piece_type = TYPE_SIMPLE_MEMORY_REFERENCE;
    piece.simple_mem_reference.type = type;
    piece.simple_mem_reference.location = location.int_constant;
  } else {
    piece.piece_type = TYPE_MEMORY_REFERENCE;
    piece.mem_reference.type = type;
    piece.mem_reference.location = new ExpressionPiece(std::move(location));
  }
  return piece;
}

// static
ExpressionPiece ExpressionPiece::UniaryExpression(const char operation,
                                                  ExpressionPiece operand) {
  ExpressionPiece piece;
  piece.piece_type = TYPE_UNIARY_EXPRESSION;
  piece.uniary_expression.operation = operation;
  piece.uniary_expression.operand = new ExpressionPiece(std::move(operand));
  return piece;
}

// static
ExpressionPiece ExpressionPiece::BinaryExpression(const char operation,
                                                  ExpressionPiece lhs,
                                                  ExpressionPiece rhs) {
  ExpressionPiece piece;
  if (operation == 30 &&
      lhs.piece_type == TYPE_SIMPLE_MEMORY_REFERENCE &&
      rhs.piece_type == TYPE_INT_CONSTANT) {
    // We can fast path so we don't allocate memory by stashing the memory
    // reference and the value in this piece.
    piece.piece_type = TYPE_SIMPLE_ASSIGNMENT;
    piece.simple_assignment.type = lhs.simple_mem_reference.type;
    piece.simple_assignment.location = lhs.simple_mem_reference.location;
    piece.simple_assignment.value = rhs.int_constant;
  } else if (lhs.piece_type == TYPE_INT_CONSTANT &&
             rhs.piece_type == TYPE_INT_CONSTANT) {
    // We can fast path so that we just compute the integer expression here.
    piece.piece_type = TYPE_INT_CONSTANT;
    piece.int_constant = PerformBinaryOperationOn(
        operation,
        lhs.int_constant,
        rhs.int_constant);
  } else {
    piece.piece_type = TYPE_BINARY_EXPRESSION;
    piece.binary_expression.operation = operation;
    piece.binary_expression.left_operand = new ExpressionPiece(std::move(lhs));
    piece.binary_expression.right_operand = new ExpressionPiece(std::move(rhs));
  }
  return piece;
}

// static
ExpressionPiece ExpressionPiece::ComplexExpression() {
  ExpressionPiece piece;
  piece.piece_type = TYPE_COMPLEX_EXPRESSION;
  new (&piece.complex_expression) std::vector<ExpressionPiece>();
  return piece;
}

// static
ExpressionPiece ExpressionPiece::SpecialExpression(const int tag) {
  ExpressionPiece piece;
  piece.piece_type = TYPE_SPECIAL_EXPRESSION;
  piece.special_expression.overload_tag = tag;
  new (&piece.special_expression.pieces) std::vector<ExpressionPiece>();
  return piece;
}

ExpressionPiece::ExpressionPiece(invalid_expression_piece_t)
    : piece_type(TYPE_INVALID) {
}

ExpressionPiece::ExpressionPiece(const ExpressionPiece& rhs)
    : piece_type(rhs.piece_type) {
  switch (piece_type) {
    case TYPE_STORE_REGISTER:
      break;
    case TYPE_INT_CONSTANT:
      int_constant = rhs.int_constant;
      break;
    case TYPE_STRING_CONSTANT:
      new (&str_constant) std::string(rhs.str_constant);
      break;
    case TYPE_MEMORY_REFERENCE:
      mem_reference.type = rhs.mem_reference.type;
      mem_reference.location =
          new ExpressionPiece(*rhs.mem_reference.location);
      break;
    case TYPE_SIMPLE_MEMORY_REFERENCE:
      simple_mem_reference.type = rhs.simple_mem_reference.type;
      simple_mem_reference.location = rhs.simple_mem_reference.location;
      break;
    case TYPE_UNIARY_EXPRESSION:
      uniary_expression.operation = rhs.uniary_expression.operation;
      uniary_expression.operand =
          new ExpressionPiece(*rhs.uniary_expression.operand);
      break;
    case TYPE_BINARY_EXPRESSION:
      binary_expression.operation = rhs.binary_expression.operation;
      binary_expression.left_operand =
          new ExpressionPiece(*rhs.binary_expression.left_operand);
      binary_expression.right_operand =
          new ExpressionPiece(*rhs.binary_expression.right_operand);
      break;
    case TYPE_SIMPLE_ASSIGNMENT:
      simple_assignment.type = rhs.simple_assignment.type;
      simple_assignment.location = rhs.simple_assignment.location;
      simple_assignment.value = rhs.simple_assignment.value;
      break;
    case TYPE_COMPLEX_EXPRESSION:
      new (&complex_expression) std::vector<ExpressionPiece>(
          rhs.complex_expression);
      break;
    case TYPE_SPECIAL_EXPRESSION:
      special_expression.overload_tag = rhs.special_expression.overload_tag;
      new (&special_expression.pieces) std::vector<ExpressionPiece>(
          rhs.special_expression.pieces);
      break;
    case TYPE_INVALID:
      break;
  }
}

ExpressionPiece::ExpressionPiece(ExpressionPiece&& rhs)
    : piece_type(rhs.piece_type) {
  switch (piece_type) {
    case TYPE_STORE_REGISTER:
      break;
    case TYPE_INT_CONSTANT:
      int_constant = rhs.int_constant;
      break;
    case TYPE_STRING_CONSTANT:
      new (&str_constant) std::string(std::move(rhs.str_constant));
      break;
    case TYPE_MEMORY_REFERENCE:
      mem_reference.type = rhs.mem_reference.type;
      mem_reference.location = rhs.mem_reference.location;
      rhs.mem_reference.location = nullptr;
      break;
    case TYPE_SIMPLE_MEMORY_REFERENCE:
      simple_mem_reference.type = rhs.simple_mem_reference.type;
      simple_mem_reference.location = rhs.simple_mem_reference.location;
      break;
    case TYPE_UNIARY_EXPRESSION:
      uniary_expression.operation = rhs.uniary_expression.operation;
      uniary_expression.operand = rhs.uniary_expression.operand;
      rhs.uniary_expression.operand = nullptr;
      break;
    case TYPE_BINARY_EXPRESSION:
      binary_expression.operation = rhs.binary_expression.operation;
      binary_expression.left_operand = rhs.binary_expression.left_operand;
      binary_expression.right_operand = rhs.binary_expression.right_operand;
      rhs.binary_expression.left_operand = nullptr;
      rhs.binary_expression.right_operand = nullptr;
      break;
    case TYPE_SIMPLE_ASSIGNMENT:
      simple_assignment.type = rhs.simple_assignment.type;
      simple_assignment.location = rhs.simple_assignment.location;
      simple_assignment.value = rhs.simple_assignment.value;
      break;
    case TYPE_COMPLEX_EXPRESSION:
      new (&complex_expression) std::vector<ExpressionPiece>(std::move(
          rhs.complex_expression));
      break;
    case TYPE_SPECIAL_EXPRESSION:
      special_expression.overload_tag = rhs.special_expression.overload_tag;
      new (&special_expression.pieces) std::vector<ExpressionPiece>(std::move(
          rhs.special_expression.pieces));
      break;
    case TYPE_INVALID:
      break;
  }

  rhs.piece_type = TYPE_INVALID;
}

ExpressionPiece::~ExpressionPiece() {
  Invalidate();
}

ExpressionPiece& ExpressionPiece::operator=(const ExpressionPiece& rhs) {
  Invalidate();

  piece_type = rhs.piece_type;
  switch (piece_type) {
    case TYPE_STORE_REGISTER:
      break;
    case TYPE_INT_CONSTANT:
      int_constant = rhs.int_constant;
      break;
    case TYPE_STRING_CONSTANT:
      new (&str_constant) std::string(rhs.str_constant);
      break;
    case TYPE_MEMORY_REFERENCE:
      mem_reference.type = rhs.mem_reference.type;
      mem_reference.location =
          new ExpressionPiece(*rhs.mem_reference.location);
      break;
    case TYPE_SIMPLE_MEMORY_REFERENCE:
      simple_mem_reference.type = rhs.simple_mem_reference.type;
      simple_mem_reference.location = rhs.simple_mem_reference.location;
      break;
    case TYPE_UNIARY_EXPRESSION:
      uniary_expression.operation = rhs.uniary_expression.operation;
      uniary_expression.operand =
          new ExpressionPiece(*rhs.uniary_expression.operand);
      break;
    case TYPE_BINARY_EXPRESSION:
      binary_expression.operation = rhs.binary_expression.operation;
      binary_expression.left_operand =
          new ExpressionPiece(*rhs.binary_expression.left_operand);
      binary_expression.right_operand =
          new ExpressionPiece(*rhs.binary_expression.right_operand);
      break;
    case TYPE_SIMPLE_ASSIGNMENT:
      simple_assignment.type = rhs.simple_assignment.type;
      simple_assignment.location = rhs.simple_assignment.location;
      simple_assignment.value = rhs.simple_assignment.value;
      break;
    case TYPE_COMPLEX_EXPRESSION:
      new (&complex_expression) std::vector<ExpressionPiece>(
          rhs.complex_expression);
      break;
    case TYPE_SPECIAL_EXPRESSION:
      special_expression.overload_tag = rhs.special_expression.overload_tag;
      new (&special_expression.pieces) std::vector<ExpressionPiece>(
          rhs.special_expression.pieces);
      break;
    case TYPE_INVALID:
      break;
  }

  return *this;
}

ExpressionPiece& ExpressionPiece::operator=(ExpressionPiece&& rhs) {
  Invalidate();

  piece_type = rhs.piece_type;
  switch (piece_type) {
    case TYPE_STORE_REGISTER:
      break;
    case TYPE_INT_CONSTANT:
      int_constant = rhs.int_constant;
      break;
    case TYPE_STRING_CONSTANT:
      new (&str_constant) std::string(std::move(rhs.str_constant));
      break;
    case TYPE_MEMORY_REFERENCE:
      mem_reference.type = rhs.mem_reference.type;
      mem_reference.location = rhs.mem_reference.location;
      rhs.mem_reference.location = nullptr;
      break;
    case TYPE_SIMPLE_MEMORY_REFERENCE:
      simple_mem_reference.type = rhs.simple_mem_reference.type;
      simple_mem_reference.location = rhs.simple_mem_reference.location;
      break;
    case TYPE_UNIARY_EXPRESSION:
      uniary_expression.operation = rhs.uniary_expression.operation;
      uniary_expression.operand = rhs.uniary_expression.operand;
      rhs.uniary_expression.operand = nullptr;
      break;
    case TYPE_BINARY_EXPRESSION:
      binary_expression.operation = rhs.binary_expression.operation;
      binary_expression.left_operand = rhs.binary_expression.left_operand;
      binary_expression.right_operand = rhs.binary_expression.right_operand;
      rhs.binary_expression.left_operand = nullptr;
      rhs.binary_expression.right_operand = nullptr;
      break;
    case TYPE_SIMPLE_ASSIGNMENT:
      simple_assignment.type = rhs.simple_assignment.type;
      simple_assignment.location = rhs.simple_assignment.location;
      simple_assignment.value = rhs.simple_assignment.value;
      break;
    case TYPE_COMPLEX_EXPRESSION:
      new (&complex_expression) std::vector<ExpressionPiece>(std::move(
          rhs.complex_expression));
      break;
    case TYPE_SPECIAL_EXPRESSION:
      special_expression.overload_tag = rhs.special_expression.overload_tag;
      new (&special_expression.pieces) std::vector<ExpressionPiece>(std::move(
          rhs.special_expression.pieces));
      break;
    case TYPE_INVALID:
      break;
  }

  rhs.piece_type = TYPE_INVALID;

  return *this;
}

bool ExpressionPiece::IsMemoryReference() const {
  return piece_type == TYPE_STORE_REGISTER ||
      piece_type == TYPE_MEMORY_REFERENCE ||
      piece_type == TYPE_SIMPLE_MEMORY_REFERENCE;
}

bool ExpressionPiece::IsComplexParameter() const {
  return piece_type == TYPE_COMPLEX_EXPRESSION;
}

bool ExpressionPiece::IsSpecialParameter() const {
  return piece_type == TYPE_SPECIAL_EXPRESSION;
}

ExpressionValueType ExpressionPiece::GetExpressionValueType() const {
  switch (piece_type) {
    case TYPE_STRING_CONSTANT:
      return ValueTypeString;
    case TYPE_MEMORY_REFERENCE:
      return is_string_location(mem_reference.type) ?
          ValueTypeString : ValueTypeInteger;
    case TYPE_SIMPLE_MEMORY_REFERENCE:
      return is_string_location(simple_mem_reference.type) ?
          ValueTypeString : ValueTypeInteger;
    default:
      return ValueTypeInteger;
  }
}

void ExpressionPiece::SetIntegerValue(RLMachine& machine, int rvalue) {
  switch (piece_type) {
    case TYPE_STORE_REGISTER:
      machine.set_store_register(rvalue);
      break;
    case TYPE_MEMORY_REFERENCE:
      machine.SetIntValue(
          IntMemRef(
              mem_reference.type,
              mem_reference.location->GetIntegerValue(machine)),
          rvalue);
      break;
    case TYPE_SIMPLE_MEMORY_REFERENCE:
      machine.SetIntValue(
          IntMemRef(
              simple_mem_reference.type,
              simple_mem_reference.location),
          rvalue);
      break;
    default: {
      std::ostringstream ss;
      ss << "ExpressionPiece::SetIntegerValue() invalid on object of type "
         << piece_type;
      throw Error(ss.str());
    }
  }
}

int ExpressionPiece::GetIntegerValue(RLMachine& machine) const {
  switch (piece_type) {
    case TYPE_STORE_REGISTER:
      return machine.store_register();
    case TYPE_INT_CONSTANT:
      return int_constant;
    case TYPE_MEMORY_REFERENCE:
      return machine.GetIntValue(IntMemRef(
          mem_reference.type,
          mem_reference.location->GetIntegerValue(machine)));
    case TYPE_SIMPLE_MEMORY_REFERENCE:
      return machine.GetIntValue(IntMemRef(
          simple_mem_reference.type,
          simple_mem_reference.location));
    case TYPE_UNIARY_EXPRESSION:
      return PerformUniaryOperationOn(
          uniary_expression.operand->GetIntegerValue(machine));
    case TYPE_BINARY_EXPRESSION:
      if (binary_expression.operation >= 20 &&
          binary_expression.operation < 30) {
        int value = PerformBinaryOperationOn(
            binary_expression.operation,
            binary_expression.left_operand->GetIntegerValue(machine),
            binary_expression.right_operand->GetIntegerValue(machine));
        binary_expression.left_operand->SetIntegerValue(machine, value);
        return value;
      } else if (binary_expression.operation == 30) {
        int value = binary_expression.right_operand->GetIntegerValue(machine);
        binary_expression.left_operand->SetIntegerValue(machine, value);
        return value;
      } else {
        return PerformBinaryOperationOn(
            binary_expression.operation,
            binary_expression.left_operand->GetIntegerValue(machine),
            binary_expression.right_operand->GetIntegerValue(machine));
      }
    case TYPE_SIMPLE_ASSIGNMENT:
      machine.SetIntValue(
          IntMemRef(simple_assignment.type,
                    simple_assignment.location),
          simple_assignment.value);
      return simple_assignment.value;
    default: {
      std::ostringstream ss;
      ss << "ExpressionPiece::GetIntegerValue() invalid on object of type "
         << piece_type;
      throw Error(ss.str());
    }
  }
}

void ExpressionPiece::SetStringValue(RLMachine& machine,
                                     const std::string& rvalue) {
  switch (piece_type) {
    case TYPE_MEMORY_REFERENCE:
      machine.SetStringValue(mem_reference.type,
                             mem_reference.location->GetIntegerValue(machine),
                             rvalue);
      return;
    case TYPE_SIMPLE_MEMORY_REFERENCE:
      machine.SetStringValue(simple_mem_reference.type,
                             simple_mem_reference.location,
                             rvalue);
      return;
    default:
      throw libreallive::Error(
          "ExpressionPiece::SetStringValue() invalid on this object");
  }
}

const std::string& ExpressionPiece::GetStringValue(RLMachine& machine) const {
  switch (piece_type) {
    case TYPE_STRING_CONSTANT:
      return str_constant;
    case TYPE_MEMORY_REFERENCE:
      return machine.GetStringValue(mem_reference.type,
                                    mem_reference.location->GetIntegerValue(
                                        machine));
    case TYPE_SIMPLE_MEMORY_REFERENCE:
      return machine.GetStringValue(simple_mem_reference.type,
                                    simple_mem_reference.location);
    default:
      throw libreallive::Error(
          "ExpressionPiece::GetStringValue() invalid on this object");
  }
}

IntReferenceIterator ExpressionPiece::GetIntegerReferenceIterator(
    RLMachine& machine) const {
  switch (piece_type) {
    case TYPE_STORE_REGISTER:
      return IntReferenceIterator(machine.store_register_address());
    case TYPE_MEMORY_REFERENCE:
      // Make sure that we are actually referencing an integer
      if (is_string_location(mem_reference.type)) {
        throw Error(
            "Request to GetIntegerReferenceIterator() on a string reference!");
      }

      return IntReferenceIterator(
          &machine.memory(),
          mem_reference.type,
          mem_reference.location->GetIntegerValue(machine));
    case TYPE_SIMPLE_MEMORY_REFERENCE:
      // Make sure that we are actually referencing an integer
      if (is_string_location(simple_mem_reference.type)) {
        throw Error(
            "Request to GetIntegerReferenceIterator() on a string reference!");
      }

      return IntReferenceIterator(
          &machine.memory(),
          simple_mem_reference.type,
          simple_mem_reference.location);
    default:
      throw libreallive::Error(
          "ExpressionPiece::GetIntegerReferenceIterator() invalid on this object");
  }
}

StringReferenceIterator ExpressionPiece::GetStringReferenceIterator(
    RLMachine& machine) const {
  switch (piece_type) {
    case TYPE_MEMORY_REFERENCE:
      // Make sure that we are actually referencing an integer
      if (!is_string_location(mem_reference.type)) {
        throw Error(
            "Request to GetStringReferenceIterator() on an integer reference!");
      }

      return StringReferenceIterator(
          &machine.memory(),
          mem_reference.type,
          mem_reference.location->GetIntegerValue(machine));
    case TYPE_SIMPLE_MEMORY_REFERENCE:
      // Make sure that we are actually referencing an integer
      if (!is_string_location(simple_mem_reference.type)) {
        throw Error(
            "Request to GetStringReferenceIterator() on an integer reference!");
      }

      return StringReferenceIterator(
          &machine.memory(),
          simple_mem_reference.type,
          simple_mem_reference.location);
    default:
      throw libreallive::Error("ExpressionPiece::GetStringReferenceIterator()"
                               " invalid on this object");
  }
}

std::string ExpressionPiece::GetSerializedExpression(RLMachine& machine) const {
  switch (piece_type) {
    case TYPE_STORE_REGISTER:
      return IntToBytecode(machine.store_register());
    case TYPE_INT_CONSTANT:
      return IntToBytecode(int_constant);
    case TYPE_STRING_CONSTANT:
      return string("\"") + str_constant + string("\"");
    case TYPE_MEMORY_REFERENCE:
      if (is_string_location(mem_reference.type)) {
        return string("\"") + GetStringValue(machine) + string("\"");
      } else {
        return IntToBytecode(GetIntegerValue(machine));
      }
    case TYPE_SIMPLE_MEMORY_REFERENCE:
      if (is_string_location(simple_mem_reference.type)) {
        return string("\"") + GetStringValue(machine) + string("\"");
      } else {
        return IntToBytecode(GetIntegerValue(machine));
      }
    case TYPE_UNIARY_EXPRESSION:
    case TYPE_BINARY_EXPRESSION:
    case TYPE_SIMPLE_ASSIGNMENT:
      return IntToBytecode(GetIntegerValue(machine));
    case TYPE_COMPLEX_EXPRESSION:
      return GetComplexSerializedExpression(machine);
    case TYPE_SPECIAL_EXPRESSION:
      return GetSpecialSerializedExpression(machine);
    case TYPE_INVALID:
      throw Error("Called GetSerializedExpression on an invalid piece.");
  }

  return "<invalid>";
}

std::string ExpressionPiece::GetDebugString() const {
  switch (piece_type) {
    case TYPE_STORE_REGISTER:
      return "<store>";
    case TYPE_INT_CONSTANT:
      return std::to_string(int_constant);
    case TYPE_STRING_CONSTANT:
      return string("\"") + str_constant + string("\"");
    case TYPE_MEMORY_REFERENCE:
      return GetMemoryDebugString(mem_reference.type,
                                  mem_reference.location->GetDebugString());
    case TYPE_SIMPLE_MEMORY_REFERENCE:
      return GetMemoryDebugString(
          simple_mem_reference.type,
          std::to_string(simple_mem_reference.location));
    case TYPE_UNIARY_EXPRESSION:
      return GetUniaryDebugString();
    case TYPE_BINARY_EXPRESSION:
      return GetBinaryDebugString(
          binary_expression.operation,
          binary_expression.left_operand->GetDebugString(),
          binary_expression.right_operand->GetDebugString());
    case TYPE_SIMPLE_ASSIGNMENT:
      return GetBinaryDebugString(
          30,
          GetMemoryDebugString(simple_assignment.type,
                               std::to_string(simple_assignment.location)),
          std::to_string(simple_assignment.value));
    case TYPE_COMPLEX_EXPRESSION:
      return GetComplexDebugString();
    case TYPE_SPECIAL_EXPRESSION:
      return GetSpecialDebugString();
    case TYPE_INVALID:
      return "<invalid>";
  }

  return "<invalid>";
}

// -----------------------------------------------------------------------------

ExpressionPiece::ExpressionPiece() : piece_type(TYPE_INVALID) {}

void ExpressionPiece::Invalidate() {
  // Needed to get around a quirk of the language
  using string_type = std::string;
  using vec_type = std::vector<ExpressionPiece>;

  switch (piece_type) {
    case TYPE_STORE_REGISTER:
    case TYPE_INT_CONSTANT:
      break;
    case TYPE_STRING_CONSTANT:
      str_constant.~string_type();
      break;
    case TYPE_MEMORY_REFERENCE:
      delete mem_reference.location;
      break;
    case TYPE_SIMPLE_MEMORY_REFERENCE:
      break;
    case TYPE_UNIARY_EXPRESSION:
      delete uniary_expression.operand;
      break;
    case TYPE_BINARY_EXPRESSION:
      delete binary_expression.left_operand;
      delete binary_expression.right_operand;
      break;
    case TYPE_SIMPLE_ASSIGNMENT:
      break;
    case TYPE_COMPLEX_EXPRESSION:
      complex_expression.~vec_type();
      break;
    case TYPE_SPECIAL_EXPRESSION:
      special_expression.pieces.~vec_type();
      break;
    case TYPE_INVALID:
      break;
  }
}

// -----------------------------------------------------------------------------

std::string ExpressionPiece::GetComplexSerializedExpression(
    RLMachine& machine) const {
  string s("(");
  for (auto const& piece : complex_expression) {
    s += "(";
    s += piece.GetSerializedExpression(machine);
    s += ")";
  }
  s += ")";
  return s;
}

std::string ExpressionPiece::GetSpecialSerializedExpression(
    RLMachine& machine) const {
  string s("a");
  s += char(special_expression.overload_tag);

  if (special_expression.pieces.size() > 1)
    s.append("(");
  for (auto const& piece : special_expression.pieces) {
    s += piece.GetSerializedExpression(machine);
  }
  if (special_expression.pieces.size() > 1)
    s.append(")");

  return s;
}

std::string ExpressionPiece::GetMemoryDebugString(
    int type,
    const std::string& location) const {
  std::ostringstream ret;

  if (type == STRS_LOCATION) {
    ret << "strS[";
  } else if (type == STRK_LOCATION) {
    ret << "strK[";
  } else if (type == STRM_LOCATION) {
    ret << "strM[";
  } else if (type == INTZ_LOCATION_IN_BYTECODE) {
    ret << "intZ[";
  } else if (type == INTL_LOCATION_IN_BYTECODE) {
    ret << "intL[";
  } else {
    char bank = 'A' + (type % 26);
    ret << "int" << bank << "[";
  }

  ret << location;

  ret << "]";
  return ret.str();
}

std::string ExpressionPiece::GetUniaryDebugString() const {
  std::ostringstream str;
  if (uniary_expression.operation == 0x01) {
    str << "-";
  }
  str << uniary_expression.operand->GetDebugString();

  return str.str();
}

std::string ExpressionPiece::GetBinaryDebugString(
    char operation,
    const std::string& lhs,
    const std::string& rhs) const {
  std::ostringstream str;
  str << lhs;
  str << " ";

  switch (operation) {
    case 0:
    case 20:
      str << "+";
      break;
    case 1:
    case 21:
      str << "-";
      break;
    case 2:
    case 22:
      str << "*";
      break;
    case 3:
    case 23:
      str << "/";
      break;
    case 4:
    case 24:
      str << "%";
      break;
    case 5:
    case 25:
      str << "&";
      break;
    case 6:
    case 26:
      str << "|";
      break;
    case 7:
    case 27:
      str << "^";
      break;
    case 8:
    case 28:
      str << "<<";
      break;
    case 9:
    case 29:
      str << ">>";
      break;
    case 30:
      str << "=";
      break;
    case 40:
      str << "==";
      break;
    case 41:
      str << "!=";
      break;
    case 42:
      str << "<=";
      break;
    case 43:
      str << "< ";
      break;
    case 44:
      str << ">=";
      break;
    case 45:
      str << "> ";
      break;
    case 60:
      str << "&&";
      break;
    case 61:
      str << "||";
      break;
    default: {
      std::ostringstream ss;
      ss << "Invalid operator "
         << static_cast<int>(binary_expression.operation)
         << " in expression!";
      throw Error(ss.str());
    }
  }

  if (operation >= 0x14 && operation != 30)
    str << "=";

  str << " ";
  str << rhs;

  return str.str();
}

std::string ExpressionPiece::GetComplexDebugString() const {
  string s("(");
  for (auto const& piece : complex_expression) {
    s += "(";
    s += piece.GetDebugString();
    s += ")";
  }
  s += ")";
  return s;
}

std::string ExpressionPiece::GetSpecialDebugString() const {
  std::ostringstream oss;

  oss << int(special_expression.overload_tag) << ":{";

  bool first = true;
  for (auto const& piece : special_expression.pieces) {
    if (!first) {
      oss << ", ";
    } else {
      first = false;
    }

    oss << piece.GetDebugString();
  }
  oss << "}";

  return oss.str();
}

int ExpressionPiece::PerformUniaryOperationOn(int int_operand) const {
  int result = int_operand;
  switch (uniary_expression.operation) {
    case 0x01:
      result = -int_operand;
      break;
    default:
      break;
  }

  return result;
}

// Stolen from xclannad
// static
int ExpressionPiece::PerformBinaryOperationOn(char operation,
                                              int lhs, int rhs) {
  switch (operation) {
    case 0:
    case 20:
      return lhs + rhs;
    case 1:
    case 21:
      return lhs - rhs;
    case 2:
    case 22:
      return lhs * rhs;
    case 3:
    case 23:
      return rhs != 0 ? lhs / rhs : lhs;
    case 4:
    case 24:
      return rhs != 0 ? lhs % rhs : lhs;
    case 5:
    case 25:
      return lhs & rhs;
    case 6:
    case 26:
      return lhs | rhs;
    case 7:
    case 27:
      return lhs ^ rhs;
    case 8:
    case 28:
      return lhs << rhs;
    case 9:
    case 29:
      return lhs >> rhs;
    case 40:
      return lhs == rhs;
    case 41:
      return lhs != rhs;
    case 42:
      return lhs <= rhs;
    case 43:
      return lhs < rhs;
    case 44:
      return lhs >= rhs;
    case 45:
      return lhs > rhs;
    case 60:
      return lhs && rhs;
    case 61:
      return lhs || rhs;
    default: {
      std::ostringstream ss;
      ss << "Invalid operator "
         << static_cast<int>(operation)
         << " in expression!";
      throw Error(ss.str());
    }
  }
}

// -----------------------------------------------------------------------

void ExpressionPiece::AddContainedPiece(ExpressionPiece piece) {
  switch (piece_type) {
    case TYPE_COMPLEX_EXPRESSION:
      complex_expression.emplace_back(piece);
      break;
    case TYPE_SPECIAL_EXPRESSION:
      special_expression.pieces.emplace_back(piece);
      break;
    default:
      throw Error("Request to AddContainedPiece() invalid!");
  }
}

const std::vector<ExpressionPiece>&
ExpressionPiece::GetContainedPieces() const {
  switch (piece_type) {
    case TYPE_COMPLEX_EXPRESSION:
      return complex_expression;
    case TYPE_SPECIAL_EXPRESSION:
      return special_expression.pieces;
    default:
      throw Error("Request to AddContainedPiece() invalid!");
  }
}

int ExpressionPiece::GetOverloadTag() const {
  switch (piece_type) {
    case TYPE_SPECIAL_EXPRESSION:
      return special_expression.overload_tag;
    default:
      throw Error("Request to GetOverloadTag() invalid!");
  }
}

}  // namespace libreallive
