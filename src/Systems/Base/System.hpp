// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006, 2007 Elliot Glaysher
//  
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
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

#ifndef __System_hpp__
#define __System_hpp__

#include <vector>
#include <string>

class GraphicsSystem;
class EventSystem;
class TextSystem;
class RLMachine;
class Gameexe;
class GameexeInterpretObject;

namespace boost { namespace filesystem { class path; } }

/**
 * The system class provides a generalized interface to all the
 * components that make up a local system that may need to be
 * implemented differently on different systems, i.e., sound,
 * graphics, filesystem et cetera.
 *
 * The base System class is an abstract base class that is meant to be
 * specialized by the 
 */
class System
{
private:
  std::vector<std::string> cachedSearchPaths;

  void addPath(GameexeInterpretObject gio);

protected:
  boost::filesystem::path System::getHomeDirectory();

public:
  virtual ~System() {}

  virtual void run(RLMachine& machine) = 0;

  virtual GraphicsSystem& graphics() = 0;
  virtual EventSystem& event() = 0;
  virtual Gameexe& gameexe() = 0;
  virtual TextSystem& text() = 0;
//  virtual SoundSystem& soundSystem() = 0;

  const std::vector<std::string>& getSearchPaths();

  /**
   * Returns a boost::filesystem object which points to the directory
   * where saved game data, preferences, et cetera should be stored
   * for this game.
   *
   * The default implementation returns "~/.rlvm/#{REGNAME}/". A Mac
   * specific override could return "~/Library/Application
   * Support/rlvm/#{REGNAME}/"
   */
  virtual boost::filesystem::path gameSaveDirectory();
};

#endif
