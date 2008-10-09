// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
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

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

/**
 * @file   expression.cpp
 *
 * @brief Defines functions and structures for representing
 * expressions in Reallive byte code.
 */

#include "libReallive/expression.h"
#include "libReallive/expression_pieces.h"
#include "libReallive/intmemref.h"
#include "MachineBase/reference.hpp"
#include "MachineBase/RLMachine.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

#include <boost/tokenizer.hpp>
#include "defs.h"

using namespace std;

namespace libReallive {

/**
 * @name Expression Tokenization
 *
 * Functions that tokenize expression data while parsing the bytecode
 * to create the BytecodeElements. These functions simply tokenize and
 * mark boundaries; they do not perform any parsing.
 *
 * @{
 */

size_t next_token(const char* src)
{
  if (*src++ != '$') return 0;
  if (*src++ == 0xff) return 6;
  if (*src++ != '[') return 2;
  return 4 + next_expr(src);
}

size_t next_term(const char* src)
{
  if (*src == '(') return 2 + next_expr(src + 1);
  if (*src == '\\') return 2 + next_term(src + 2);
  return next_token(src);
}

size_t next_arith(const char* src)
{
  size_t lhs = next_term(src);
  return (src[lhs] == '\\') ? lhs + 2 + next_arith(src + lhs + 2) : lhs;
}

size_t next_cond(const char* src)
{
  size_t lhs = next_arith(src);
  return (src[lhs] == '\\' && src[lhs + 1] >= 0x28 && src[lhs + 1] <= 0x2d) ?
    lhs + 2 + next_arith(src + lhs + 2) : lhs;
}

size_t next_and(const char* src)
{
  size_t lhs = next_cond(src);
  return (src[lhs] == '\\' && src[lhs + 1] == '<') ?
    lhs + 2 + next_and(src + lhs + 2) : lhs;
}

size_t next_expr(const char* src)
{
  size_t lhs = next_and(src);
  return (src[lhs] == '\\' && src[lhs + 1] == '=') ?
    lhs + 2 + next_expr(src + lhs + 2) : lhs;
}

size_t next_string(const char* src)
{
  bool quoted = false;
  const char* end = src;
  while (true) {
    if (quoted) {
      quoted = *end != '"';
    }
    else {
      quoted = *end == '"';
      if (strncmp(end, "###PRINT(", 9) == 0) {
        end += 9;
        end += 1 + next_expr(end);
        continue;
      }
      if (!((*end >= 0x81 && *end <= 0x9f) || (*end >= 0xe0 && *end <= 0xef)
            || (*end >= 'A'  && *end <= 'Z')  || (*end >= '0'  && *end <= '9')
            || *end == '?' || *end == '_' || *end == '"')) break;
    }
    if ((*end >= 0x81 && *end <= 0x9f) || (*end >= 0xe0 && *end <= 0xef))
      end += 2;
    else
      ++end;
  }
  return end - src;
}

/**
 * @note This still isn't a robust solution. While it takes care of
 *       the CLANNAD errors, it's not general. What's really needed is
 *       a way to tell if something should be a complex parameter or a
 *       parenthisized expression at tokenization time.
 */
size_t next_data(const char* src)
{
  if (*src == ',')
    return 1 + next_data(src + 1);
  if ((*src >= 0x81 && *src <= 0x9f) || (*src >= 0xe0 && *src <= 0xef)
      || (*src >= 'A'  && *src <= 'Z')  || (*src >= '0'  && *src <= '9')
      || *src == '?' || *src == '_' || *src == '"'
      || strcmp(src, "###PRINT(") == 0)
    return next_string(src);
  if (*src == 'a' || *src == '(') {
    const char* end = src;
    if (*end++ == 'a') {
      ++end;
      if (*end != '(') {
        end += next_data(end);
        return end - src;
      } else end++;
    }
    while (*end != ')') end += next_data(end);
    end++;
    if(*end == '\\')
      end += next_expr(end);
    return end - src;
  }
  else return next_expr(src);
}

//@}

// -----------------------------------------------------------------------

/**
 * @name Expression Parsing
 *
 * @author Elliot, but really Haeleth.
 *
 * Functions used at runtime to parse expressions, both as
 * ExpressionPieces, parameters in function calls, and other uses in
 * some special cases. These functions form a recursive descent parser
 * that parses expressions and parameters in Reallive byte code into
 * ExpressionPieces, which are executed with the current RLMachine.
 *
 * These functions were translated from the O'Caml implementation in
 * dissassembler.ml in RLDev, so really, while I coded this, Haeleth
 * really gets all the credit.
 *
 * @see libReallive::ExpressionElement::parsedExpression()
 * @see RLOperation::parseParameters()
 *
 * @{
 */

ExpressionPiece* get_expr_token(const char*& src)
{
  if(src[0] == 0xff) {
    src++;
    int value = read_i32(src);
    src += 4;
    return new IntegerConstant(value);
  } else if(src[0] == 0xc8) {
    src++;
    return new StoreRegisterExpressionPiece();
  } else if((src[0] != 0xc8 && src[0] != 0xff) && src[1] == '[') {
    int type = src[0];
    src += 2;
    ExpressionPiece* location = get_expression(src);

    if(src[0] != ']') {
      ostringstream ss;
      ss << "Unexpected character '" << src[0] << "' in get_expr_token"
         << " (']' expected)";
      throw Error(ss.str());
    }
    src++;

    return new MemoryReference(type, location);
  } else if(src[0] == 0) {
    throw Error("Unexpected end of buffer in get_expr_token");
  } else {
    ostringstream err;
    err << "Unknown toke type 0x" << hex << src[0] << " in get_expr_token" << endl;
    throw Error(err.str());
  }
}

ExpressionPiece* get_expr_term(const char*& src)
{
  if(src[0] == '$') {
    src++;
    return get_expr_token(src);
  } else if(src[0] == '\\' && src[1] == 0x00) {
    src += 2;
    return get_expr_term(src);
  } else if(src[0] == '\\' && src[1] == 0x01) {
    // Uniary -
    src += 2;
    return new UniaryExpressionOperator(0x01, get_expr_term(src));
  } else if(src[0] == '(') {
    src++;
    ExpressionPiece* p = get_expr_bool(src);
    if(src[0] != ')') {
      ostringstream ss;
      ss << "Unexpected character '" << src[0] << "' in get_expr_term"
         << " (')' expected)";
      throw Error(ss.str());
    }
    src++;
    return p;
  } else if(src[0] == 0) {
    throw Error("Unexpected end of buffer in get_expr_term");
  } else {
    ostringstream err;
    err << "Unknown token type 0x" << hex << (short)src[0] << " in get_expr_term";
    throw Error(err.str());
  }
}

static ExpressionPiece* get_expr_arith_loop_hi_prec(const char*& src,
                                                    ExpressionPiece* tok)
{
  if(src[0] == '\\' && src[1] >= 0x02 && src[1] <= 0x09) {
    char op = src[1];
    // Advance past this operator
    src += 2;
    ExpressionPiece* rhs = get_expr_term(src);
    ExpressionPiece* newPiece = new BinaryExpressionOperator(op, tok, rhs);
    return get_expr_arith_loop_hi_prec(src, newPiece);
  } else {
    // We don't consume anything and just return our input token.
    return tok;
  }
}

static ExpressionPiece* get_expr_arith_loop(const char*& src, ExpressionPiece* tok)
{
  if(src[0] == '\\' && (src[1] == 0x00 || src[1] == 0x01)) {
    char op = src[1];
    src += 2;
    ExpressionPiece* other = get_expr_term(src);
    ExpressionPiece* rhs = get_expr_arith_loop_hi_prec(src, other);
    ExpressionPiece* newPiece = new BinaryExpressionOperator(op, tok, rhs);
    return get_expr_arith_loop(src, newPiece);
  } else {
    return tok;
  }
}

ExpressionPiece* get_expr_arith(const char*& src)
{
  return get_expr_arith_loop(src, get_expr_arith_loop_hi_prec(src, get_expr_term(src)));
}

static ExpressionPiece* get_expr_cond_loop(const char*& src, ExpressionPiece* tok)
{
  if(src[0] == '\\' && (src[1] >= 0x28 && src[1] <= 0x2d)) {
    char op = src[1];
    src += 2;
    ExpressionPiece* rhs = get_expr_arith(src);
    ExpressionPiece* newPiece = new BinaryExpressionOperator(op, tok, rhs);
    return get_expr_cond_loop(src, newPiece);
  } else {
    return tok;
  }
}

ExpressionPiece* get_expr_cond(const char*& src)
{
  return get_expr_cond_loop(src, get_expr_arith(src));
}

static ExpressionPiece* get_expr_bool_loop_and(const char*& src, ExpressionPiece* tok)
{
  if(src[0] == '\\' && src[1] == '<') {
    src += 2;
    ExpressionPiece* rhs = get_expr_cond(src);
    return get_expr_bool_loop_and(src, new BinaryExpressionOperator(0x3c, tok, rhs));
  } else {
    return tok;
  }
}

static ExpressionPiece* get_expr_bool_loop_or(const char*& src, ExpressionPiece* tok)
{
  if(src[0] == '\\' && src[1] == '=') {
    src += 2;
    ExpressionPiece* innerTerm = get_expr_cond(src);
    ExpressionPiece* rhs = get_expr_bool_loop_and(src, innerTerm);
    return get_expr_bool_loop_or(src, new BinaryExpressionOperator(0x3d, tok, rhs));
  } else {
    return tok;
  }
}

ExpressionPiece* get_expr_bool(const char*& src)
{
  return get_expr_bool_loop_or(src, get_expr_bool_loop_and(src, get_expr_cond(src)));
}

ExpressionPiece* get_expression(const char*& src)
{
  return get_expr_bool(src);
}

/**
 * Parses an expression of the form [dest] = [source expression];
 *
 * @param src Current location in string to parse
 * @return The parsed ExpressionPiece
 */
ExpressionPiece* get_assignment(const char*& src)
{
  auto_ptr<ExpressionPiece> itok(get_expr_term(src));
  int op = src[1];
  src += 2;
  auto_ptr<ExpressionPiece> etok(get_expression(src));
  if(op >= 0x14 && op <= 0x24) {
    return new AssignmentExpressionOperator(op, itok.release(), etok.release());
  } else {
    throw Error("Undefined assignment in get_assignment");
  }
}

/**
 * Parses a string in the parameter list.
 *
 * @param src Current location in input string to parse
 * @return A StringConstant ExpressionPiece* containg the string.
 */
static ExpressionPiece* get_string(const char*& src)
{
  // Get the length of this string in the bytecode:
  size_t length = next_string(src);

  string s;
  // Check to see if the string is quoted;
  if(src[0] == '"')
    s = string(src + 1, src + length - 1);
  else
    s = string(src, src + length);

  // Increment the source by that many characters
  src += length;

  return new StringConstant(s);
}

/**
 * Parses a parameter in the parameter list. This is the only method
 * of all the get_*(const char*& src) functions that can parse
 * strings. It also deals with things like special and complex
 * parameters.
 *
 * @param src Current location in string to parse
 * @return The parsed ExpressionPiece
 */
ExpressionPiece* get_data(const char*& src)
{
  if(*src == ',') {
    ++src;
    return get_data(src);
  } else if((*src >= 0x81 && *src <= 0x9f)
            || (*src >= 0xe0 && *src <= 0xef)
            || (*src >= 'A'  && *src <= 'Z')
            || (*src >= '0'  && *src <= '9')
            || *src == '?' || *src == '_' || *src == '"'
            || strcmp(src, "###PRINT(") == 0) {
    return get_string(src);
  } else if(*src == 'a') {
    // @todo Cleanup below.
    const char* end = src;
    auto_ptr<ComplexExpressionPiece> cep;

    if (*end++ == 'a') {
      int tag = *end++;
      cep.reset(new SpecialExpressionPiece(tag));

      if (*end != '(')
      {
        // We have a single parameter in this special expression;
        cep->addContainedPiece(get_data(end));
        return cep.release();
      } else end++;
    }
    else
      cep.reset(new ComplexExpressionPiece());

    while (*end != ')') {
      cep->addContainedPiece(get_data(end));
    }

    return cep.release();
  }
  else
    return get_expression(src);
}

// -----------------------------------------------------------------------

ExpressionPiece* get_complex_param(const char*& src)
{
  if(*src == ',') {
    ++src;
    return get_data(src);
  } else if(*src == '(') {
    ++src;
    auto_ptr<ComplexExpressionPiece> cep(new ComplexExpressionPiece());

    while (*src != ')') {
      cep->addContainedPiece(get_data(src));
    }

    return cep.release();
  }
  else
    return get_expression(src);
}


// -----------------------------------------------------------------------

/**
 * Converts a parameter string (as read from the binary SEEN.TXT file)
 * into a human readable (and printable) format.
 *
 * @param src Raw string to turn into a printable string
 * @return Printable string
 */
std::string parsableToPrintableString(const std::string& src)
{
  string output;

  bool firstToken = true;
  for(string::const_iterator it = src.begin(); it != src.end(); ++it)
  {
    if(firstToken)
      firstToken = false;
    else
    {
      output += " ";
    }

    char tok = *it;
    if(tok == '(' || tok == ')' || tok == '$' || tok == '[' || tok == ']')
      output.push_back(tok);
    else
    {
      ostringstream ss;
      ss << std::hex << std::setw(2) << std::setfill('0') << int(tok);
      output += ss.str();
    }
  }

  return output;
}

// -----------------------------------------------------------------------

/**
 * Converts a printable string (i.e., "$ 05 [ $ FF EE 03 00 00 ]")
 * into one that can be parsed by all the get_expr family of functions.
 *
 * @param src Printable string
 * @return Parsable string
 */
std::string printableToParsableString(const std::string& src)
{
  typedef boost::tokenizer<boost::char_separator<char> > ttokenizer;

  std::string output;

  boost::char_separator<char> sep(" ");
  ttokenizer tokens(src, sep);
  for(ttokenizer::iterator it = tokens.begin(); it != tokens.end(); ++it)
  {
    const std::string& tok = *it;
    if(tok == "(" || tok == ")" || tok == "$" || tok == "[" || tok == "]")
      output.push_back(tok[0]);
    else
    {
      char charToAdd;
      istringstream ss(tok);
      ss >> std::hex >> charToAdd;
      output.push_back(charToAdd);
    }
  }

  return output;
}

//@}

// ----------------------------------------------------------------------

ExpressionPiece::~ExpressionPiece() {
}
bool ExpressionPiece::isMemoryReference() const  { return false; }
bool ExpressionPiece::isOperator() const         { return false; }
bool ExpressionPiece::isComplexParameter() const { return false; }
bool ExpressionPiece::isSpecialParamater() const { return false; }

ExpressionValueType ExpressionPiece::expressionValueType() const {
  return ValueTypeInteger;
}

/// A default implementation is provided since not everything will have assign
/// semantics.
void ExpressionPiece::assignIntValue(RLMachine& machine, int rvalue) {}
int ExpressionPiece::integerValue(RLMachine& machine) const
{ throw libReallive::Error("ExpressionPiece::getStringValue() invalid on this object"); }

void ExpressionPiece::assignStringValue(RLMachine& machine) {}
const std::string& ExpressionPiece::getStringValue(RLMachine& machine) const
{ throw libReallive::Error("ExpressionPiece::getStringValue() invalid on this object"); }

// -----------------------------------------------------------------------

bool StoreRegisterExpressionPiece::isMemoryReference() const { return true; }
void StoreRegisterExpressionPiece::assignIntValue(RLMachine& machine, int rvalue) {
  machine.setStoreRegister(rvalue);
}
int StoreRegisterExpressionPiece::integerValue(RLMachine& machine) const {
  return machine.getStoreRegisterValue();
}

ExpressionPiece* StoreRegisterExpressionPiece::clone() const
{
  return new StoreRegisterExpressionPiece;
}

// -----------------------------------------------------------------------

// IntegerConstant
IntegerConstant::IntegerConstant(const int in) : constant(in) {}
IntegerConstant::~IntegerConstant() {}

int IntegerConstant::integerValue(RLMachine& machine) const { return constant; }

ExpressionPiece* IntegerConstant::clone() const
{
  return new IntegerConstant(constant);
}

// -----------------------------------------------------------------------

// StringConstant
StringConstant::StringConstant(const std::string& in) : constant(in) {}
ExpressionValueType StringConstant::expressionValueType() const
{ return ValueTypeString; }
const std::string& StringConstant::getStringValue(RLMachine& machine) const {
  return constant;
}

ExpressionPiece* StringConstant::clone() const
{
  return new StringConstant(constant);
}

// -----------------------------------------------------------------------

// MemoryReference
MemoryReference::MemoryReference(int inType, ExpressionPiece* target)
  : type(inType), location(target) {}
MemoryReference::~MemoryReference() {}

bool MemoryReference::isMemoryReference() const { return true; }
ExpressionValueType MemoryReference::expressionValueType() const {
  if(isStringLocation(type))
  {
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

IntReferenceIterator MemoryReference::getIntegerReferenceIterator(RLMachine& machine) const {
  // Make sure that we are actually referencing an integer
  if(isStringLocation(type)) {
    throw Error("Request to getIntegerReferenceIterator() on a string reference!");
  }

  return IntReferenceIterator(&machine.memory(), type, location->integerValue(machine));
}

StringReferenceIterator MemoryReference::getStringReferenceIterator(RLMachine& machine) const {
  // Make sure that we are actually referencing an integer
  if(!isStringLocation(type)) {
    throw Error("Request to getStringReferenceIterator() on an integer reference!");
  }

  return StringReferenceIterator(&machine.memory(), type, location->integerValue(machine));
}

ExpressionPiece* MemoryReference::clone() const
{
  return new MemoryReference(type, location->clone());
}

// ----------------------------------------------------------------------

UniaryExpressionOperator::UniaryExpressionOperator(char inOperation,
                                                   ExpressionPiece* inOperand)
  : operand(inOperand), operation(inOperation) {}

UniaryExpressionOperator::~UniaryExpressionOperator() {}

int UniaryExpressionOperator::performOperationOn(int int_operand) const
{
  int result = int_operand;
  switch(operation) {
  case 0x01:
    result = - int_operand;
    break;
  default:
    break;
  };

  return result;
}

int UniaryExpressionOperator::integerValue(RLMachine& machine) const {
  return performOperationOn(operand->integerValue(machine));
}

ExpressionPiece* UniaryExpressionOperator::clone() const
{
  return new UniaryExpressionOperator(operation, operand->clone());
}

// ----------------------------------------------------------------------

BinaryExpressionOperator::BinaryExpressionOperator(char inOperation,
                                                   ExpressionPiece* lhs,
                                                   ExpressionPiece* rhs)
  : operation(inOperation), leftOperand(lhs), rightOperand(rhs)
{}

BinaryExpressionOperator::~BinaryExpressionOperator()
{}

// Stolen from xclannad
int BinaryExpressionOperator::performOperationOn(int lhs, int rhs) const
{
  switch(operation) {
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
  case 40: return lhs == rhs;
  case 41: return lhs != rhs;
  case 42: return lhs <= rhs;
  case 43: return lhs <  rhs;
  case 44: return lhs >= rhs;
  case 45: return lhs >  rhs;
  case 60: return lhs && rhs;
  case 61: return lhs || rhs;
  default:
  {
    ostringstream ss;
    ss << "Invalid operator " << (int)operation << " in expression!";
    throw Error(ss.str());
  }
  }
}

int BinaryExpressionOperator::integerValue(RLMachine& machine) const {
  return performOperationOn(leftOperand->integerValue(machine),
                            rightOperand->integerValue(machine));
}

ExpressionPiece* BinaryExpressionOperator::clone() const
{
  return new BinaryExpressionOperator(operation, leftOperand->clone(),
                                      rightOperand->clone());
}

// ----------------------------------------------------------------------

AssignmentExpressionOperator::AssignmentExpressionOperator(char op,
                                                           ExpressionPiece* lhs,
                                                           ExpressionPiece* rhs)
  : BinaryExpressionOperator(op, lhs, rhs)
{}

AssignmentExpressionOperator::~AssignmentExpressionOperator()
{
}

int AssignmentExpressionOperator::integerValue(RLMachine& machine) const
{
  if(operation == 30) {
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

ExpressionPiece* AssignmentExpressionOperator::clone() const
{
  return new AssignmentExpressionOperator(operation, leftOperand->clone(),
                                          rightOperand->clone());
}


// -----------------------------------------------------------------------

bool ComplexExpressionPiece::isComplexParameter() const
{
  return true;
}

// -----------------------------------------------------------------------

/**
 * Adds an ExpressionPiece to this complex expressionPiece. This
 * instance of ComplexExpressionPiece takes ownership of any
 * ExpressionPiece passed in this way.
 *
 * @param piece Piece to pass take ownership of
 */
void ComplexExpressionPiece::addContainedPiece(ExpressionPiece* piece)
{
  containedPieces.push_back(piece);
}

// -----------------------------------------------------------------------

ExpressionPiece* ComplexExpressionPiece::clone() const
{
  ComplexExpressionPiece* cep = new ComplexExpressionPiece;
  cep->containedPieces = containedPieces.clone();
  return cep;
}

// -----------------------------------------------------------------------

SpecialExpressionPiece::SpecialExpressionPiece(int tag)
  : overloadTag(tag)
{}

// -----------------------------------------------------------------------

bool SpecialExpressionPiece::isSpecialParamater() const {
  return true;
}

ExpressionPiece* SpecialExpressionPiece::clone() const
{
  SpecialExpressionPiece* cep = new SpecialExpressionPiece(overloadTag);
  cep->containedPieces = containedPieces.clone();
  return cep;
}


}
