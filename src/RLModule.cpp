
#include "RLModule.hpp"
#include "RLOperation.hpp"
#include "RLMachine.hpp"

#include <sstream>

using namespace std;
using namespace libReallive;

RLModule::RLModule(const std::string& inModuleName, int inModuleType, 
                   int inModuleNumber)
  : m_moduleType(inModuleType), m_moduleNumber(inModuleNumber), 
    m_moduleName(inModuleName) 
{}

int RLModule::packOpcodeNumber(int opcode, unsigned char overload)
{
  return ((int)opcode << 8) | overload;
}

void RLModule::unpackOpcodeNumber(int packedOpcode, int& opcode, unsigned char& overload)
{
  opcode = (packedOpcode >> 8);
  overload = packedOpcode & 0xFF;
}

void RLModule::addOpcode(int opcode, unsigned char overload, RLOperation* op) 
{
  int packedOpcode = packOpcodeNumber(opcode, overload);
  storedOperations.insert(packedOpcode, op);
}

void RLModule::dispatchFunction(RLMachine& machine, const CommandElement& f) 
{
  OpcodeMap::iterator it = storedOperations.find(packOpcodeNumber(f.opcode(), f.overload()));
  if(it != storedOperations.end()) {
    it->dispatchFunction(machine, f);
  } else {
    stringstream ss;
    ss << "Undefined opcode<" << f.modtype() << ":" << f.module() << ":" << f.opcode() 
       << ", " << f.overload() << ">";
    throw Error(ss.str());
  }
}
