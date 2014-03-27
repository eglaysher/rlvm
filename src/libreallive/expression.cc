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
      quoted = *end != '"';
      if (!quoted && *(end - 1) != '\\') {
        end++;  // consume the final quote
        break;
      }
    } else {
      quoted = *end == '"';
      if (strncmp(end, "###PRINT(", 9) == 0) {
        end += 9;
        end += 1 + NextExpression(end);
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

std::unique_ptr<ExpressionPiece> GetExpressionToken(const char*& src) {
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
    std::unique_ptr<ExpressionPiece> location = GetExpression(src);

    if (src[0] != ']') {
      std::ostringstream ss;
      ss << "Unexpected character '" << src[0] << "' in GetExpressionToken"
         << " (']' expected)";
      throw Error(ss.str());
    }
    src++;

    return std::unique_ptr<ExpressionPiece>(
        new MemoryReference(type, std::move(location)));
  } else if (src[0] == 0) {
    throw Error("Unexpected end of buffer in GetExpressionToken");
  } else {
    std::ostringstream err;
    err << "Unknown toke type 0x" << std::hex << (short)src[0]
        << " in GetExpressionToken" << std::endl;
    throw Error(err.str());
  }
}

std::unique_ptr<ExpressionPiece> GetExpressionTerm(const char*& src) {
  if (src[0] == '$') {
    src++;
    return GetExpressionToken(src);
  } else if (src[0] == '\\' && src[1] == 0x00) {
    src += 2;
    return GetExpressionTerm(src);
  } else if (src[0] == '\\' && src[1] == 0x01) {
    // Uniary -
    src += 2;
    return std::unique_ptr<ExpressionPiece>(
        new UniaryExpressionOperator(0x01, GetExpressionTerm(src)));
  } else if (src[0] == '(') {
    src++;
    std::unique_ptr<ExpressionPiece> p = GetExpressionBoolean(src);
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

static std::unique_ptr<ExpressionPiece> GetExpressionArithmaticLoopHiPrec(
    const char*& src,
    std::unique_ptr<ExpressionPiece> tok) {
  if (src[0] == '\\' && src[1] >= 0x02 && src[1] <= 0x09) {
    char op = src[1];
    // Advance past this operator
    src += 2;
    std::unique_ptr<ExpressionPiece> newPiece(
        new BinaryExpressionOperator(op, std::move(tok), GetExpressionTerm(src)));
    return GetExpressionArithmaticLoopHiPrec(src, std::move(newPiece));
  } else {
    // We don't consume anything and just return our input token.
    return tok;
  }
}

static std::unique_ptr<ExpressionPiece> GetExpressionArithmaticLoop(
    const char*& src,
    std::unique_ptr<ExpressionPiece> tok) {
  if (src[0] == '\\' && (src[1] == 0x00 || src[1] == 0x01)) {
    char op = src[1];
    src += 2;
    std::unique_ptr<ExpressionPiece> other = GetExpressionTerm(src);
    std::unique_ptr<ExpressionPiece> rhs =
        GetExpressionArithmaticLoopHiPrec(src, std::move(other));
    std::unique_ptr<ExpressionPiece> newPiece(
        new BinaryExpressionOperator(op, std::move(tok), std::move(rhs)));
    return GetExpressionArithmaticLoop(src, std::move(newPiece));
  } else {
    return tok;
  }
}

std::unique_ptr<ExpressionPiece> GetExpressionArithmatic(const char*& src) {
  return GetExpressionArithmaticLoop(
      src, GetExpressionArithmaticLoopHiPrec(src, GetExpressionTerm(src)));
}

static std::unique_ptr<ExpressionPiece> GetExpressionConditionLoop(
    const char*& src,
    std::unique_ptr<ExpressionPiece> tok) {
  if (src[0] == '\\' && (src[1] >= 0x28 && src[1] <= 0x2d)) {
    char op = src[1];
    src += 2;
    std::unique_ptr<ExpressionPiece> rhs = GetExpressionArithmatic(src);
    std::unique_ptr<ExpressionPiece> newPiece(
        new BinaryExpressionOperator(op, std::move(tok), std::move(rhs)));
    return GetExpressionConditionLoop(src, std::move(newPiece));
  } else {
    return tok;
  }
}

std::unique_ptr<ExpressionPiece> GetExpressionCondition(const char*& src) {
  return GetExpressionConditionLoop(src, GetExpressionArithmatic(src));
}

static std::unique_ptr<ExpressionPiece> GetExpressionBooleanLoopAnd(
    const char*& src,
    std::unique_ptr<ExpressionPiece> tok) {
  if (src[0] == '\\' && src[1] == '<') {
    src += 2;
    std::unique_ptr<ExpressionPiece> rhs = GetExpressionCondition(src);
    return GetExpressionBooleanLoopAnd(
        src,
        std::unique_ptr<ExpressionPiece>(new BinaryExpressionOperator(
            0x3c, std::move(tok), std::move(rhs))));
  } else {
    return tok;
  }
}

static std::unique_ptr<ExpressionPiece> GetExpressionBooleanLoopOr(
    const char*& src,
    std::unique_ptr<ExpressionPiece> tok) {
  if (src[0] == '\\' && src[1] == '=') {
    src += 2;
    std::unique_ptr<ExpressionPiece> innerTerm = GetExpressionCondition(src);
    std::unique_ptr<ExpressionPiece> rhs =
        GetExpressionBooleanLoopAnd(src, std::move(innerTerm));
    return GetExpressionBooleanLoopOr(
        src,
        std::unique_ptr<ExpressionPiece>(new BinaryExpressionOperator(
            0x3d, std::move(tok), std::move(rhs))));
  } else {
    return tok;
  }
}

std::unique_ptr<ExpressionPiece> GetExpressionBoolean(const char*& src) {
  return GetExpressionBooleanLoopOr(
      src,
      GetExpressionBooleanLoopAnd(src, GetExpressionCondition(src)));
}

std::unique_ptr<ExpressionPiece> GetExpression(const char*& src) {
  return GetExpressionBoolean(src);
}

// Parses an expression of the form [dest] = [source expression];
std::unique_ptr<ExpressionPiece> GetAssignment(const char*& src) {
  std::unique_ptr<ExpressionPiece> itok(GetExpressionTerm(src));
  int op = src[1];
  src += 2;
  std::unique_ptr<ExpressionPiece> etok(GetExpression(src));
  if (op >= 0x14 && op <= 0x24) {
    return std::unique_ptr<ExpressionPiece>(
        new AssignmentExpressionOperator(op, std::move(itok), std::move(etok)));
  } else {
    throw Error("Undefined assignment in GetAssignment");
  }
}

// Parses a string in the parameter list.
static std::unique_ptr<ExpressionPiece> GetString(const char*& src) {
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

  return std::unique_ptr<ExpressionPiece>(new StringConstant(s));
}

// Parses a parameter in the parameter list. This is the only method
// of all the get_*(const char*& src) functions that can parse
// strings. It also deals with things like special and complex
// parameters.
std::unique_ptr<ExpressionPiece> GetData(const char*& src) {
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
        cep->AddContainedPiece(GetData(end));
        return std::unique_ptr<ExpressionPiece>(cep.release());
      } else {
        end++;
      }
    } else {
      cep.reset(new ComplexExpressionPiece());
    }

    while (*end != ')') {
      cep->AddContainedPiece(GetData(end));
    }

    return std::unique_ptr<ExpressionPiece>(cep.release());
  } else {
    return GetExpression(src);
  }
}

