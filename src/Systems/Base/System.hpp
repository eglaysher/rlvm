// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006, 2007 Elliot Glaysher
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

#ifndef __System_hpp__
#define __System_hpp__

#include <vector>
#include <string>
#include <boost/serialization/access.hpp>
#include <boost/serialization/version.hpp>
#include <boost/filesystem/path.hpp>

class GraphicsSystem;
class EventSystem;
class TextSystem;
class SoundSystem;
class RLMachine;
class Gameexe;
class GameexeInterpretObject;
class Platform;

// -----------------------------------------------------------------------

/**
 * @name Syscom Constants
 *
 * Associations between syscom integer values and their names.
 *
 * @{
 */
const int NUM_SYSCOM_ENTRIES = 32;
const int SYSCOM_INVISIBLE = 0;
const int SYSCOM_VISIBLE = 1;
const int SYSCOM_GREYED_OUT = 2;

const int SYSCOM_SAVE = 0;
const int SYSCOM_LOAD = 1;
const int SYSCOM_MESSAGE_SPEED = 2;
const int SYSCOM_WINDOW_ATTRIBUTES = 3;
const int SYSCOM_VOLUME_SETTINGS = 4;
const int SYSCOM_SCREEN_MODE = 5;
const int SYSCOM_MISCELLANEOUS_SETTINGS = 6;
// No 7?
const int SYSCOM_VOICE_SETTINGS = 8;
const int SYSCOM_FONT_SELECTION = 9;
const int SYSCOM_BGM_FADE = 10;
const int SYSCOM_BGM_SETTINGS = 11;
const int SYSCOM_WINDOW_DECORATION_STYLE = 12;
const int SYSCOM_AUTO_MODE_SETTINGS = 13;
const int SYSCOM_RETURN_TO_PREVIOUS_SELECTION = 14;
const int SYSCOM_USE_KOE = 15;
const int SYSCOM_DISPLAY_VERSION = 16;
const int SYSCOM_SHOW_WEATHER = 17;
const int SYSCOM_SHOW_OBJECT_1 = 18;
const int SYSCOM_SHOW_OBJECT_2 = 19;
const int SYSCOM_CLASSIFY_TEXT = 20; // ??????? Unknown function.
const int SYSCOM_GENERIC_1 = 21;
const int SYSCOM_GENERIC_2 = 22;
// No 23?
const int SYSCOM_OPEN_MANUAL_PATH = 24;
const int SYSCOM_SET_SKIP_MODE = 25;
const int SYSCOM_AUTO_MODE = 26;
// No 27?
const int SYSCOM_MENU_RETURN = 28;
const int SYSCOM_EXIT_GAME = 29;
const int SYSCOM_HIDE_MENU = 30;
const int SYSCOM_SHOW_BACKGROUND = 31;
/// @}

// -----------------------------------------------------------------------

/**
 * Struct containing the global memory to get serialized to disk with
 * global memory.
 */
struct SystemGlobals
{
  SystemGlobals();

  /// Whether we should put up a yes/no dialog box when saving/loading.
  bool confirm_save_load_;

  /**
   * From the rldev documentation:
   *
   * "This flag is described in the default menu as 'make this program run
   * slower so that other programs will run smoothly'. Its effect is unclear;
   * it does not lower the process priority, but it might cause RealLive to
   * yield control to other processes more frequently."
   *
   * I suspect that this is a placebo. I'll track the value, but I don't think
   * it's relevant to anything.
   */
  bool low_priority_;

  /// boost::serialization support
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version)
  {
    ar & confirm_save_load_;

    if (version > 0)
      ar & low_priority_;
  }
};

BOOST_CLASS_VERSION(SystemGlobals, 1)

// -----------------------------------------------------------------------

/**
 * The system class provides a generalized interface to all the
 * components that make up a local system that may need to be
 * implemented differently on different systems, i.e., sound,
 * graphics, filesystem et cetera.
 *
 * The base System class is an abstract base class that is meant to be
 * specialized.
 *
 * @see SystemGlobals
 */
