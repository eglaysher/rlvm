// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006 Elliot Glaysher
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
//
// -----------------------------------------------------------------------

#ifndef SRC_SYSTEMS_SDL_SDL_SYSTEM_H_
#define SRC_SYSTEMS_SDL_SDL_SYSTEM_H_

#include <memory>

#include "systems/base/system.h"
#include "systems/sdl/sdl_text_system.h"

class SDLGraphicsSystem;
class SDLEventSystem;
class SDLTextSystem;
class SDLSoundSystem;

// -----------------------------------------------------------------------

class SDLSystem : public System {
 public:
  explicit SDLSystem(Gameexe& gameexe);
  virtual ~SDLSystem();

  // Implementation of System:
  virtual void Run(RLMachine& machine) override;
  virtual GraphicsSystem& graphics() override;
  virtual EventSystem& event() override;
  virtual Gameexe& gameexe() override;
  virtual SDLTextSystem& text() override;
  virtual SoundSystem& sound() override;

 private:
  std::unique_ptr<SDLGraphicsSystem> graphics_system_;
  std::unique_ptr<SDLEventSystem> event_system_;
  std::unique_ptr<SDLTextSystem> text_system_;
  std::unique_ptr<SDLSoundSystem> sound_system_;
  Gameexe& gameexe_;
};

// Convenience function to do the casting.
SDLGraphicsSystem* getSDLGraphics(System& system);

#endif  // SRC_SYSTEMS_SDL_SDL_SYSTEM_H_
