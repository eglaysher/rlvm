
#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <boost/scoped_ptr.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include "libReallive/defs.h"
#include "MachineBase/reference.hpp"

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

/** 
 * Represents the store register in an Expression (both as an lvalue
 * and rvlaue).
 */
class StoreRegisterExpressionPiece : public ExpressionPiece {
public:
  /// StoreRegisterExpressionPiece is a memory reference; returns true
  virtual bool isMemoryReference() const;

  /// Assign the incoming value to the store register of the passed in
  /// machine.
  virtual void assignIntValue(RLMachine& machine, int rvalue);

  /// Returns the store register value of the passed in machine
  virtual int integerValue(RLMachine& machine) const;

  virtual ExpressionPiece* clone() const {
    return new StoreRegisterExpressionPiece;
  }
};

/** 
 * Represents a constant integer in an Expression.
 */
class IntegerConstant : public ExpressionPiece {
private:
  /// The value of this constant
  int constant;
public:
  IntegerConstant(const int in);

  /// Returns the constant value
  virtual int integerValue(RLMachine& machine) const;

  virtual ExpressionPiece* clone() const {
    return new IntegerConstant(constant);
  }
};

class StringConstant : public ExpressionPiece {
private:
  std::string constant;
public:
  StringConstant(const std::string& inStr);

  virtual ExpressionValueType expressionValueType() const;
  virtual const std::string& getStringValue(RLMachine& machine) const;

  virtual ExpressionPiece* clone() const {
    return new StringConstant(constant);
  }
};

// -----------------------------------------------------------------------

/**
 * Reference to a piece of memory in an RLMachine. Noe that this 
 */
class MemoryReference : public ExpressionPiece {
private:
  /* The type of an memory reference refers to both the memory
   * bank we're accessing, and how we're addressing it.
   * @see RLMachine::getIntValue
   * @see RLMachine::getStringValue
   */
  int type;

  /** Arbitrarily complex expression to evaluate at runtime to
   * determine the index of the location we want to address.
   *
   * @see RLMachine::getIntValue
   * @see RLMachine::getStringValue
   */
  boost::scoped_ptr<ExpressionPiece> location;

public:
  MemoryReference(int type, ExpressionPiece* inLoc);

  virtual bool isMemoryReference() const;
  virtual ExpressionValueType expressionValueType() const;

  virtual void assignIntValue(RLMachine& machine, int rvalue);
  virtual int integerValue(RLMachine& machine) const;

  virtual void assignStringValue(RLMachine& machine, const std::string& rvalue);
  virtual const std::string& getStringValue(RLMachine& machine) const;

  IntReferenceIterator getIntegerReferenceIterator(RLMachine& machine) const;
  StringReferenceIterator getStringReferenceIterator(RLMachine& machine) const;

  virtual ExpressionPiece* clone() const {
    return new MemoryReference(type, location->clone());
  }
};

// ----------------------------------------------------------------------

/** 
 * Represents an operation on one ExpressionPiece, i.e. unary
 * negative, et cetera.
 */
class UniaryExpressionOperator : public ExpressionPiece {
private:
  /// The sub-Expression to operate on
  boost::scoped_ptr<ExpressionPiece> operand;

  /// Which operation we are to perform.
  char operation;

  /** 
   * Performs operation on the passed in parameter, and returns the
   * value.
   * 
   * @param x Number to operate on.
   * @return The result of the operation
   */
  int performOperationOn(int x) const;

public:
  UniaryExpressionOperator(char inOperation, ExpressionPiece* inOperand);
  virtual int integerValue(RLMachine& machine) const;

  virtual ExpressionPiece* clone() const {
    return new UniaryExpressionOperator(operation, operand->clone());
  }
};

// ----------------------------------------------------------------------

class BinaryExpressionOperator : public ExpressionPiece {
protected:
  /// The operation to perform
  char operation;

  /// The left operand for this expression
  boost::scoped_ptr<ExpressionPiece> leftOperand;
  /// The right operand for this expression
  boost::scoped_ptr<ExpressionPiece> rightOperand;

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

public:
  BinaryExpressionOperator(char inOperation, ExpressionPiece* lhs,
                           ExpressionPiece* rhs);
  virtual int integerValue(RLMachine& machine) const;

  virtual ExpressionPiece* clone() const {
    return new BinaryExpressionOperator(operation, leftOperand->clone(), 
                                        rightOperand->clone());
  }
};

// ----------------------------------------------------------------------

/** 
 * Operation that modies a given memory location, sucha as +=, -=, /=,
 * et cetera.
 */
class AssignmentExpressionOperator : public BinaryExpressionOperator {
public:
  AssignmentExpressionOperator(char operation, ExpressionPiece* lhs, 
                               ExpressionPiece* rhs);
  ~AssignmentExpressionOperator();

  /** For the entire assignment operator hiearchy, we use integerValue,
   * since it acts as the execute.
   */
  virtual int integerValue(RLMachine& machine) const;

  virtual ExpressionPiece* clone() const {
    return new AssignmentExpressionOperator(operation, leftOperand->clone(), 
                                            rightOperand->clone());
  }
};


// -----------------------------------------------------------------------

/**
 * Represents a Complex parameter to a function. This ExpressionPiece
 * contains multiple ExpressionPieces.
 */
class ComplexExpressionPiece : public ExpressionPiece {
protected:
  /// Poiter owning container of all the ExpressionPieces we compose
  boost::ptr_vector<ExpressionPiece> containedPieces;

public:
  virtual bool isComplexParameter() const;

  void addContainedPiece(ExpressionPiece* piece);

  const boost::ptr_vector<ExpressionPiece>& getContainedPieces() const 
    { return containedPieces; }

  virtual ExpressionPiece* clone() const {
    ComplexExpressionPiece* cep = new ComplexExpressionPiece;
    cep->containedPieces = containedPieces.clone();
    return cep;
  }
};

// -----------------------------------------------------------------------

/** 
 * Represents a Special parameter; a type that can be multiple types. 
 * 
 * In both Haeleth's code and my study of the bytecode, Special
 * parameters seem to be types of Complex expressions. Even if they
 * weren't, we need to maintain this relationships to make Special_T
 * work.
 */
class SpecialExpressionPiece : public ComplexExpressionPiece {
private:
  int overloadTag;

public:
  SpecialExpressionPiece(int tag);
  virtual bool isSpecialParamater() const;

  int getOverloadTag() const { return overloadTag; }

  virtual ExpressionPiece* clone() const {
    SpecialExpressionPiece* cep = new SpecialExpressionPiece(overloadTag);
    cep->containedPieces = containedPieces.clone();
    return cep;
  }
};

}

#endif
