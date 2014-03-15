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

#include <boost/algorithm/string/predicate.hpp>
#include <boost/tokenizer.hpp>

#include <iomanip>
#include <sstream>
#include <string>

#include "libreallive/defs.h"
#include "libreallive/expression_pieces.h"
#include "libreallive/intmemref.h"
#include "machine/reference.h"
#include "machine/rlmachine.h"

namespace {

std::string IntToBytecode(int val) {
  std::string prefix("$\xFF");
  libreallive::append_i32(prefix, val);
  return prefix;
}

}  // namespace

namespace libreallive {

// Expression Tokenization
//
// Functions that tokenize expression data while parsing the bytecode
// to create the BytecodeElements. These functions simply tokenize and
// mark boundaries; they do not perform any parsing.

size_t next_token(const char* src) {
  if (*src++ != '$')
    return 0;
  if (*src++ == 0xff)
    return 6;
  if (*src++ != '[')
    return 2;
  return 4 + next_expr(src);
}

size_t next_term(const char* src) {
  if (*src == '(')
    return 2 + next_expr(src + 1);
  if (*src == '\\')
    return 2 + next_term(src + 2);
  return next_token(src);
}

size_t next_arith(const char* src) {
  size_t lhs = next_term(src);
  return (src[lhs] == '\\') ? lhs + 2 + next_arith(src + lhs + 2) : lhs;
}

size_t next_cond(const char* src) {
  size_t lhs = next_arith(src);
  return (src[lhs] == '\\' && src[lhs + 1] >= 0x28 && src[lhs + 1] <= 0x2d)
             ? lhs + 2 + next_arith(src + lhs + 2)
             : lhs;
}

size_t next_and(const char* src) {
  size_t lhs = next_cond(src);
  return (src[lhs] == '\\' && src[lhs + 1] == '<')
             ? lhs + 2 + next_and(src + lhs + 2)
             : lhs;
}

size_t next_expr(const char* src) {
  size_t lhs = next_and(src);
  return (src[lhs] == '\\' && src[lhs + 1] == '=')
             ? lhs + 2 + next_expr(src + lhs + 2)
             : lhs;
}

size_t next_string(const char* src) {
  bool quoted = false;
  const char* end = src;

  while (true) {
    if (quoted) {
      quoted = *end != '"';
      if (!quoted && *(end - 1) != '\\') {
        end++;  // consume the final quote
        break;
      }
    } else {
      quoted = *end == '"';
      if (strncmp(end, "###PRINT(", 9) == 0) {
        end += 9;
        end += 1 + next_expr(end);
        continue;
      }
      if (!((*end >= 0x81 && *end <= 0x9f) || (*end >= 0xe0 && *end <= 0xef) ||
            (*end >= 'A' && *end <= 'Z') || (*end >= '0' && *end <= '9') ||
            *end == ' ' || *end == '?' || *end == '_' || *end == '"'))
        break;
    }
    if ((*end >= 0x81 && *end <= 0x9f) || (*end >= 0xe0 && *end <= 0xef))
      end += 2;
    else
      ++end;
  }

  return end - src;
}

size_t next_data(const char* src) {
  if (*src == ',')
    return 1 + next_data(src + 1);
  if (*src == '\n')
    return 3 + next_data(src + 3);
  if ((*src >= 0x81 && *src <= 0x9f) || (*src >= 0xe0 && *src <= 0xef) ||
      (*src >= 'A' && *src <= 'Z') || (*src >= '0' && *src <= '9') ||
      *src == ' ' || *src == '?' || *src == '_' || *src == '"' ||
      strcmp(src, "###PRINT(") == 0)
    return next_string(src);
  if (*src == 'a' || *src == '(') {
    const char* end = src;
    if (*end++ == 'a') {
      ++end;

      // Some special cases have multiple tags.
      if (*end == 'a')
        end += 2;

      if (*end != '(') {
        end += next_data(end);
        return end - src;
      } else {
        end++;
      }
    }

    while (*end != ')')
      end += next_data(end);
    end++;
    if (*end == '\\')
      end += next_expr(end);
    return end - src;
  } else {
    return next_expr(src);
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

std::unique_ptr<ExpressionPiece> get_expr_token(const char*& src) {
  if (src[0] == 0xff) {
    src++;
    int value = read_i32(src);
    src += 4;
    return std::unique_ptr<ExpressionPiece>(new IntegerConstant(value));
  } else if (src[0] == 0xc8) {
    src++;
    return std::unique_ptr<ExpressionPiece>(new StoreRegisterExpressionPiece());
  } else if ((src[0] != 0xc8 && src[0] != 0xff) && src[1] == '[') {
    int type = src[0];
    src += 2;
    std::unique_ptr<ExpressionPiece> location = get_expression(src);

    if (src[0] != ']') {
      std::ostringstream ss;
      ss << "Unexpected character '" << src[0] << "' in get_expr_token"
         << " (']' expected)";
      throw Error(ss.str());
    }
    src++;

    return std::unique_ptr<ExpressionPiece>(
        new MemoryReference(type, std::move(location)));
  } else if (src[0] == 0) {
    throw Error("Unexpected end of buffer in get_expr_token");
  } else {
    std::ostringstream err;
    err << "Unknown toke type 0x" << std::hex << (short)src[0]
        << " in get_expr_token" << std::endl;
    throw Error(err.str());
  }
}

std::unique_ptr<ExpressionPiece> get_expr_term(const char*& src) {
  if (src[0] == '$') {
    src++;
    return get_expr_token(src);
  } else if (src[0] == '\\' && src[1] == 0x00) {
    src += 2;
    return get_expr_term(src);
  } else if (src[0] == '\\' && src[1] == 0x01) {
    // Uniary -
    src += 2;
    return std::unique_ptr<ExpressionPiece>(
        new UniaryExpressionOperator(0x01, get_expr_term(src)));
  } else if (src[0] == '(') {
    src++;
    std::unique_ptr<ExpressionPiece> p = get_expr_bool(src);
    if (src[0] != ')') {
      std::ostringstream ss;
      ss << "Unexpected character '" << src[0] << "' in get_expr_term"
         << " (')' expected)";
      throw Error(ss.str());
    }
    src++;
    return p;
  } else if (src[0] == 0) {
    throw Error("Unexpected end of buffer in get_expr_term");
  } else {
    std::ostringstream err;
    err << "Unknown token type 0x" << std::hex << (short)src[0]
        << " in get_expr_term";
    throw Error(err.str());
  }
}

static std::unique_ptr<ExpressionPiece> get_expr_arith_loop_hi_prec(
    const char*& src,
    std::unique_ptr<ExpressionPiece> tok) {
  if (src[0] == '\\' && src[1] >= 0x02 && src[1] <= 0x09) {
    char op = src[1];
    // Advance past this operator
    src += 2;
    std::unique_ptr<ExpressionPiece> newPiece(
        new BinaryExpressionOperator(op, std::move(tok), get_expr_term(src)));
    return get_expr_arith_loop_hi_prec(src, std::move(newPiece));
  } else {
    // We don't consume anything and just return our input token.
    return tok;
  }
}

static std::unique_ptr<ExpressionPiece> get_expr_arith_loop(
    const char*& src,
    std::unique_ptr<ExpressionPiece> tok) {
  if (src[0] == '\\' && (src[1] == 0x00 || src[1] == 0x01)) {
    char op = src[1];
    src += 2;
    std::unique_ptr<ExpressionPiece> other = get_expr_term(src);
    std::unique_ptr<ExpressionPiece> rhs =
        get_expr_arith_loop_hi_prec(src, std::move(other));
    std::unique_ptr<ExpressionPiece> newPiece(
        new BinaryExpressionOperator(op, std::move(tok), std::move(rhs)));
    return get_expr_arith_loop(src, std::move(newPiece));
  } else {
    return tok;
  }
}

std::unique_ptr<ExpressionPiece> get_expr_arith(const char*& src) {
  return get_expr_arith_loop(
      src, get_expr_arith_loop_hi_prec(src, get_expr_term(src)));
}

static std::unique_ptr<ExpressionPiece> get_expr_cond_loop(
    const char*& src,
    std::unique_ptr<ExpressionPiece> tok) {
  if (src[0] == '\\' && (src[1] >= 0x28 && src[1] <= 0x2d)) {
    char op = src[1];
    src += 2;
    std::unique_ptr<ExpressionPiece> rhs = get_expr_arith(src);
    std::unique_ptr<ExpressionPiece> newPiece(
        new BinaryExpressionOperator(op, std::move(tok), std::move(rhs)));
    return get_expr_cond_loop(src, std::move(newPiece));
  } else {
    return tok;
  }
}

std::unique_ptr<ExpressionPiece> get_expr_cond(const char*& src) {
  return get_expr_cond_loop(src, get_expr_arith(src));
}

static std::unique_ptr<ExpressionPiece> get_expr_bool_loop_and(
    const char*& src,
    std::unique_ptr<ExpressionPiece> tok) {
  if (src[0] == '\\' && src[1] == '<') {
    src += 2;
    std::unique_ptr<ExpressionPiece> rhs = get_expr_cond(src);
    return get_expr_bool_loop_and(
        src,
        std::unique_ptr<ExpressionPiece>(new BinaryExpressionOperator(
            0x3c, std::move(tok), std::move(rhs))));
  } else {
    return tok;
  }
}

static std::unique_ptr<ExpressionPiece> get_expr_bool_loop_or(
    const char*& src,
    std::unique_ptr<ExpressionPiece> tok) {
  if (src[0] == '\\' && src[1] == '=') {
    src += 2;
    std::unique_ptr<ExpressionPiece> innerTerm = get_expr_cond(src);
    std::unique_ptr<ExpressionPiece> rhs =
        get_expr_bool_loop_and(src, std::move(innerTerm));
    return get_expr_bool_loop_or(
        src,
        std::unique_ptr<ExpressionPiece>(new BinaryExpressionOperator(
            0x3d, std::move(tok), std::move(rhs))));
  } else {
    return tok;
  }
}

std::unique_ptr<ExpressionPiece> get_expr_bool(const char*& src) {
  return get_expr_bool_loop_or(src,
                               get_expr_bool_loop_and(src, get_expr_cond(src)));
}

std::unique_ptr<ExpressionPiece> get_expression(const char*& src) {
  return get_expr_bool(src);
}

// Parses an expression of the form [dest] = [source expression];
std::unique_ptr<ExpressionPiece> get_assignment(const char*& src) {
  std::unique_ptr<ExpressionPiece> itok(get_expr_term(src));
  int op = src[1];
  src += 2;
  std::unique_ptr<ExpressionPiece> etok(get_expression(src));
  if (op >= 0x14 && op <= 0x24) {
    return std::unique_ptr<ExpressionPiece>(
        new AssignmentExpressionOperator(op, std::move(itok), std::move(etok)));
  } else {
    throw Error("Undefined assignment in get_assignment");
  }
}

// Parses a string in the parameter list.
static std::unique_ptr<ExpressionPiece> get_string(const char*& src) {
  // Get the length of this string in the bytecode:
  size_t length = next_string(src);

  string s;
  // Check to see if the string is quoted;
  if (src[0] == '"')
    s = string(src + 1, src + length - 1);
  else
    s = string(src, src + length);

  // Increment the source by that many characters
  src += length;

  return std::unique_ptr<ExpressionPiece>(new StringConstant(s));
}

// Parses a parameter in the parameter list. This is the only method
// of all the get_*(const char*& src) functions that can parse
// strings. It also deals with things like special and complex
// parameters.
std::unique_ptr<ExpressionPiece> get_data(const char*& src) {
  if (*src == ',') {
    ++src;
    return get_data(src);
  } else if (*src == '\n') {
    src += 3;
    return get_data(src);
  } else if ((*src >= 0x81 && *src <= 0x9f) || (*src >= 0xe0 && *src <= 0xef) ||
             (*src >= 'A' && *src <= 'Z') || (*src >= '0' && *src <= '9') ||
             *src == ' ' || *src == '?' || *src == '_' || *src == '"' ||
             strcmp(src, "###PRINT(") == 0) {
    return get_string(src);
  } else if (*src == 'a') {
    // @todo Cleanup below.
    const char* end = src;
    std::unique_ptr<ComplexExpressionPiece> cep;

    if (*end++ == 'a') {
      int tag = *end++;

      // Some special cases have multiple tags.
      if (*end == 'a') {
        end++;
        int second = *end++;
        tag = (second << 16) | tag;
      }

      cep.reset(new SpecialExpressionPiece(tag));

      if (*end != '(') {
        // We have a single parameter in this special expression;
        cep->addContainedPiece(get_data(end));
        return std::unique_ptr<ExpressionPiece>(cep.release());
      } else {
        end++;
      }
    } else {
      cep.reset(new ComplexExpressionPiece());
    }

    while (*end != ')') {
      cep->addContainedPiece(get_data(end));
    }

    return std::unique_ptr<ExpressionPiece>(cep.release());
  } else {
    return get_expression(src);
  }
}

std::unique_ptr<ExpressionPiece> get_complex_param(const char*& src) {
  if (*src == ',') {
    ++src;
    return get_data(src);
  } else if (*src == '(') {
    ++src;
    std::unique_ptr<ComplexExpressionPiece> cep(new ComplexExpressionPiece());

    while (*src != ')') {
      cep->addContainedPiece(get_data(src));
    }

    return std::unique_ptr<ExpressionPiece>(cep.release());
  } else {
    return get_expression(src);
  }
}

std::string evaluatePRINT(RLMachine& machine, const std::string& in) {
  // Currently, this doesn't evaluate the # commands inline. See 5.12.11 of the
  // rldev manual.
  if (boost::starts_with(in, "###PRINT(")) {
    const char* expression_start = in.c_str() + 9;
    std::unique_ptr<ExpressionPiece> piece(get_expression(expression_start));

    if (*expression_start != ')') {
      std::ostringstream ss;
      ss << "Unexpected character '" << *expression_start
         << "' in evaluatePRINT (')' expected)";
      throw Error(ss.str());
    }

    return piece->getStringValue(machine);
  } else {
    // Just a normal string we can ignore
    return in;
  }
}

std::string parsableToPrintableString(const std::string& src) {
  string output;

  bool firstToken = true;
  for (string::const_iterator it = src.begin(); it != src.end(); ++it) {
    if (firstToken) {
      firstToken = false;
    } else {
      output += " ";
    }

    char tok = *it;
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

std::string printableToParsableString(const std::string& src) {
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

ExpressionPiece::~ExpressionPiece() {}
bool ExpressionPiece::isMemoryReference() const { return false; }
bool ExpressionPiece::isOperator() const { return false; }
bool ExpressionPiece::isAssignment() const { return false; }
bool ExpressionPiece::isComplexParameter() const { return false; }
bool ExpressionPiece::isSpecialParamater() const { return false; }

ExpressionValueType ExpressionPiece::expressionValueType() const {
  return ValueTypeInteger;
}

// A default implementation is provided since not everything will have assign
// semantics.
void ExpressionPiece::assignIntValue(RLMachine& machine, int rvalue) {}

int ExpressionPiece::integerValue(RLMachine& machine) const {
  throw libreallive::Error(
      "ExpressionPiece::getStringValue() invalid on this object");
}

void ExpressionPiece::assignStringValue(RLMachine& machine,
                                        const std::string&) {}
const std::string& ExpressionPiece::getStringValue(RLMachine& machine) const {
  throw libreallive::Error(
      "ExpressionPiece::getStringValue() invalid on this object");
}

IntReferenceIterator ExpressionPiece::getIntegerReferenceIterator(
    RLMachine& machine) const {
  throw libreallive::Error(
      "ExpressionPiece::getIntegerReferenceIterator() invalid on this object");
}

StringReferenceIterator ExpressionPiece::getStringReferenceIterator(
    RLMachine& machine) const {
  throw libreallive::Error(
      "ExpressionPiece::getStringReferenceIterator() invalid on this object");
}

// -----------------------------------------------------------------------

bool StoreRegisterExpressionPiece::isMemoryReference() const { return true; }

void StoreRegisterExpressionPiece::assignIntValue(RLMachine& machine,
                                                  int rvalue) {
  machine.setStoreRegister(rvalue);
}

int StoreRegisterExpressionPiece::integerValue(RLMachine& machine) const {
  return machine.getStoreRegisterValue();
}

std::string StoreRegisterExpressionPiece::serializedValue(RLMachine& machine)
    const {
  return IntToBytecode(machine.getStoreRegisterValue());
}

std::string StoreRegisterExpressionPiece::getDebugValue(RLMachine& machine)
    const {
  return std::to_string(machine.getStoreRegisterValue());
}

std::string StoreRegisterExpressionPiece::getDebugString() const {
  return "<store>";
}

IntReferenceIterator StoreRegisterExpressionPiece::getIntegerReferenceIterator(
    RLMachine& machine) const {
  return IntReferenceIterator(machine.storeRegisterAddress());
}

std::unique_ptr<ExpressionPiece> StoreRegisterExpressionPiece::clone() const {
  return std::unique_ptr<ExpressionPiece>(new StoreRegisterExpressionPiece);
}

// -----------------------------------------------------------------------

// IntegerConstant
IntegerConstant::IntegerConstant(const int in) : constant(in) {}

IntegerConstant::~IntegerConstant() {}

int IntegerConstant::integerValue(RLMachine& machine) const { return constant; }

std::string IntegerConstant::serializedValue(RLMachine& machine) const {
  return IntToBytecode(constant);
}

std::string IntegerConstant::getDebugValue(RLMachine& machine) const {
  return std::to_string(constant);
}

std::string IntegerConstant::getDebugString() const {
  return std::to_string(constant);
}

std::unique_ptr<ExpressionPiece> IntegerConstant::clone() const {
  return std::unique_ptr<ExpressionPiece>(new IntegerConstant(constant));
}

// -----------------------------------------------------------------------

// StringConstant
StringConstant::StringConstant(const std::string& in) : constant(in) {}

ExpressionValueType StringConstant::expressionValueType() const {
  return ValueTypeString;
}

const std::string& StringConstant::getStringValue(RLMachine& machine) const {
  return constant;
}

std::string StringConstant::serializedValue(RLMachine& machine) const {
  return string("\"") + constant + string("\"");
}

std::string StringConstant::getDebugValue(RLMachine& machine) const {
  return serializedValue(machine);
}

std::string StringConstant::getDebugString() const {
  return string("\"") + constant + string("\"");
}

std::unique_ptr<ExpressionPiece> StringConstant::clone() const {
  return std::unique_ptr<ExpressionPiece>(new StringConstant(constant));
}

// -----------------------------------------------------------------------

// MemoryReference
MemoryReference::MemoryReference(int inType,
                                 std::unique_ptr<ExpressionPiece> target)
    : type(inType), location(std::move(target)) {}

MemoryReference::~MemoryReference() {}

bool MemoryReference::isMemoryReference() const { return true; }

ExpressionValueType MemoryReference::expressionValueType() const {
  if (isStringLocation(type)) {
    return ValueTypeString;
  } else {
    return ValueTypeInteger;
  }
}

void MemoryReference::assignIntValue(RLMachine& machine, int rvalue) {
  return machine.setIntValue(IntMemRef(type, location->integerValue(machine)),
                             rvalue);
}

int MemoryReference::integerValue(RLMachine& machine) const {
  return machine.getIntValue(IntMemRef(type, location->integerValue(machine)));
}

void MemoryReference::assignStringValue(RLMachine& machine,
                                        const std::string& rvalue) {
  return machine.setStringValue(type, location->integerValue(machine), rvalue);
}

const std::string& MemoryReference::getStringValue(RLMachine& machine) const {
  return machine.getStringValue(type, location->integerValue(machine));
}

std::string MemoryReference::serializedValue(RLMachine& machine) const {
  if (isStringLocation(type)) {
    return string("\"") + getStringValue(machine) + string("\"");
  } else {
    return IntToBytecode(integerValue(machine));
  }
}

std::string MemoryReference::getDebugValue(RLMachine& machine) const {
  if (isStringLocation(type)) {
    return string("\"") + getStringValue(machine) + string("\"");
  } else {
    return std::to_string(integerValue(machine));
  }
}

std::string MemoryReference::getDebugString() const {
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

  ret << location->getDebugString();

  ret << "]";
  return ret.str();
}

IntReferenceIterator MemoryReference::getIntegerReferenceIterator(
    RLMachine& machine) const {
  // Make sure that we are actually referencing an integer
  if (isStringLocation(type)) {
    throw Error(
        "Request to getIntegerReferenceIterator() on a string reference!");
  }

  return IntReferenceIterator(
      &machine.memory(), type, location->integerValue(machine));
}

StringReferenceIterator MemoryReference::getStringReferenceIterator(
    RLMachine& machine) const {
  // Make sure that we are actually referencing an integer
  if (!isStringLocation(type)) {
    throw Error(
        "Request to getStringReferenceIterator() on an integer reference!");
  }

  return StringReferenceIterator(
      &machine.memory(), type, location->integerValue(machine));
}

std::unique_ptr<ExpressionPiece> MemoryReference::clone() const {
  return std::unique_ptr<ExpressionPiece>(
      new MemoryReference(type, location->clone()));
}

// ----------------------------------------------------------------------

UniaryExpressionOperator::UniaryExpressionOperator(
    char inOperation,
    std::unique_ptr<ExpressionPiece> inOperand)
    : operand(std::move(inOperand)), operation(inOperation) {}

UniaryExpressionOperator::~UniaryExpressionOperator() {}

int UniaryExpressionOperator::performOperationOn(int int_operand) const {
  int result = int_operand;
  switch (operation) {
    case 0x01:
      result = -int_operand;
      break;
    default:
      break;
  }

  return result;
}

int UniaryExpressionOperator::integerValue(RLMachine& machine) const {
  return performOperationOn(operand->integerValue(machine));
}

std::string UniaryExpressionOperator::serializedValue(RLMachine& machine)
    const {
  return IntToBytecode(integerValue(machine));
}

std::string UniaryExpressionOperator::getDebugValue(RLMachine& machine) const {
  return std::to_string(integerValue(machine));
}

std::string UniaryExpressionOperator::getDebugString() const {
  std::ostringstream str;
  if (operation == 0x01) {
    str << "-";
  }
  str << operand->getDebugString();

  return str.str();
}

std::unique_ptr<ExpressionPiece> UniaryExpressionOperator::clone() const {
  return std::unique_ptr<ExpressionPiece>(
      new UniaryExpressionOperator(operation, operand->clone()));
}

// ----------------------------------------------------------------------

BinaryExpressionOperator::BinaryExpressionOperator(
    char inOperation,
    std::unique_ptr<ExpressionPiece> lhs,
    std::unique_ptr<ExpressionPiece> rhs)
    : operation(inOperation),
      leftOperand(std::move(lhs)),
      rightOperand(std::move(rhs)) {}

BinaryExpressionOperator::~BinaryExpressionOperator() {}

// Stolen from xclannad
int BinaryExpressionOperator::performOperationOn(int lhs, int rhs) const {
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
      ss << "Invalid operator " << (int)operation << " in expression!";
      throw Error(ss.str());
    }
  }
}

int BinaryExpressionOperator::integerValue(RLMachine& machine) const {
  return performOperationOn(leftOperand->integerValue(machine),
                            rightOperand->integerValue(machine));
}

std::string BinaryExpressionOperator::serializedValue(RLMachine& machine)
    const {
  return IntToBytecode(integerValue(machine));
}

std::string BinaryExpressionOperator::getDebugValue(RLMachine& machine) const {
  return std::to_string(integerValue(machine));
}

std::string BinaryExpressionOperator::getDebugString() const {
  std::ostringstream str;
  str << leftOperand->getDebugString();
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
      ss << "Invalid operator " << (int)operation << " in expression!";
      throw Error(ss.str());
    }
  }

  if (isAssignment() && operation != 30) {
    str << "=";
  }

  str << " ";
  str << rightOperand->getDebugString();

  return str.str();
}

std::unique_ptr<ExpressionPiece> BinaryExpressionOperator::clone() const {
  return std::unique_ptr<ExpressionPiece>(new BinaryExpressionOperator(
      operation, leftOperand->clone(), rightOperand->clone()));
}

// ----------------------------------------------------------------------

AssignmentExpressionOperator::AssignmentExpressionOperator(
    char op,
    std::unique_ptr<ExpressionPiece> lhs,
    std::unique_ptr<ExpressionPiece> rhs)
    : BinaryExpressionOperator(op, std::move(lhs), std::move(rhs)) {}

AssignmentExpressionOperator::~AssignmentExpressionOperator() {}

bool AssignmentExpressionOperator::isAssignment() const { return true; }

int AssignmentExpressionOperator::integerValue(RLMachine& machine) const {
  if (operation == 30) {
    int value = rightOperand->integerValue(machine);
    leftOperand->assignIntValue(machine, value);
    return value;
  } else {
    int value = performOperationOn(leftOperand->integerValue(machine),
                                   rightOperand->integerValue(machine));
    leftOperand->assignIntValue(machine, value);
    return value;
  }
}

std::string AssignmentExpressionOperator::getDebugValue(RLMachine& machine)
    const {
  return "<assignment>";
}

std::unique_ptr<ExpressionPiece> AssignmentExpressionOperator::clone() const {
  return std::unique_ptr<ExpressionPiece>(new AssignmentExpressionOperator(
      operation, leftOperand->clone(), rightOperand->clone()));
}

// -----------------------------------------------------------------------

bool ComplexExpressionPiece::isComplexParameter() const { return true; }

void ComplexExpressionPiece::addContainedPiece(
    std::unique_ptr<ExpressionPiece> piece) {
  contained_pieces_.push_back(std::move(piece));
}

std::string ComplexExpressionPiece::serializedValue(RLMachine& machine) const {
  string s("(");
  for (auto const& piece : contained_pieces_) {
    s += "(";
    s += piece->serializedValue(machine);
    s += ")";
  }
  s += ")";
  return s;
}

std::string ComplexExpressionPiece::getDebugValue(RLMachine& machine) const {
  return "<complex value>";
}

std::string ComplexExpressionPiece::getDebugString() const {
  string s("(");
  for (auto const& piece : contained_pieces_) {
    s += "(";
    s += piece->getDebugString();
    s += ")";
  }
  s += ")";
  return s;
}

std::unique_ptr<ExpressionPiece> ComplexExpressionPiece::clone() const {
  ComplexExpressionPiece* cep = new ComplexExpressionPiece;
  for (auto const& piece : contained_pieces_)
    cep->contained_pieces_.push_back(piece->clone());
  return std::unique_ptr<ExpressionPiece>(cep);
}

// -----------------------------------------------------------------------

SpecialExpressionPiece::SpecialExpressionPiece(int tag) : overloadTag(tag) {}

bool SpecialExpressionPiece::isSpecialParamater() const { return true; }

std::string SpecialExpressionPiece::serializedValue(RLMachine& machine) const {
  string s("a");
  s += char(overloadTag);

  if (contained_pieces_.size() > 1)
    s.append("(");
  for (auto const& piece : contained_pieces_) {
    s += piece->serializedValue(machine);
  }
  if (contained_pieces_.size() > 1)
    s.append(")");

  return s;
}

std::string SpecialExpressionPiece::getDebugValue(RLMachine& machine) const {
  std::ostringstream oss;

  oss << int(overloadTag) << ":{";

  bool first = true;

  for (auto const& piece : contained_pieces_) {
    if (!first) {
      oss << ", ";
    } else {
      first = false;
    }

    oss << piece->getDebugValue(machine);
  }
  oss << "}";

  return oss.str();
}

std::string SpecialExpressionPiece::getDebugString() const {
  std::ostringstream oss;

  oss << int(overloadTag) << ":{";

  bool first = true;
  for (auto const& piece : contained_pieces_) {
    if (!first) {
      oss << ", ";
    } else {
      first = false;
    }

    oss << piece->getDebugString();
  }
  oss << "}";

  return oss.str();
}

std::unique_ptr<ExpressionPiece> SpecialExpressionPiece::clone() const {
  SpecialExpressionPiece* cep = new SpecialExpressionPiece(overloadTag);
  for (auto const& piece : contained_pieces_)
    cep->contained_pieces_.push_back(piece->clone());
  return std::unique_ptr<ExpressionPiece>(cep);
}

}  // namespace libreallive
