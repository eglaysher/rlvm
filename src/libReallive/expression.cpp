/**
 * @file   expression.cpp
 * 
 * @brief Defines functions and structures for representing
 * expressions in Reallive byte code.
 */

#include "libReallive/expression.h"
#include "MachineBase/reference.hpp"
#include "MachineBase/RLMachine.hpp"

#include <iostream>
#include <sstream>

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
	    	if (strcmp(end, "###PRINT(") == 0) {
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
		return end - src + 1;
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
      cerr << "Src: '" << src << "'" << endl;
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
  } else if(*src == 'a' || *src == '(') {
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

//@}

// ----------------------------------------------------------------------

ExpressionPiece::~ExpressionPiece() {
//  cerr << "Destroying expression piece " << this << endl;
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
int ExpressionPiece::integerValue(RLMachine& machine) const {}

void ExpressionPiece::assignStringValue(RLMachine& machine) {}
const std::string& ExpressionPiece::getStringValue(RLMachine& machine) const {}

bool StoreRegisterExpressionPiece::isMemoryReference() const { return true; }
void StoreRegisterExpressionPiece::assignIntValue(RLMachine& machine, int rvalue) {
  machine.setStoreRegister(rvalue);
}
int StoreRegisterExpressionPiece::integerValue(RLMachine& machine) const {
  return machine.getStoreRegisterValue();
}

// IntegerConstant
IntegerConstant::IntegerConstant(const int in) : constant(in) {}
int IntegerConstant::integerValue(RLMachine& machine) const { return constant; }

// StringConstant
StringConstant::StringConstant(const std::string& in) : constant(in) {}
ExpressionValueType StringConstant::expressionValueType() const 
{ return ValueTypeString; }
const std::string& StringConstant::getStringValue(RLMachine& machine) const {
  return constant; 
}


// MemoryReference
MemoryReference::MemoryReference(int inType, ExpressionPiece* target) 
  : type(inType), location(target) {}
bool MemoryReference::isMemoryReference() const { return true; }
ExpressionValueType MemoryReference::expressionValueType() const {
  if(type == 0x12 || type == 0x0A || type == 0x0C) {
    return ValueTypeString;
  } else {
    return ValueTypeInteger;
  }
}

void MemoryReference::assignIntValue(RLMachine& machine, int rvalue) { 
  return machine.setIntValue(type, location->integerValue(machine), rvalue); 
}
int MemoryReference::integerValue(RLMachine& machine) const {
  return machine.getIntValue(type, location->integerValue(machine)); 
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
  if(type == 0x12 || type == 0x0C) {
    throw Error("Request to getIntegerReferenceIterator() on a string reference!");
  }

  return IntReferenceIterator(&machine, type, location->integerValue(machine));
}

StringReferenceIterator MemoryReference::getStringReferenceIterator(RLMachine& machine) const {
  // Make sure that we are actually referencing an integer
  if(!(type == 0x12 || type == 0x0C)) {
    throw Error("Request to getStringReferenceIterator() on an integer reference!");
  }

  return StringReferenceIterator(&machine, type, location->integerValue(machine));
}

// ----------------------------------------------------------------------

UniaryExpressionOperator::UniaryExpressionOperator(char inOperation, 
                                                   ExpressionPiece* inOperand)
  : operation(inOperation), operand(inOperand) {}

int UniaryExpressionOperator::performOperationOn(int int_operand) const
{
  int result;
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

// ----------------------------------------------------------------------

BinaryExpressionOperator::BinaryExpressionOperator(char inOperation,
                                                   ExpressionPiece* lhs,
                                                   ExpressionPiece* rhs) 
  : operation(inOperation), leftOperand(lhs), rightOperand(rhs)
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

// ----------------------------------------------------------------------

AssignmentExpressionOperator::AssignmentExpressionOperator(char op,
                                                           ExpressionPiece* lhs,
                                                           ExpressionPiece* rhs)
  : BinaryExpressionOperator(op, lhs, rhs)
{}

AssignmentExpressionOperator::~AssignmentExpressionOperator()
{
//  cerr << "Destroying AssignmentExpressionOperator(" << this << ")" << endl;
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

SpecialExpressionPiece::SpecialExpressionPiece(int tag)
  : overloadTag(tag)
{}

// -----------------------------------------------------------------------

bool SpecialExpressionPiece::isSpecialParamater() const {
  return true;
}


}
