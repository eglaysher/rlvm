#ifndef __NullSystem_hpp__
#define __NullSystem_hpp__

#include "Systems/Base/System.hpp"
#include "Systems/Null/NullGraphicsSystem.hpp"
#include "Systems/Null/NullEventSystem.hpp"
#include "libReallive/gameexe.h"

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
  Gameexe m_gameexe;

public:
  virtual void run(RLMachine& machine) { /* do nothing */ }

  virtual GraphicsSystem& graphics() { return nullGraphicsSystem; }
  virtual EventSystem& event() { return nullEventSystem; }
  virtual Gameexe& gameexe() { return m_gameexe; }
  virtual TextSystem& text() { throw "whatever"; }
};

#endif
