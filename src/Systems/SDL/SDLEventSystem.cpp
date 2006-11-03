
#include "Systems/SDL/SDLEventSystem.hpp"

#include <SDL/SDL.h>

#include <iostream>

using namespace std;

// -----------------------------------------------------------------------
// Private implementation
// -----------------------------------------------------------------------

void SDLEventSystem::handleKeyDown(SDL_Event& e)
{
  switch(e.key.keysym.sym)
  {
    case SDLK_LSHIFT:
    case SDLK_RSHIFT:
    {
      m_shiftPressed = true;
      break;
    }
    case SDLK_LCTRL:
    case SDLK_RCTRL:
    {
      m_ctrlPressed = true;
      break;
    }
  }
}

// -----------------------------------------------------------------------

void SDLEventSystem::handleKeyUp(SDL_Event& e)
{
  switch(e.key.keysym.sym)
  {
    case SDLK_LSHIFT:
    case SDLK_RSHIFT:
    {
      m_shiftPressed = false;
      break;
    }
    case SDLK_LCTRL:
    case SDLK_RCTRL:
    {
      m_ctrlPressed = false;
      break;
    }
  }
}

// -----------------------------------------------------------------------
// Public implementation
// -----------------------------------------------------------------------

SDLEventSystem::SDLEventSystem()
  : m_shiftPressed(false), m_ctrlPressed(false)
{}

// -----------------------------------------------------------------------

void SDLEventSystem::executeEventSystem(RLMachine& machine)
{
  SDL_Event event;

  while(SDL_PollEvent(&event))
  {
    switch(event.type)
    {
    case SDL_KEYDOWN:
      handleKeyDown(event);
      break;
    case SDL_KEYUP:
      handleKeyUp(event);
      break;
    }
  }

//  SDL_Delay(10);
}

// -----------------------------------------------------------------------

unsigned int SDLEventSystem::getTicks() const
{
  return SDL_GetTicks();
}

// -----------------------------------------------------------------------

void SDLEventSystem::wait(unsigned int milliseconds) const
{
  SDL_Delay(milliseconds);
}
