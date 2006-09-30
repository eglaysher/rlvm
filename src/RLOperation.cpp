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

void RLOperation::parseParameters(const CommandElement& ff, 
                                  ptr_vector<ExpressionPiece>& parameterPieces)
{
    size_t numberOfParameters = ff.param_count();
    for(size_t i = 0; i < numberOfParameters; ++i) {
      addParameterTo(ff.get_param(i), parameterPieces);
    }
}

void RLOperation::dispatchFunction(RLMachine& machine, const CommandElement& ff) 
{
  try {
    // Well, here it is. What a mess.
    ptr_vector<ExpressionPiece> parameterPieces;
    parseParameters(ff, parameterPieces);

    // Now make sure these parameters match what we expect. 
//    cerr<< "opcode<" << ff.modtype() << ":" << ff.module() << ":" << ff.opcode() 
//        << ", " << ff.overload() << "> " << endl;
    if(!checkTypes(machine, parameterPieces)) {
      throw Error("Expected type mismatch in parameters.");
    }

    // Now dispatch based on these parameters.
    dispatch(machine, parameterPieces);

    // By default, we advacne the instruction pointer on any instruction we
    // perform. Weird special cases all derive from RLOp_SpecialCase, which
    // redefines the dispatcher, so this is ok.
    if(advanceInstructionPointer())
      machine.advanceInstructionPointer();
  }
  catch(...) {
    // If there was an exception on this line, and we are in warning
    // mode, where RLMachine will catch the exception and just log a
    // warning, then an exception will cause an infinite loop if we
    // don't also increment the instruciton pointer.
    // 
    // Note that we don't check to see whether this is a good idea or
    // not; we increment to prevent the infinite loop.
    machine.advanceInstructionPointer();

    throw;
  }
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
  return true;
}

void RLOp_SpecialCase::dispatch(
  RLMachine& machine, 
  boost::ptr_vector<Reallive::ExpressionPiece>& parameters) 
{}

void RLOp_SpecialCase::dispatchFunction(RLMachine& machine, 
                                        const Reallive::CommandElement& f)
{
  // Pass this on to the implementation of this functor.
  try {
    operator()(machine, f);
  } catch(std::bad_cast& e) {
    // Prevent an infinite loop in RLMachine if we are in warning mode
    // so that we don't try to rerun this instruction
    machine.advanceInstructionPointer();

    stringstream ss;
    ss << "Type mismatch in op<" << f.modtype() << ":" << f.module() << ":" 
       << f.opcode() << ", " << f.overload() << ">";
    throw Error(ss.str());
  } catch(...) {
    // Prevent an infinite loop in RLMachine if we are in warning mode
    // so that we don't try to rerun this instruction
    machine.advanceInstructionPointer();

    // Rethrow since we can't handle this
    throw;
  }
}
