#ifndef __SDLEventSystem_hpp__
#define __SDLEventSystem_hpp__

#include "Systems/Base/EventSystem.hpp"

#include <SDL/SDL_events.h>

class SDLEventSystem : public EventSystem
{
private:
  bool m_shiftPressed, m_ctrlPressed;

  void handleKeyDown(SDL_Event& e);
  void handleKeyUp(SDL_Event& e);

public:
  SDLEventSystem();

  virtual void executeEventSystem(RLMachine& machine);
  
  virtual bool shiftPressed() const { return m_shiftPressed; }
  virtual bool ctrlPressed() const  { return m_ctrlPressed;  }

  virtual unsigned int getTicks() const;
};

#endif
