// This file is part of RLVM, a RealLive virutal machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006 El Riot
//  
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//  
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//  
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//  
// -----------------------------------------------------------------------

/** 
 * @file
 * @ingroup RLOperationGroup
 * @brief Defines all the base RLOperations and their type checking structs.
 */

#include "MachineBase/RLOperation.hpp"

#include <iostream>
#include <sstream>

using namespace std;
using namespace boost;
using namespace libReallive;

void RLOperation::addParameterTo(const string& parameter, 
                                 ptr_vector<ExpressionPiece>& outputParameters)
{
  const char* location = parameter.c_str();
  outputParameters.push_back(get_data(location));
}

// -----------------------------------------------------------------------

void RLOperation::parseParameters(const CommandElement& ff, 
                                  ptr_vector<ExpressionPiece>& parameterPieces)
{
    size_t numberOfParameters = ff.param_count();
    for(size_t i = 0; i < numberOfParameters; ++i) {
      addParameterTo(ff.get_param(i), parameterPieces);
    }
}

// -----------------------------------------------------------------------

void RLOperation::dispatchFunction(RLMachine& machine, const CommandElement& ff) 
{
  // Well, here it is. What a mess.
  ptr_vector<ExpressionPiece> parameterPieces;
  parseParameters(ff, parameterPieces);

  // Now make sure these parameters match what we expect. 
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

// -----------------------------------------------------------------------

// Implementation for IntConstant_T
bool IntConstant_T::verifyType(boost::ptr_vector<libReallive::ExpressionPiece>& p, 
                               int position) {
  return position < p.size() & 
    p[position].expressionValueType() == libReallive::ValueTypeInteger;
}

// -----------------------------------------------------------------------

bool IntReference_T::verifyType(boost::ptr_vector<libReallive::ExpressionPiece>& p,
                                int position) {
  return position < p.size() && p[position].isMemoryReference() &&
    p[position].expressionValueType() == libReallive::ValueTypeInteger;
}

// -----------------------------------------------------------------------

bool StrConstant_T::verifyType(boost::ptr_vector<libReallive::ExpressionPiece>& p,
                               int position) { 
  return position < p.size() && 
    p[position].expressionValueType() == libReallive::ValueTypeString; 
}

// -----------------------------------------------------------------------

bool StrReference_T::verifyType(boost::ptr_vector<libReallive::ExpressionPiece>& p,
                                   int position) {
  return position < p.size() && p[position].isMemoryReference() &&
    p[position].expressionValueType() == libReallive::ValueTypeString;
}

// -----------------------------------------------------------------------

bool RLOp_SpecialCase::checkTypes(
  RLMachine& machine, 
  boost::ptr_vector<libReallive::ExpressionPiece>& parameters) 
{       
  return true;
}

// -----------------------------------------------------------------------

void RLOp_SpecialCase::dispatch(
  RLMachine& machine, 
  boost::ptr_vector<libReallive::ExpressionPiece>& parameters) 
{}

// -----------------------------------------------------------------------

void RLOp_SpecialCase::dispatchFunction(RLMachine& machine, 
                                        const libReallive::CommandElement& f)
{
  // Pass this on to the implementation of this functor.
  operator()(machine, f);
}
