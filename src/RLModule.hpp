#ifndef __RLModule_hpp__
#define __RLModule_hpp__

#include <boost/utility.hpp>
#include <boost/ptr_container/ptr_map.hpp>

#include "expression.h"

#include <map>

namespace libReallive {
class FunctionElement;
class CommandElement;
};      

class RLOperation;
class RLMachine;

/** Describes a Module, a grouping of functions. Modules are added to an RLMachine before
 * the machine starts.
 */
class RLModule : public boost::noncopyable {
private:
  int m_moduleType;
  int m_moduleNumber;
  std::string m_moduleName;

  // Store functions. 
  typedef boost::ptr_map<int, RLOperation> OpcodeMap;
  OpcodeMap storedOperations;

  int packOpcodeNumber(int opcode, unsigned char overload);
  void unpackOpcodeNumber(int packedOpcode, int& opcode, unsigned char& overload);

protected:
  RLModule(const std::string& inModuleName, int inModuleType, int inModuleNumber);

  /** Used in derived Module constructors to declare all the 
   *
   * The RLModule class takes ownership of any RLOperation objects passed in this
   * way.
   */
  void addOpcode(int opcode, unsigned char overload, RLOperation*);

public:
  int moduleType() const { return m_moduleType; }
  int moduleNumber() const { return m_moduleNumber; }
  const std::string& moduleName() const { return m_moduleName; }

  void dispatchFunction(RLMachine& machine, const libReallive::CommandElement& f);
};

#endif
