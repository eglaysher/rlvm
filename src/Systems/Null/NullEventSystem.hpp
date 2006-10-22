#ifndef __NullEventSystem_hpp__
#define __NullEventSystem_hpp__

#include "Systems/Base/EventSystem.hpp"

class NullEventSystem : public EventSystem
{
public:
  virtual void executeEventSystem(RLMachine& machine) {}
  virtual bool shiftPressed() const {return false;}
  virtual bool ctrlPressed() const {return false; }
  virtual unsigned int getTicks() const { return 0; }
};

#endif
