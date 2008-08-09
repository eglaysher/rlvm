// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2007 Elliot Glaysher
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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// -----------------------------------------------------------------------

#ifndef __NullSystem_hpp__
#define __NullSystem_hpp__

#include "Systems/Base/System.hpp"
#include "NullSystem/NullGraphicsSystem.hpp"
#include "NullSystem/NullEventSystem.hpp"
#include "NullSystem/NullTextSystem.hpp"
#include "NullSystem/NullSoundSystem.hpp"
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
  Gameexe gameexe_;

  NullGraphicsSystem nullGraphicsSystem;
  NullEventSystem nullEventSystem;
  NullTextSystem nullTextSystem;
  NullSoundSystem nullSoundSystem;

public:
  NullSystem(const std::string& pathToGameexe);
  NullSystem();

  virtual void run(RLMachine& machine);

  virtual GraphicsSystem& graphics();
  virtual EventSystem& event();
  virtual Gameexe& gameexe();
  virtual TextSystem& text();
  virtual SoundSystem& sound();
};

#endif
