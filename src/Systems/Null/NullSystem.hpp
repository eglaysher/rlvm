#ifndef __NullSystem_hpp__
#define __NullSystem_hpp__

#include "Systems/Base/System.hpp"
#include "Systems/Null/NullGraphicsSystem.hpp"
#include "Systems/Null/NullEventSystem.hpp"

/** 
 * The Null system contains absolutely no input/ouput 
 * 
 * 
 * @return 
 */
class NullSystem : public System
{
private:
  NullGraphicsSystem nullGraphicsSystem;
  NullEventSystem nullEventSystem;

public:
  virtual void run(RLMachine& machine) { /* do nothing */ }

  virtual GraphicsSystem& graphics() { return nullGraphicsSystem; }
  virtual EventSystem& event() { return nullEventSystem; }
};

#endif
