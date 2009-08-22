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

#ifndef SRC_SYSTEMS_BASE_TEXTSYSTEM_HPP_
#define SRC_SYSTEMS_BASE_TEXTSYSTEM_HPP_

#include "Systems/Base/EventListener.hpp"

#include "MachineBase/LongOperation.hpp"

#include <boost/ptr_container/ptr_list.hpp>
#include <boost/ptr_container/ptr_map.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/signal.hpp>
#include <string>
#include <vector>
#include <map>

class Gameexe;
class Memory;
class Point;
class RLMachine;
class Surface;
class System;
class TextKeyCursor;
class TextPage;
class TextWindow;

// -----------------------------------------------------------------------

struct TextSystemGlobals {
  TextSystemGlobals();
  TextSystemGlobals(Gameexe& gexe);

  int auto_mode_base_time;
  int auto_mode_char_time;

  /// Message speed; range from 0 to 255
  char message_speed;

  /// The default \#WINDOW_ATTR. This is what is changed by the
  std::vector<int> window_attr;

  /// boost::serialization support
  template<class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar & auto_mode_base_time & auto_mode_char_time & message_speed
      & window_attr;
  }
};

// -----------------------------------------------------------------------

class TextSystem : public EventListener {
public:
  /// Internal structure used to keep track of the state of
  typedef boost::ptr_map<int, TextPage> PageSet;

public:
  TextSystem(System& system, Gameexe& gexe);
  virtual ~TextSystem();

  /**
   * @name Master visibility control
   *
   * Controls whether the text system is rendered at all.
   *
   * @{
   */
  void setSystemVisible(bool in) { system_visible_ = in; }
  bool systemVisible() const { return system_visible_; }
  /// @}

  /**
   * @name Implementation detail interface
   *
   * @{
   */

  void executeTextSystem();

  void render(std::ostream* tree);
  void hideTextWindow(int win_number);
  void hideAllTextWindows();
  void showTextWindow(int win_number);
  void showAllTextWindows();
  void clearAllTextWindows();
  void setVisualOverride(int win_number, bool show_window);
  void setVisualOverrideAll(bool show_window);
  void clearVisualOverrides();
  virtual boost::shared_ptr<TextWindow> textWindow(int text_window_number) = 0;
  boost::shared_ptr<TextWindow> currentWindow();

  /// @}

  void setInPauseState(bool in) { in_pause_state_ = in; }

  int activeWindow() const { return active_window_; }
  void setActiveWindow(int window) { active_window_ = window; }

  std::vector<int> activeWindows();

  /**
   * Take a snapshot of the current window state, with their
   * respective TextPages, and add it to the backlog.
   */
  void snapshot();

  /**
   * Resets the text page in the current_set
   */
  void newPageOnWindow(int window);

  /**
   * Get the active page. This function will return
   * windows_[active_window_].page().
   */
  TextPage& currentPage();

  /**
   * @name Backlog management
   *
   * @{
   */

  /**
   * Clears the screen, moves back one page and renders it.
   */
  void backPage();
  void forwardPage();

  void replayPageSet(PageSet& set, bool is_current_page);

  bool isReadingBacklog() const;
  void stopReadingBacklog();

  /// @}

  /**
   * @name Auto mode
   *
   * It is possible to set the interpreter up to advance text
   * automatically instead of waiting for player input after each
   * screen is displayed; the `auto mode' controls permit this
   * behaviour to be customized.
   *
   * @{
   */
  void setAutoMode(int i);
  int autoMode() const { return (int)auto_mode_; }
  boost::signal<void(bool)>& autoModeSignal() { return auto_mode_signal_; }

  void setAutoBaseTime(int i) { globals_.auto_mode_base_time = i; }
  int autoBaseTime() const { return globals_.auto_mode_base_time; }

  void setAutoCharTime(int i) { globals_.auto_mode_char_time = i; }
  int autoCharTime() const { return globals_.auto_mode_char_time; }

