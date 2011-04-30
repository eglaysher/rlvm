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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
//
// -----------------------------------------------------------------------

#ifndef SRC_SYSTEMS_BASE_SYSTEM_HPP_
#define SRC_SYSTEMS_BASE_SYSTEM_HPP_

#include <map>
#include <vector>
#include <sstream>
#include <string>
#include <boost/scoped_ptr.hpp>
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

// Syscom Constants
//
// Associations between syscom integer values and their names.
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
const int SYSCOM_CLASSIFY_TEXT = 20;  // ??????? Unknown function.
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

// File type constants.
//
// These constant, externed vectors are passed as parameters to
// findFile to control which file types are searched for. Defaults to
// all.
extern const std::vector<std::string> OBJ_FILETYPES;
extern const std::vector<std::string> IMAGE_FILETYPES;
extern const std::vector<std::string> PDT_IMAGE_FILETYPES;
extern const std::vector<std::string> GAN_FILETYPES;
extern const std::vector<std::string> ANM_FILETYPES;
extern const std::vector<std::string> HIK_FILETYPES;
extern const std::vector<std::string> SOUND_FILETYPES;
extern const std::vector<std::string> KOE_ARCHIVE_FILETYPES;
extern const std::vector<std::string> KOE_LOOSE_FILETYPES;


// Struct containing the global memory to get serialized to disk with
// global memory.
struct SystemGlobals {
  SystemGlobals();

  // Whether we should put up a yes/no dialog box when saving/loading.
  bool confirm_save_load_;

  // I suspect that this is a placebo. I'll track the value, but I don't think
  // it's relevant to anything.
  bool low_priority_;

  // boost::serialization support
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version) {
    ar & confirm_save_load_;

    if (version > 0)
      ar & low_priority_;
  }
};

BOOST_CLASS_VERSION(SystemGlobals, 1)


// The system class provides a generalized interface to all the
// components that make up a local system that may need to be
// implemented differently on different systems, i.e., sound,
// graphics, filesystem et cetera.
//
// The base System class is an abstract base class that is meant to be
// specialized.
class System {
 public:
  System();
  virtual ~System();

  void setPlatform(Platform* platform);
  Platform* platform() { return platform_.get(); }

  // Takes and restores the previous selection snapshot; a special emphemeral
  // save game slot that autosaves on selections and is restored through a
  // special kepago method/syscom call.
  void takeSelectionSnapshot(RLMachine& machine);
  void restoreSelectionSnapshot(RLMachine& machine);

  // Whether certain normal machine operations need to be suspeded due to a
  // native interface being brought up.
  void setSystemPaused(bool paused);
  bool system_paused() const { return system_paused_; }

  // Syscom related functions
  //
  // RealLive provides a context menu system to handle most actions
  // and configuration settings. The system command menu is configured
  // with the \#SYSCOM variables in gameexe.ini. It can be disabled by
  // setting \#SYSCOM_USE to 0, and if a \#CANCELCALL hook is defined it
  // will never be used at all (Clannad does this, although it uses
  // the internal flags associated with the system command menu to
  // control its own menu system).
  //
  // These functions are used to manipulate the visibility, change the
  // values of, and invoke standard dialogs for various SYSCOM elements.

  // Checks the visibility of a single syscom command. Returns 0 if the given
  // system command is invisible, 1 if it is visible, and 2 if it is visible
  // but disabled (greyed out).
  int isSyscomEnabled(int syscom);

  // Hides all syscom entries
  void hideSyscom();

  // Hides the syscom entry |syscom|
  void hideSyscomEntry(int syscom);

  // Enables all syscom entries
  void enableSyscom();

  // Enables the syscom entry |syscom|
  void enableSyscomEntry(int syscom);

  // Disables all syscom entries
  void disableSyscom();

  // Disables the syscom entry |syscom|
  void disableSyscomEntry(int syscom);

  // Reads the corresponding value for syscom number |syscom|
  int readSyscom(int syscom);

  // Called by various LongOperations to show the right click menu.
  void showSyscomMenu(RLMachine& machine);

  // If there is a standard dialog box associated with syscom, it is
  // displayed; if there is a standard action, it is performed. The list of
  // menu commands in section 4.5 has details of which menu commands have
  // standard dialogs. The optional value is used for the setting where
  // relevant (for example, InvokeSyscom(5, val) is exactly equivalent to
  // SetScreenMode(val)).
  void invokeSyscom(RLMachine& machine, int syscom);

  // Shows a screen with certain information about the current state of the
  // interpreter.
  void showSystemInfo(RLMachine& machine);

