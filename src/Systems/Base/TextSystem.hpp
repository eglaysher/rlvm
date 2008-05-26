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

#ifndef __TextSystem_hpp__
#define __TextSystem_hpp__

#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/ptr_container/ptr_map.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/serialization/split_member.hpp>

class Gameexe;
class RLMachine;
class TextWindow;
class TextPage;
class TextKeyCursor;
class Surface;
class Point;

// -----------------------------------------------------------------------

struct TextSystemGlobals
{
  TextSystemGlobals();
  TextSystemGlobals(Gameexe& gexe);

  int autoModeBaseTime;
  int autoModeCharTime;

  /// Message speed; range from 0 to 255
  char messageSpeed;

  /// The default \#WINDOW_ATTR. This is what is changed by the 
  std::vector<int> windowAttr;

  /// boost::serialization support
  template<class Archive>
  void serialize(Archive& ar, const unsigned int version)
  {
    ar & autoModeBaseTime & autoModeCharTime & messageSpeed
      & windowAttr;
  }
};

// -----------------------------------------------------------------------

class TextSystem
{
protected:
  /// TextPage will call our internals since it actually does most of
  /// the work while we hold state.
  friend class TextPage;

  /**
   * @name Auto mode (variables)
   * 
   * @{
   */
  /// Whether Auto mode is enabled
  bool m_autoMode;
  /// @}

  /// Whether holding down the control key will skip text.
  bool m_ctrlKeySkip;

  /// Fast text mode
  bool m_fastTextMode;

  /// Internal 'no wait' flag
  bool m_messageNoWait;

  /** 
   * @name Textwindow Management
   * 
   * @{
   */
  /// Sets which window is the current active window.
  int m_activeWindow;

  /// Type of the Window storage
  typedef boost::ptr_map<int, TextWindow> WindowMap;

  /// Storage of active windows
  WindowMap m_textWindow;
  /// @}

  /**
   * @name Backlog Management
   * 
   * @{
   */

  /// Whether we are reading the backlog
  bool m_isReadingBacklog;

  /// Internal structure used to keep track of the state of 
  typedef boost::ptr_map<int, TextPage> PageSet;

  /// The current page set. Represents what is on the screen right now.
  std::auto_ptr<PageSet> m_currentPageset;

  /// Previous Text Pages. The TextSystem owns the list of previous
  /// pages because multiple windows can be displayed in one text page.
  boost::ptr_vector<PageSet> m_previousPageSets;

  /// When m_previousPageIt == m_previousPages.end(), m_activePage is
  /// currently being rendered to the screen. When it is any valid
  /// iterator pointing into m_previousPages, that is the current page
  /// being rendered.
  boost::ptr_vector<PageSet>::iterator m_previousPageIt;

  /// Whether we are in a state where the interpreter is pause()d.
  bool m_inPauseState;

  /// @}

  boost::shared_ptr<TextKeyCursor> m_textKeyCursor;

  /**
   * @name Global Window Button Toggles
   * 
   * @{
   */
  bool m_moveUse, m_clearUse, m_readJumpUse, m_automodeUse, m_msgbkUse,
    m_msgbkleftUse, m_msgbkrightUse, m_exbtnUse;

  void checkAndSetBool(Gameexe& gexe, const std::string& key, bool& out);
  /// @}

  TextSystemGlobals m_globals;

  bool m_systemVisible;

public:
  TextSystem(Gameexe& gexe);
  virtual ~TextSystem();

  /** 
   * @name Master visibility control
   *
   * Controls whether the text system is rendered at all. 
   *
   * @{
   */
  void setSystemVisible(bool in) { m_systemVisible = in; }
  bool systemVisible() const { return m_systemVisible; }
  /// @}

  /**
   * @name Implementation detail interface
   * 
   * @{
   */

  void executeTextSystem(RLMachine& machine);

  virtual void render(RLMachine& machine) = 0;
  void hideTextWindow(int winNumber);
  void hideAllTextWindows();
  void clearAllTextWindows();
  virtual TextWindow& textWindow(RLMachine&, int textWindowNumber) = 0;
  TextWindow& currentWindow(RLMachine& machine);

  /// @}

  void setInPauseState(bool in) { m_inPauseState = in; }

  int activeWindow() const { return m_activeWindow; }
  void setActiveWindow(int window) { m_activeWindow = window; }

  std::vector<int> activeWindows();

  /** 
   * Take a snapshot of the current window state, with their
   * respective TextPages, and add it to the backlog.
   */
  void snapshot(RLMachine& machine);

  /** 
   * Resets the text page in the currentSet
   */
  void newPageOnWindow(RLMachine& machine, int window);

  /** 
   * Get the active page. This function will return
   * m_windows[m_activeWindow].page().
   */
  TextPage& currentPage(RLMachine& machine);

