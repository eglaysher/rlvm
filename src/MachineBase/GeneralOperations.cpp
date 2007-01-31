
#include <boost/ptr_container/ptr_vector.hpp>

#include "MachineBase/GeneralOperations.hpp"
#include "MachineBase/RLMachine.hpp"
#include "libReallive/defs.h"

using namespace boost;
using namespace libReallive;

void MultiDispatch::operator()(RLMachine& machine, const libReallive::CommandElement& ff) 
{
  ptr_vector<ExpressionPiece> parameterPieces;
  parseParameters(ff, parameterPieces);

  for(int i = 0; i < parameterPieces.size(); ++i) {
    ptr_vector<ExpressionPiece>& element = 
      dynamic_cast<ComplexExpressionPiece&>(parameterPieces[i]).getContainedPieces();

    if(!handler->checkTypes(machine, element)) {
      throw Error("Expected type mismatch in parameters in MultiDispatch.");
    }

    handler->dispatch(machine, element);
  }

  machine.advanceInstructionPointer();
}