  bool confirmSaveLoad() const { return globals_.confirm_save_load_; }
  void setConfirmSaveLoad(const int in) { globals_.confirm_save_load_ = in; }

  bool lowPriority() const { return globals_.low_priority_; }
  void setLowPriority(const int in) { globals_.low_priority_ = in; }

  // Finds a file on disk based on its basename with a list of possible
  // extensions, or empty() if file not found.
  boost::filesystem::path findFile(
      const std::string& fileName,
      const std::vector<std::string>& extensions);

  // Resets the present values of the system; this doesn't clear user settings,
  // but clears things like the current graphics state and the status of all
  // the text windows. This method is called when the user loads a game or
  // resets the machine. The System implementation of reset() will call
  // reset() on all systems.
  void reset();

  // Returns the global state for saving/restoring
  SystemGlobals& globals() { return globals_; }

  // Cleans the regname entry from the gameexe and makes it filesystem
  // safe. This translates it to UTF-8, as Gameexe files are written in
  // Shift-JIS.
  std::string regname();

  // Returns a boost::filesystem object which points to the directory
  // where saved game data, preferences, et cetera should be stored
  // for this game.
  //
  // The default implementation returns "~/.rlvm/#{REGNAME}/". A Mac
  // specific override could return "~/Library/Application
  // Support/rlvm/#{REGNAME}/"
  boost::filesystem::path gameSaveDirectory();

  // Testing and Debugging Tools

  // Whether we are zooming through text and events quickly. Currently can be
  // triggered by holding down the control key, or using skip previously read
  // text.
  bool fastForward();

  // Whether we're currently forcing fast forward (only used during game tests
  // to zoom through).
  bool forceFastForward() { return force_fast_forward_; }

  // Set in luaRlvm, to speed through the game with maximum speed!
  void setForceFastForward() { force_fast_forward_ = true; }

  // Renders the screen and dumps a textual representation of the screen.
  void dumpRenderTree(RLMachine& machine);

  bool forceWait() { return force_wait_; }
  void setForceWait(bool in) { force_wait_ = in; }

  // Called once per gameloop.
  virtual void run(RLMachine& machine) = 0;

  // Returns the specific subclasses.
  virtual GraphicsSystem& graphics() = 0;
  virtual EventSystem& event() = 0;
  virtual Gameexe& gameexe() = 0;
  virtual TextSystem& text() = 0;
  virtual SoundSystem& sound() = 0;

 private:
  typedef std::multimap<
   std::string,
   std::pair<std::string, boost::filesystem::path> > FileSystemCache;


  boost::filesystem::path getHomeDirectory();

  // Invokes a custom dialog or the standard one if none present.
  void invokeSaveOrLoad(RLMachine& machine,
                        int syscom,
                        const std::string& mod_key,
                        const std::string& location);

  // Verify that |index| is valid and throw if it isn't.
  void checkSyscomIndex(int index, const char* function);

  // Builds a list of all files that are in a directory specified in the
  // #FOLDNAME part of the Gameexe.ini file.
  void buildFileSystemCache();

  // Recursese on |directory| and adds all filetypes that we can read to our
  // FileSystemCache.
  void addDirectoryToCache(const boost::filesystem::path& directory);

  // The visibility status for all syscom entries
  int syscom_status_[NUM_SYSCOM_ENTRIES];

  // Whether the SYSCOM menu is currently being displayed.
  bool in_menu_;

  // Whether we are being forced to fast forward through the game for testing
  // reasons.
  bool force_fast_forward_;

  // Forces a 10ms sleep at the end of the System::run function. Used to lower
  // CPU usage during manual redrawing.
  bool force_wait_;

  // Whether certain normal operations in the system are paused due do native
  // dialogs.
  bool system_paused_;

  // Cached view of the filesystem, mapping a lowercase filename to an
  // extension and the local file path for that file.
  FileSystemCache filesystem_cache_;

  SystemGlobals globals_;

  // A stream with the save game data at the time of the last selection. Used
  // for the Return to Previous Selection feature.
  boost::shared_ptr<std::stringstream> previous_selection_;

  // Native widget drawer. Can be NULL.
  boost::scoped_ptr<Platform> platform_;

  // Implementation detail which resets in_menu_;
  friend class MenuReseter;

  friend class boost::serialization::access;

  // boost::serialization
  template<class Archive>
  void serialize(Archive& ar, unsigned int version) {
    // For now, does nothing
  }
};

// -----------------------------------------------------------------------

// Returns a version string suitable for printing. Used on the command line
// interface and on the info screen.
std::string rlvm_version();

#endif  // SRC_SYSTEMS_BASE_SYSTEM_HPP_