  int getAutoTime(int num_chars);
  /// @}

  void setKeyCursor(int new_cursor);

  /**
   * Returns the key cursor index.
   *
   * @return The key cursor number (or -1 if no key cursor).
   */
  int cursorNumber() const;

  void setCtrlKeySkip(int i) { ctrl_key_skip_ = i; }
  int ctrlKeySkip() const { return ctrl_key_skip_; }

  void setFastTextMode(int i) { fast_text_mode_ = i; }
  int fastTextMode() const { return fast_text_mode_; }

  void setMessageNoWait(int i) { message_no_wait_ = i; }
  int messageNoWait() const { return message_no_wait_; }

  void setMessageSpeed(int i) { globals_.message_speed = i; }
  int messageSpeed() const { return globals_.message_speed; }

  /**
   * @name Window Attr Related functions
   *
   * @note Any class deriving from TextSystem is responsible for
   *       overriding all the virtual functions in this section, so as
   *       to alert any TextWindow derived objects that it owns that
   *       the default window attr has changed.
   * @{
   */
  void setDefaultWindowAttr(const std::vector<int>& attr);
  std::vector<int> windowAttr() const { return globals_.window_attr; }

  int windowAttrR() const { return globals_.window_attr.at(0); }
  int windowAttrG() const { return globals_.window_attr.at(1); }
  int windowAttrB() const { return globals_.window_attr.at(2); }
  int windowAttrA() const { return globals_.window_attr.at(3); }
  int windowAttrF() const { return globals_.window_attr.at(4); }

  void setWindowAttrR(int i);
  void setWindowAttrG(int i);
  void setWindowAttrB(int i);
  void setWindowAttrA(int i);
  void setWindowAttrF(int i);
  /// @}

  /**
   * @name Window button state
   *
   * @{
   */
  bool windowMoveUse() const { return move_use_; }
  bool windowClearUse() const { return clear_use_; }
  bool windowReadJumpUse() const { return read_jump_use_; }
  bool windowAutomodeUse() const { return automode_use_; }
  bool windowMsgbkUse() const { return msgbk_use_; }
  bool windowMsgbkleftUse() const { return msgbkleft_use_; }
  bool windowMsgbkrightUse() const { return msgbkright_use_; }
  bool windowExbtnUse() const { return exbtn_use_; }

  /// Update the mouse cursor.
  void setMousePosition(const Point& pos);
  bool handleMouseClick(RLMachine& machine, const Point& pos, bool pressed);
  /// @}

  /**
   * @name Font Management and Text Rendering
   *
   * @{
   */
   virtual boost::shared_ptr<Surface> renderText(
       const std::string& utf8str, int size, int xspace,
       int yspace, int colour) = 0;
  /// @}

  TextSystemGlobals& globals() { return globals_; }

  /**
   * Resets non-configuration values (so we can load games).
   */
  virtual void reset();

  bool kidokuRead() const { return kidoku_read_; }
  void setKidokuRead(const int in);
  boost::signal<void(bool)>& skipModeEnabledSignal() {
    return skip_mode_enabled_signal_;
  }

  bool skipMode() const { return skip_mode_; }
  void setSkipMode(const int in);
  boost::signal<void(bool)>& skipModeSignal() { return skip_mode_signal_; }

  bool currentlySkipping() const;

  void setInSelectionMode(const bool in) { in_selection_mode_ = in; }

  // Overriden from EventListener
  virtual bool mouseButtonStateChanged(MouseButton mouse_button, bool pressed);
  virtual bool keyStateChanged(KeyCode key_code, bool pressed);

  System& system() { return system_; }

protected:
  void updateWindowsForChangeToWindowAttr();

  bool showWindow(int win_num) const;

  /// TextPage will call our internals since it actually does most of
  /// the work while we hold state.
  friend class TextPage;

  /**
   * @name Auto mode (variables)
   *
   * @{
   */
  /// Whether Auto mode is enabled
  bool auto_mode_;

