#ifndef EXPRESSION_H
#define EXPRESSION_H

#include "defs.h"
#include <boost/scoped_ptr.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include "reference.hpp"

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

enum ExpressionValueType {
  ValueTypeInteger,
  ValueTypeString
};

class ExpressionPiece {
public:
  virtual ~ExpressionPiece();

  virtual bool isMemoryReference() const;
  virtual bool isOperator() const;

  virtual bool isComplexParameter() const;
  virtual bool isSpecialParamater() const;

  virtual ExpressionValueType expressionValueType() const;

  virtual void assignIntValue(RLMachine& machine, int rvalue);
  virtual int getIntegerValue(RLMachine& machine) const;

  virtual void assignStringValue(RLMachine& machine);
  virtual const std::string& getStringValue(RLMachine& machine) const;
};

class StoreRegisterExpressionPiece : public ExpressionPiece {
public:
  virtual bool isMemoryReference() const;
  
  virtual void assignIntValue(RLMachine& machine, int rvalue);
  virtual int getIntegerValue(RLMachine& machine) const;
};

class IntegerConstant : public ExpressionPiece {
private:
  int constant;
public:
  IntegerConstant(const int in);

  virtual int getIntegerValue(RLMachine& machine) const;
};

class StringConstant : public ExpressionPiece {
private:
  std::string constant;
public:
  StringConstant(const std::string& inStr);

  virtual ExpressionValueType expressionValueType() const;
  virtual const std::string& getStringValue(RLMachine& machine) const;
};

// -----------------------------------------------------------------------

/* Reference into the IntA memory
 *
 */
class MemoryReference : public ExpressionPiece {
private:
  /* The type of an memory reference refers to both the memory
   * bank we're accessing, and how we're addressing it.
   * @see RLMachine::getIntValue
   */
  int type;

  /** Arbitrarily complex expression to evaluate at runtime to
   * determine the index of the location we want to address.
   *
   * @see RLMachine::getIntValue
   */
  boost::scoped_ptr<ExpressionPiece> location;

public:
  MemoryReference(int type, ExpressionPiece* inLoc);

  virtual bool isMemoryReference() const;
  virtual ExpressionValueType expressionValueType() const;

  virtual void assignIntValue(RLMachine& machine, int rvalue);
  virtual int getIntegerValue(RLMachine& machine) const;

  virtual void assignStringValue(RLMachine& machine, const std::string& rvalue);
  virtual const std::string& getStringValue(RLMachine& machine) const;

  IntReferenceIterator getIntegerReferenceIterator(RLMachine& machine) const;
  StringReferenceIterator getStringReferenceIterator(RLMachine& machine) const;
};

// ----------------------------------------------------------------------

class UniaryExpressionOperator : public ExpressionPiece {
private:
  boost::scoped_ptr<ExpressionPiece> operand;

  char operation;
  int performOperationOn(int) const;

public:
  UniaryExpressionOperator(char inOperation, ExpressionPiece* inOperand);
  virtual int getIntegerValue(RLMachine& machine) const;
};

// ----------------------------------------------------------------------

class BinaryExpressionOperator : public ExpressionPiece {
protected:
  char operation;
  boost::scoped_ptr<ExpressionPiece> leftOperand;
  boost::scoped_ptr<ExpressionPiece> rightOperand;

  /**
   * To anyone who says that creating a full object hiearchy of all the
   * operations would be more OO, I tried, and it's 20x more misserable.
   */
  int performOperationOn(int lhs, int rhs) const;
public:
  BinaryExpressionOperator(char inOperation, ExpressionPiece* lhs,
                           ExpressionPiece* rhs);
  virtual int getIntegerValue(RLMachine& machine) const;
};

// ----------------------------------------------------------------------

/** 
 * @bug There is some sort of odd double free in either
 * AssignmentExpressionOperator or its superclass. This needs full
 * attention later.
 */
class AssignmentExpressionOperator : public BinaryExpressionOperator {
public:
  AssignmentExpressionOperator(char operation, ExpressionPiece* lhs, 
                               ExpressionPiece* rhs);
  ~AssignmentExpressionOperator();

  /** For the entire assignment operator hiearchy, we use getIntegerValue,
   * since it acts as the execute.
   */
  virtual int getIntegerValue(RLMachine& machine) const;
};


// -----------------------------------------------------------------------

/**
 * Represents a Complex parameter to a function. This ExpressionPiece
 * contains multiple ExpressionPieces.
 */
class ComplexExpressionPiece : public ExpressionPiece {
private:
  /// Poiter owning container of all the ExpressionPieces we compose
  boost::ptr_vector<ExpressionPiece> containedPieces;

public:
  virtual bool isComplexParameter() const;

  void addContainedPiece(ExpressionPiece* piece);

  boost::ptr_vector<ExpressionPiece>& getContainedPieces() 
    { return containedPieces; }
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
};

}

#endif
