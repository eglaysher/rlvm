#ifndef __NullSystem_hpp__
#define __NullSystem_hpp__

#include "Systems/Base/System.hpp"
#include "NullSystem/NullGraphicsSystem.hpp"
#include "NullSystem/NullEventSystem.hpp"
#include "NullSystem/NullTextSystem.hpp"
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
  Gameexe m_gameexe;

  NullGraphicsSystem nullGraphicsSystem;
  NullEventSystem nullEventSystem;
  NullTextSystem nullTextSystem;

public:
  NullSystem() : m_gameexe(), nullEventSystem(m_gameexe), 
                 nullTextSystem(m_gameexe) {}

  virtual void run(RLMachine& machine) { /* do nothing */ }

  virtual GraphicsSystem& graphics() { return nullGraphicsSystem; }
  virtual EventSystem& event() { return nullEventSystem; }
  virtual Gameexe& gameexe() { return m_gameexe; }
  virtual TextSystem& text() { return nullTextSystem; }
};

#endif