  /// Signal to slots who are monitoring whether auto mode is enabled.
  boost::signal<void(bool)> auto_mode_signal_;
  /// @}

  /// Whether holding down the control key will skip text.
  bool ctrl_key_skip_;

  /// Fast text mode
  bool fast_text_mode_;

  /// Internal 'no wait' flag
  bool message_no_wait_;

  /**
   * @name Textwindow Management
   *
   * @{
   */
  /// Sets which window is the current active window.
  int active_window_;

  /// Type of the Window storage
  typedef std::map<int, boost::shared_ptr<TextWindow> > WindowMap;

  /// Storage of active windows
  WindowMap text_window_;
  /// @}

  /**
   * @name Backlog Management
   *
   * @{
   */

  /// Whether we are reading the backlog
  bool is_reading_backlog_;

  /// The current page set. Represents what is on the screen right now.
  std::auto_ptr<PageSet> current_pageset_;

  /// Previous Text Pages. The TextSystem owns the list of previous
  /// pages because multiple windows can be displayed in one text page.
  boost::ptr_list<PageSet> previous_page_sets_;

  /// When previous_page_it_ == previous_pages_.end(), active_page_ is
  /// currently being rendered to the screen. When it is any valid
  /// iterator pointing into previous_pages_, that is the current page
  /// being rendered.
  boost::ptr_list<PageSet>::iterator previous_page_it_;

  /// Whether we are in a state where the interpreter is pause()d.
  bool in_pause_state_;

  /// @}

  boost::shared_ptr<TextKeyCursor> text_key_cursor_;

  /**
   * @name Global Window Button Toggles
   *
   * @{
   */
  bool move_use_, clear_use_, read_jump_use_, automode_use_, msgbk_use_,
    msgbkleft_use_, msgbkright_use_, exbtn_use_;

  void checkAndSetBool(Gameexe& gexe, const std::string& key, bool& out);
  /// @}

  TextSystemGlobals globals_;

  bool system_visible_;

  /// Whether we skip text that we've already seen
  bool skip_mode_;

  /// Signal to slots who are monitoring whether skip mode is enabled.
  boost::signal<void(bool)> skip_mode_signal_;

  /// Whether we are currently on a page of text that we've previously read.
  bool kidoku_read_;

  /// Signal to slots who are monitoring whether skip mode is enabled.
  boost::signal<void(bool)> skip_mode_enabled_signal_;

  /// Whether we are currently paused at a user choice.
  bool in_selection_mode_;

  /// Contains overrides for showing or hiding the text windows.
  std::map<int, bool> window_visual_override_;

  /**
   * Reduces the number of page snapshots in previous_page_sets_ down to a
   * manageable constant number.
   */
  void expireOldPages();

  /// Our parent system object.
  System& system_;

  /// boost::serialization support
  friend class boost::serialization::access;

  template<class Archive>
  void save(Archive & ar, const unsigned int file_version) const;

  template<class Archive>
  void load(Archive& ar, const unsigned int file_version);

  BOOST_SERIALIZATION_SPLIT_MEMBER()
};

// -----------------------------------------------------------------------

int convertNameVar(const std::string& value);

/**
 * Name parser. Takes a raw, local machine encoded string and replaces name
 * variable placeholders with the names from Memory.
 *
 * @note This function assumes that text is in CP932 encoding, and will need to
 *       be generalized when we try to support other hacks on top of cp932.
 * @relates TextSystem
 */
void parseNames(const Memory& memory, const std::string& input,
                std::string& output);

// -----------------------------------------------------------------------

/**
 * LongOperation which just calls text().setSystemVisible(true) and removes
 * itself from the callstack.
 */
struct RestoreTextSystemVisibility : public LongOperation {
  virtual bool operator()(RLMachine& machine);
};

#endif  // SRC_SYSTEMS_BASE_TEXTSYSTEM_HPP_