  /**
   * @name Backlog management
   * 
   * @{
   */

  /**
   * Clears the screen, moves back one page and renders it.
   */
  void backPage(RLMachine& machine);
  void forwardPage(RLMachine& machine);

  void replayPageSet(PageSet& set, bool isCurrentPage);

  bool isReadingBacklog() const;
  void stopReadingBacklog();

  /// @}

  /**
   * @name Auto mode
   * 
   * It is possible to set the interpreter up to advance text
   * automatically instead of waiting for player input after each
   * screen is displayed; the `auto mode' controls permit this
   * behaviour to be customised.
   * 
   * @{
   */
  void setAutoMode(int i) { m_autoMode = (bool)i; }
  int autoMode() const { return (int)m_autoMode; }

  void setAutoBaseTime(int i) { m_globals.autoModeBaseTime = i; }
  int autoBaseTime() const { return m_globals.autoModeBaseTime; }

  void setAutoCharTime(int i) { m_globals.autoModeCharTime = i; }
  int autoCharTime() const { return m_globals.autoModeCharTime; }

  int getAutoTime(int numChars);
  /// @}

  void setKeyCursor(RLMachine& machine, int newCursor);

  /** 
   * Returns the key cursor index.
   * 
   * @return The key cursor number (or -1 if no key cursor).
   */
  int cursorNumber() const;

  void setCtrlKeySkip(int i) { m_ctrlKeySkip = i; }
  int ctrlKeySkip() const { return m_ctrlKeySkip; }

  void setFastTextMode(int i) { m_fastTextMode = i; }
  int fastTextMode() const { return m_fastTextMode; }

  void setMessageNoWait(int i) { m_messageNoWait = i; }
  int messageNoWait() const { return m_messageNoWait; }

  void setMessageSpeed(int i) { m_globals.messageSpeed = i; }
  int messageSpeed() const { return m_globals.messageSpeed; }

  /**
   * @name Window Attr Related functions
   *
   * @note Any class deriving from TextSystem is responsible for
   *       overriding all the virtual functions in this section, so as
   *       to alert any TextWindow derived objects that it owns that
   *       the default window attr has changed.
   * @{
   */
  virtual void setDefaultWindowAttr(const std::vector<int>& attr);
  std::vector<int> windowAttr() const { return m_globals.windowAttr; }

  int windowAttrR() const { return m_globals.windowAttr.at(0); }
  int windowAttrG() const { return m_globals.windowAttr.at(1); }
  int windowAttrB() const { return m_globals.windowAttr.at(2); }
  int windowAttrA() const { return m_globals.windowAttr.at(3); }
  int windowAttrF() const { return m_globals.windowAttr.at(4); }

  virtual void setWindowAttrR(int i) { m_globals.windowAttr.at(0) = i; }
  virtual void setWindowAttrG(int i) { m_globals.windowAttr.at(1) = i; }
  virtual void setWindowAttrB(int i) { m_globals.windowAttr.at(2) = i; }
  virtual void setWindowAttrA(int i) { m_globals.windowAttr.at(3) = i; }
  virtual void setWindowAttrF(int i) { m_globals.windowAttr.at(4) = i; }
  /// @}

  /**
   * @name Window button state
   * 
   * @{
   */
  bool windowMoveUse() const { return m_moveUse; }
  bool windowClearUse() const { return m_clearUse; }
  bool windowReadJumpUse() const { return m_readJumpUse; }
  bool windowAutomodeUse() const { return m_automodeUse; }
  bool windowMsgbkUse() const { return m_msgbkUse; }
  bool windowMsgbkleftUse() const { return m_msgbkleftUse; }
  bool windowMsgbkrightUse() const { return m_msgbkrightUse; }
  bool windowExbtnUse() const { return m_exbtnUse; }

  /// Update the mouse cursor.
  virtual void setMousePosition(RLMachine& machine, const Point& pos) = 0;
  virtual bool handleMouseClick(RLMachine& machine, const Point& pos,
                                bool pressed) = 0;
  /// @}

  /**
   * @name Font Management and Text Rendering
   * 
   * @{
   */
   virtual boost::shared_ptr<Surface> renderText(
	RLMachine& machine, const std::string& utf8str, int size, int xspace,
	int yspace, int colour) = 0;
  /// @}

  TextSystemGlobals& globals() { return m_globals; }

  /**
   * Resets non-configuration values (so we can load games).
   */
  virtual void reset();

  template<class Archive>
  void save(Archive & ar, const unsigned int file_version) const;

  template<class Archive>
  void load(Archive& ar, const unsigned int file_version);

  BOOST_SERIALIZATION_SPLIT_MEMBER()
};

#endif