class System
{
public:
  System();

  virtual ~System() {}

  virtual void run(RLMachine& machine) = 0;

  virtual GraphicsSystem& graphics() = 0;
  virtual EventSystem& event() = 0;
  virtual Gameexe& gameexe() = 0;
  virtual TextSystem& text() = 0;
  virtual SoundSystem& sound() = 0;

  void setPlatform(const boost::shared_ptr<Platform>& platform) {
    platform_ = platform;
  }
  boost::shared_ptr<Platform> platform() { return platform_; }

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
   * values of, and invoke standard dialogs for various SYSCOM elements.
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
  int isSyscomEnabled(int syscom);

  /// Hides all syscom entries
  void hideSyscom();

  /// Hides the syscom entry @c syscom
  void hideSyscomEntry(int syscom);

  /// Enables all syscom entries
  void enableSyscom();

  /// Enables the syscom entry @c syscom
  void enableSyscomEntry(int syscom);

  /// Disables all syscom entries
  void disableSyscom();

  /// Disables the syscom entry @c syscom
  void disableSyscomEntry(int syscom);

  /// Reads the corresponding value for syscom number @c syscom
  int readSyscom(int syscom);

  /// Called by various LongOperations to show the right click menu.
  void showSyscomMenu(RLMachine& machine);

  /// If there is a standard dialog box associated with syscom, it is
  /// displayed; if there is a standard action, it is performed. The list of
  /// menu commands in section 4.5 has details of which menu commands have
  /// standard dialogs. The optional value is used for the setting where
  /// relevant (for example, InvokeSyscom(5, val) is exactly equivalent to
  /// SetScreenMode(val)).
  void invokeSyscom(RLMachine& machine, int syscom);

  /// @}

  /**
   * @name Variables we track here
   *
   * @{
   */
  bool confirmSaveLoad() const { return globals_.confirm_save_load_; }
  void setConfirmSaveLoad(const int in) { globals_.confirm_save_load_ = in; }

  bool lowPriority() const { return globals_.low_priority_; }
  void setLowPriority(const int in) { globals_.low_priority_ = in; }
  /// @}

  const std::vector<boost::filesystem::path>& getSearchPaths();

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
  SystemGlobals& globals() { return globals_; }

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

  /**
   * @name Testing and Debugging Tools
   *
   * @{
   */

  /**
   * Whether we are zooming through text and events quickly. Currently can be
   * triggered by holding down the control key, or using skip previously read
   * text.
   */
  bool fastForward();

  /// Set in luaRlvm, to speed through the game with maximum speed!
  void forceFastForward() { force_fast_forward_ = true; }

  void dumpRenderTree(RLMachine& machine);
  /// @}

  bool forceWait() { return force_wait_; }
  void setForceWait(bool in) { force_wait_ = in; }

protected:
  boost::filesystem::path getHomeDirectory();

private:
  /// The visibility status for all syscom entries
  int syscom_status_[NUM_SYSCOM_ENTRIES];

  /// Whether the SYSCOM menu is currently being displayed.
  bool in_menu_;

  /// Whether we are being forced to fast forward through the game for testing
  /// reasons.
  bool force_fast_forward_;

  /// Forces a 10ms sleep at the end of the System::run function. Used to lower
  /// CPU usage during manual redrawing.
  bool force_wait_;

  void checkSyscomIndex(int index, const char* function);

  std::vector<boost::filesystem::path> cached_search_paths;

  void addPath(GameexeInterpretObject gio);

  SystemGlobals globals_;

  /// Native widget drawer. Can be NULL.
  boost::shared_ptr<Platform> platform_;

  /// Implementation detail which resets in_menu_;
  friend class MenuReseter;

  friend class boost::serialization::access;

  /// boost::serialization
  template<class Archive>
  void serialize(Archive& ar, unsigned int version)
  {
    // For now, does nothing
  }
};

#endif
