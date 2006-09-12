/** 
 * @file
 * @ingroup RLOperation
 * @brief Defines all the base RLOperations and their type checking structs.
 */

#include "RLOperation.hpp"

#include <iostream>
#include <sstream>

using namespace std;
using namespace boost;
using namespace Reallive;

void RLOperation::addParameterTo(const string& parameter, 
                                 ptr_vector<ExpressionPiece>& outputParameters)
{
  const char* location = parameter.c_str();
  outputParameters.push_back(get_data(location));
}

void RLOperation::dispatchFunction(RLMachine& machine, const CommandElement& ff) 
{
  // Cast to correct type
  const FunctionElement& f = static_cast<const FunctionElement&>(ff);

  // Well, here it is. What a mess.
  ptr_vector<ExpressionPiece> parameterPieces;
  size_t numberOfParameters = f.param_count();
  for(size_t i = 0; i < numberOfParameters; ++i) {
    addParameterTo(f.get_param(i), parameterPieces);
  }

  // Now make sure these parameters match what we expect. 
  if(!checkTypes(machine, parameterPieces)) {
    throw Error("Expected type mismatch in parameters.");
  }

  // Now dispatch based on these parameters.
  dispatch(machine, parameterPieces);
}

// -----------------------------------------------------------------------

// Implementation for IntConstant_T
bool IntConstant_T::verifyType(boost::ptr_vector<Reallive::ExpressionPiece>& p, int position) {
  return position < p.size() & 
    p[position].expressionValueType() == Reallive::ValueTypeInteger;
}

bool IntReference_T::verifyType(boost::ptr_vector<Reallive::ExpressionPiece>& p,
                                int position) {
  return position < p.size() && p[position].isMemoryReference() &&
    p[position].expressionValueType() == Reallive::ValueTypeInteger;
}

bool StrConstant_T::verifyType(boost::ptr_vector<Reallive::ExpressionPiece>& p, int position) { 
  return position < p.size() && 
    p[position].expressionValueType() == Reallive::ValueTypeString; 
}

bool StrReference_T::verifyType(boost::ptr_vector<Reallive::ExpressionPiece>& p,
                                   int position) {
  return position < p.size() && p[position].isMemoryReference() &&
    p[position].expressionValueType() == Reallive::ValueTypeString;
}

// -----------------------------------------------------------------------

bool RLOp_SpecialCase::checkTypes(
  RLMachine& machine, 
  boost::ptr_vector<Reallive::ExpressionPiece>& parameters) 
{       
}

void RLOp_SpecialCase::dispatch(
  RLMachine& machine, 
  boost::ptr_vector<Reallive::ExpressionPiece>& parameters) 
{
}

void RLOp_SpecialCase::dispatchFunction(RLMachine& machine, 
                                        const Reallive::CommandElement& f)
{
  // Pass this on to the implementation of this functor.
  try {
    operator()(machine, f);
  } catch(std::bad_cast& e) {
    stringstream ss;
    ss << "Type mismatch in op<" << f.modtype() << ":" << f.module() << ":" 
       << f.opcode() << ", " << f.overload() << ">";
    throw Error(ss.str());
  }
}