std::unique_ptr<ExpressionPiece> GetComplexParam(const char*& src) {
  if (*src == ',') {
    ++src;
    return GetData(src);
  } else if (*src == '(') {
    ++src;
    std::unique_ptr<ComplexExpressionPiece> cep(new ComplexExpressionPiece());

    while (*src != ')') {
      cep->AddContainedPiece(GetData(src));
    }

    return std::unique_ptr<ExpressionPiece>(cep.release());
  } else {
    return GetExpression(src);
  }
}

std::string EvaluatePRINT(RLMachine& machine, const std::string& in) {
  // Currently, this doesn't evaluate the # commands inline. See 5.12.11 of the
  // rldev manual.
  if (boost::starts_with(in, "###PRINT(")) {
    const char* expression_start = in.c_str() + 9;
    std::unique_ptr<ExpressionPiece> piece(GetExpression(expression_start));

    if (*expression_start != ')') {
      std::ostringstream ss;
      ss << "Unexpected character '" << *expression_start
         << "' in evaluatePRINT (')' expected)";
      throw Error(ss.str());
    }

    return piece->GetStringValue(machine);
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

ExpressionPiece::~ExpressionPiece() {}
bool ExpressionPiece::IsMemoryReference() const { return false; }
bool ExpressionPiece::IsOperator() const { return false; }
bool ExpressionPiece::IsAssignment() const { return false; }
bool ExpressionPiece::IsComplexParameter() const { return false; }
bool ExpressionPiece::IsSpecialParameter() const { return false; }

ExpressionValueType ExpressionPiece::GetExpressionValueType() const {
  return ValueTypeInteger;
}

// A default implementation is provided since not everything will have assign
// semantics.
void ExpressionPiece::SetIntegerValue(RLMachine& machine, int rvalue) {}

int ExpressionPiece::GetIntegerValue(RLMachine& machine) const {
  throw libreallive::Error(
      "ExpressionPiece::GetStringValue() invalid on this object");
}

void ExpressionPiece::SetStringValue(RLMachine& machine,
                                        const std::string&) {}
const std::string& ExpressionPiece::GetStringValue(RLMachine& machine) const {
  throw libreallive::Error(
      "ExpressionPiece::GetStringValue() invalid on this object");
}

IntReferenceIterator ExpressionPiece::GetIntegerReferenceIterator(
    RLMachine& machine) const {
  throw libreallive::Error(
      "ExpressionPiece::GetIntegerReferenceIterator() invalid on this object");
}

StringReferenceIterator ExpressionPiece::GetStringReferenceIterator(
    RLMachine& machine) const {
  throw libreallive::Error(
      "ExpressionPiece::GetStringReferenceIterator() invalid on this object");
}

// -----------------------------------------------------------------------

StoreRegisterExpressionPiece::StoreRegisterExpressionPiece() {}

StoreRegisterExpressionPiece::~StoreRegisterExpressionPiece() {}

bool StoreRegisterExpressionPiece::IsMemoryReference() const { return true; }

void StoreRegisterExpressionPiece::SetIntegerValue(RLMachine& machine,
                                                  int rvalue) {
  machine.set_store_register(rvalue);
}

int StoreRegisterExpressionPiece::GetIntegerValue(RLMachine& machine) const {
  return machine.store_register();
}

std::string StoreRegisterExpressionPiece::GetSerializedExpression(RLMachine& machine)
    const {
  return IntToBytecode(machine.store_register());
}

std::string StoreRegisterExpressionPiece::GetDebugString() const {
  return "<store>";
}

IntReferenceIterator StoreRegisterExpressionPiece::GetIntegerReferenceIterator(
    RLMachine& machine) const {
  return IntReferenceIterator(machine.store_register_address());
}

std::unique_ptr<ExpressionPiece> StoreRegisterExpressionPiece::Clone() const {
  return std::unique_ptr<ExpressionPiece>(new StoreRegisterExpressionPiece);
}

// -----------------------------------------------------------------------

// IntegerConstant
IntegerConstant::IntegerConstant(const int in) : constant(in) {}

IntegerConstant::~IntegerConstant() {}

int IntegerConstant::GetIntegerValue(RLMachine& machine) const { return constant; }

std::string IntegerConstant::GetSerializedExpression(RLMachine& machine) const {
  return IntToBytecode(constant);
}

std::string IntegerConstant::GetDebugString() const {
  return std::to_string(constant);
}

std::unique_ptr<ExpressionPiece> IntegerConstant::Clone() const {
  return std::unique_ptr<ExpressionPiece>(new IntegerConstant(constant));
}

// -----------------------------------------------------------------------

// StringConstant
StringConstant::StringConstant(const std::string& in) : constant(in) {}

StringConstant::~StringConstant() {}

ExpressionValueType StringConstant::GetExpressionValueType() const {
  return ValueTypeString;
}

const std::string& StringConstant::GetStringValue(RLMachine& machine) const {
  return constant;
}

std::string StringConstant::GetSerializedExpression(RLMachine& machine) const {
  return string("\"") + constant + string("\"");
}

std::string StringConstant::GetDebugString() const {
  return string("\"") + constant + string("\"");
}

std::unique_ptr<ExpressionPiece> StringConstant::Clone() const {
  return std::unique_ptr<ExpressionPiece>(new StringConstant(constant));
}

// -----------------------------------------------------------------------

// MemoryReference
MemoryReference::MemoryReference(int inType,
                                 std::unique_ptr<ExpressionPiece> target)
    : type(inType), location(std::move(target)) {}

MemoryReference::~MemoryReference() {}

bool MemoryReference::IsMemoryReference() const { return true; }

ExpressionValueType MemoryReference::GetExpressionValueType() const {
  if (isStringLocation(type)) {
    return ValueTypeString;
  } else {
    return ValueTypeInteger;
  }
}

void MemoryReference::SetIntegerValue(RLMachine& machine, int rvalue) {
  return machine.SetIntValue(IntMemRef(type, location->GetIntegerValue(machine)),
                             rvalue);
}

int MemoryReference::GetIntegerValue(RLMachine& machine) const {
  return machine.GetIntValue(IntMemRef(type, location->GetIntegerValue(machine)));
}

void MemoryReference::SetStringValue(RLMachine& machine,
                                        const std::string& rvalue) {
  return machine.SetStringValue(type, location->GetIntegerValue(machine), rvalue);
}

const std::string& MemoryReference::GetStringValue(RLMachine& machine) const {
  return machine.GetStringValue(type, location->GetIntegerValue(machine));
}

std::string MemoryReference::GetSerializedExpression(RLMachine& machine) const {
  if (isStringLocation(type)) {
    return string("\"") + GetStringValue(machine) + string("\"");
  } else {
    return IntToBytecode(GetIntegerValue(machine));
  }
}

std::string MemoryReference::GetDebugString() const {
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

  ret << location->GetDebugString();

  ret << "]";
  return ret.str();
}

IntReferenceIterator MemoryReference::GetIntegerReferenceIterator(
    RLMachine& machine) const {
  // Make sure that we are actually referencing an integer
  if (isStringLocation(type)) {
    throw Error(
        "Request to GetIntegerReferenceIterator() on a string reference!");
  }

  return IntReferenceIterator(
      &machine.memory(), type, location->GetIntegerValue(machine));
}

StringReferenceIterator MemoryReference::GetStringReferenceIterator(
    RLMachine& machine) const {
  // Make sure that we are actually referencing an integer
  if (!isStringLocation(type)) {
    throw Error(
        "Request to GetStringReferenceIterator() on an integer reference!");
  }

  return StringReferenceIterator(
      &machine.memory(), type, location->GetIntegerValue(machine));
}

std::unique_ptr<ExpressionPiece> MemoryReference::Clone() const {
  return std::unique_ptr<ExpressionPiece>(
      new MemoryReference(type, location->Clone()));
}

// ----------------------------------------------------------------------

UniaryExpressionOperator::UniaryExpressionOperator(
    char inOperation,
    std::unique_ptr<ExpressionPiece> inOperand)
    : operand(std::move(inOperand)), operation(inOperation) {}

UniaryExpressionOperator::~UniaryExpressionOperator() {}

int UniaryExpressionOperator::PerformOperationOn(int int_operand) const {
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

int UniaryExpressionOperator::GetIntegerValue(RLMachine& machine) const {
  return PerformOperationOn(operand->GetIntegerValue(machine));
}

std::string UniaryExpressionOperator::GetSerializedExpression(RLMachine& machine)
    const {
  return IntToBytecode(GetIntegerValue(machine));
}

std::string UniaryExpressionOperator::GetDebugString() const {
  std::ostringstream str;
  if (operation == 0x01) {
    str << "-";
  }
  str << operand->GetDebugString();

  return str.str();
}

std::unique_ptr<ExpressionPiece> UniaryExpressionOperator::Clone() const {
  return std::unique_ptr<ExpressionPiece>(
      new UniaryExpressionOperator(operation, operand->Clone()));
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
int BinaryExpressionOperator::PerformOperationOn(int lhs, int rhs) const {
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

int BinaryExpressionOperator::GetIntegerValue(RLMachine& machine) const {
  return PerformOperationOn(leftOperand->GetIntegerValue(machine),
                            rightOperand->GetIntegerValue(machine));
}

std::string BinaryExpressionOperator::GetSerializedExpression(RLMachine& machine)
    const {
  return IntToBytecode(GetIntegerValue(machine));
}

std::string BinaryExpressionOperator::GetDebugString() const {
  std::ostringstream str;
  str << leftOperand->GetDebugString();
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

  if (IsAssignment() && operation != 30) {
    str << "=";
  }

  str << " ";
  str << rightOperand->GetDebugString();

  return str.str();
}

std::unique_ptr<ExpressionPiece> BinaryExpressionOperator::Clone() const {
  return std::unique_ptr<ExpressionPiece>(new BinaryExpressionOperator(
      operation, leftOperand->Clone(), rightOperand->Clone()));
}

// ----------------------------------------------------------------------

AssignmentExpressionOperator::AssignmentExpressionOperator(
    char op,
    std::unique_ptr<ExpressionPiece> lhs,
    std::unique_ptr<ExpressionPiece> rhs)
    : BinaryExpressionOperator(op, std::move(lhs), std::move(rhs)) {}

AssignmentExpressionOperator::~AssignmentExpressionOperator() {}

bool AssignmentExpressionOperator::IsAssignment() const { return true; }

int AssignmentExpressionOperator::GetIntegerValue(RLMachine& machine) const {
  if (operation == 30) {
    int value = rightOperand->GetIntegerValue(machine);
    leftOperand->SetIntegerValue(machine, value);
    return value;
  } else {
    int value = PerformOperationOn(leftOperand->GetIntegerValue(machine),
                                   rightOperand->GetIntegerValue(machine));
    leftOperand->SetIntegerValue(machine, value);
    return value;
  }
}

std::unique_ptr<ExpressionPiece> AssignmentExpressionOperator::Clone() const {
  return std::unique_ptr<ExpressionPiece>(new AssignmentExpressionOperator(
      operation, leftOperand->Clone(), rightOperand->Clone()));
}

// -----------------------------------------------------------------------

ComplexExpressionPiece::ComplexExpressionPiece() {}

ComplexExpressionPiece::~ComplexExpressionPiece() {}

bool ComplexExpressionPiece::IsComplexParameter() const { return true; }

void ComplexExpressionPiece::AddContainedPiece(
    std::unique_ptr<ExpressionPiece> piece) {
  contained_pieces_.push_back(std::move(piece));
}

std::string ComplexExpressionPiece::GetSerializedExpression(RLMachine& machine) const {
  string s("(");
  for (auto const& piece : contained_pieces_) {
    s += "(";
    s += piece->GetSerializedExpression(machine);
    s += ")";
  }
  s += ")";
  return s;
}

std::string ComplexExpressionPiece::GetDebugString() const {
  string s("(");
  for (auto const& piece : contained_pieces_) {
    s += "(";
    s += piece->GetDebugString();
    s += ")";
  }
  s += ")";
  return s;
}

std::unique_ptr<ExpressionPiece> ComplexExpressionPiece::Clone() const {
  ComplexExpressionPiece* cep = new ComplexExpressionPiece;
  for (auto const& piece : contained_pieces_)
    cep->contained_pieces_.push_back(piece->Clone());
  return std::unique_ptr<ExpressionPiece>(cep);
}

// -----------------------------------------------------------------------

SpecialExpressionPiece::SpecialExpressionPiece(int tag) : overload_tag_(tag) {}

SpecialExpressionPiece::~SpecialExpressionPiece() {}

bool SpecialExpressionPiece::IsSpecialParameter() const { return true; }

std::string SpecialExpressionPiece::GetSerializedExpression(RLMachine& machine) const {
  string s("a");
  s += char(overload_tag_);

  if (contained_pieces_.size() > 1)
    s.append("(");
  for (auto const& piece : contained_pieces_) {
    s += piece->GetSerializedExpression(machine);
  }
  if (contained_pieces_.size() > 1)
    s.append(")");

  return s;
}

std::string SpecialExpressionPiece::GetDebugString() const {
  std::ostringstream oss;

  oss << int(overload_tag_) << ":{";

  bool first = true;
  for (auto const& piece : contained_pieces_) {
    if (!first) {
      oss << ", ";
    } else {
      first = false;
    }

    oss << piece->GetDebugString();
  }
  oss << "}";

  return oss.str();
}

std::unique_ptr<ExpressionPiece> SpecialExpressionPiece::Clone() const {
  SpecialExpressionPiece* cep = new SpecialExpressionPiece(overload_tag_);
  for (auto const& piece : contained_pieces_)
    cep->contained_pieces_.push_back(piece->Clone());
  return std::unique_ptr<ExpressionPiece>(cep);
}

}  // namespace libreallive
