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

#include <boost/ptr_container/ptr_vector.hpp>

#include "MachineBase/GeneralOperations.hpp"
#include "MachineBase/RLMachine.hpp"
#include "libReallive/defs.h"

using namespace std;
using namespace boost;
using namespace libReallive;

MultiDispatch::MultiDispatch(RLOperation* op) 
  : handler(op)
{}

// -----------------------------------------------------------------------

MultiDispatch::~MultiDispatch()
{}

// -----------------------------------------------------------------------

void MultiDispatch::parseParameters(
  const std::vector<std::string>& input,
  boost::ptr_vector<libReallive::ExpressionPiece>& output)
{
  for(vector<string>::const_iterator it = input.begin(); it != input.end();
      ++it)
  {
    const char* src = it->c_str();
    output.push_back(get_complex_param(src));
  }
}

// -----------------------------------------------------------------------

/// @todo Port this up to the new expression handling code
void MultiDispatch::operator()(
  RLMachine& machine, 
  const libReallive::CommandElement& ff) 
{
  const ptr_vector<ExpressionPiece>& parameterPieces = ff.getParameters();

  for(unsigned int i = 0; i < parameterPieces.size(); ++i) {
    const ptr_vector<ExpressionPiece>& element = 
      dynamic_cast<const ComplexExpressionPiece&>(parameterPieces[i]).getContainedPieces();

    // @todo Do whatever is needed to get this part working...
//     if(!handler->checkTypes(machine, element)) {
//       throw Error("Expected type mismatch in parameters in MultiDispatch.");
//     }

    handler->dispatch(machine, element);
  }

  machine.advanceInstructionPointer();
}
