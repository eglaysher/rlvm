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
#include <boost/serialization/access.hpp>
#include <boost/filesystem/path.hpp>

class GraphicsSystem;
class EventSystem;
class TextSystem;
class RLMachine;
class Gameexe;
class GameexeInterpretObject;


const int NUM_SYSCOM_ENTRIES = 32;

const int SYSCOM_INVISIBLE = 0;
const int SYSCOM_VISIBLE = 1;
const int SYSCOM_GREYED_OUT = 2;

// -----------------------------------------------------------------------

/**
 * Struct containing the global memory to get serialized to disk with
 */
struct SystemGlobals
{
  SystemGlobals();

  /// Whether we should put up a yes/no dialog box when saving/loading.
  bool m_confirmSaveLoad;

  /// boost::serialization support
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version)
  {
    ar & m_confirmSaveLoad;
  }
};

// -----------------------------------------------------------------------

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
  /// The visibility status for all syscom entries
  int m_syscomStatus[NUM_SYSCOM_ENTRIES];

  void checkSyscomIndex(int index, const char* function);

  std::vector<std::string> cachedSearchPaths;

  void addPath(GameexeInterpretObject gio);

  SystemGlobals m_globals;

  friend class boost::serialization::access;

  /// boost::serialization
  template<class Archive>
  void serialize(Archive& ar, unsigned int version)
  {
    // For now, does nothing
  }

protected:
  boost::filesystem::path getHomeDirectory();

public:
  System();

  virtual ~System() {}

  virtual void run(RLMachine& machine) = 0;

  virtual GraphicsSystem& graphics() = 0;
  virtual EventSystem& event() = 0;
  virtual Gameexe& gameexe() = 0;
  virtual TextSystem& text() = 0;
//  virtual SoundSystem& soundSystem() = 0;

  /**
   * @name Syscom related functions
   * 
   * RealLive provides a context menu system to handle most actions
   * and configuration settings. The system command menu is configured
   * with the \#SYSCOM variables in gameexe.ini. It can be disabled by
   * setting \#SYSCOM_USE to 0, and if a \#CANCELCALL hook is defined it
   * will never be used at all (Clannad does this, although it uses
   * the internal flags associated with the system command menu to
   * control its own menu system).
   *
   * These functions are used to manipulate the visibility, change the
   * values of, and invoke standard dialogs for 
   *
   * @{
   */
  
  /** 
   * Checks the visibility of a single syscom command.
   * 
   * @param syscom The syscom number to check
   * @return Returns 0 if the given system command is invisible, 1 if
   *         it is visible, and 2 if it is visible but disabled
   *         (greyed out).
   */
  bool isSyscomEnabled(int syscom);

  /// Hides all syscom entries
  void hideSyscom();

  /// Hides the syscom entry @c syscom
  void hideSyscom(int syscom);

  /// Enables all syscom entries
  void enableSyscom();

  /// Enables the syscom entry @c syscom
  void enableSyscom(int syscom);

  /// Disables all syscom entries
  void disableSyscom();

  /// Disables the syscom entry @c syscom
  void disableSyscom(int syscom);



  /// @todo Write InvokeSyscom

  /// Reads the corresponding value for syscom number @c syscom
  int readSyscom(int syscom);

  /// @}

  bool confirmSaveLoad() const { return m_globals.m_confirmSaveLoad; }
  void setConfirmSaveLoad(const bool in) { m_globals.m_confirmSaveLoad = in; }

  const std::vector<std::string>& getSearchPaths();

  /**
   * Resets the present values of the system; this doesn't clear user
   * settings, but clears things like the current graphics state and
   * the status of all the text windows. 
   *
   * This method is called when the user loads a game or resets the
   * machine.
   *
   * The System implementation of reset() will call reset() on all
   * systems.
   */
  virtual void reset();

  /// Returns the global state for saving/restoring
  SystemGlobals& globals() { return m_globals; }

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
