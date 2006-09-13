
#include "reference.hpp"
#include "RLMachine.hpp"

IntAccessor::operator int() const
{
  return it->machine->getIntValue(it->type, it->location); 
}

// -----------------------------------------------------------------------

IntAccessor& IntAccessor::operator=(const int newValue) { 
  it->machine->setIntValue(it->type, it->location, newValue);
  return *this;
} 

// -----------------------------------------------------------------------

StringAccessor::operator std::string() const
{
  return it->machine->getStringValue(it->type, it->location);
}

// -----------------------------------------------------------------------

StringAccessor& StringAccessor::operator=(const std::string& newValue) {
  it->machine->setStringValue(it->type, it->location, newValue);
  return *this;
}

// -----------------------------------------------------------------------

bool StringAccessor::operator==(const std::string& rhs) {
  return operator std::string() == rhs;
}
