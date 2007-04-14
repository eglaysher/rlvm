// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006 El Riot
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

#ifndef __TextSystem_hpp__
#define __TextSystem_hpp__

#include <string>

#include <boost/ptr_container/ptr_vector.hpp>

class Gameexe;
class RLMachine;
class TextWindow;
class TextPage;
class TextKeyCursor;

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

  int m_autoModeBaseTime;
  int m_autoModeCharTime;

  /// @}

  /// Whether holding down the control key will skip text.
  bool m_ctrlKeySkip;

  /// Fast text mode
  bool m_fastTextMode;

  /// Internal 'no wait' flag
  bool m_messageNoWait;

  /// Message speed; range from 0 to 255
  char m_messageSpeed;

  /// Default text window to render to. We need this to initialize new
  /// pages.
  int m_defaultTextWindow;

  /// Previous Text Pages. The TextSystem owns the list of previous
  /// pages because multiple windows can be displayed in one text page.
  boost::ptr_vector<TextPage> m_previousPages;

  /// When m_previousPageIt == m_previousPages.end(), m_activePage is
  /// currently being rendered to the screen. When it is any valid
  /// iterator pointing into m_previousPages, that is the current page
  /// being rendered.
  boost::ptr_vector<TextPage>::iterator m_previousPageIt;

  /// The current text page. 
  std::auto_ptr<TextPage> m_activePage;

  /// Whether we are in a state where the interpreter is pause()d.
  bool m_inPauseState;

  boost::shared_ptr<TextKeyCursor> m_textKeyCursor;

  /// The default \#WINDOW_ATTR. This is what is changed by the 
  std::vector<int> m_windowAttr;

  /**
   * @name Global Window Button Toggles
   * 
   * @{
   */
  bool m_moveUse, m_clearUse, m_readJumpUse, m_automodeUse, m_msgbkUse,
    m_msgbkleftUse, m_msgbkrightUse, m_exbtnUse;

  void checkAndSetBool(Gameexe& gexe, const std::string& key, bool& out);
  /// @}

public:
  TextSystem(Gameexe& gexe);
  virtual ~TextSystem();

  /**
   * @name Implementation detail interface
   * 
   * @{
   */

  virtual void executeTextSystem(RLMachine& machine) = 0;

  virtual void render(RLMachine& machine) = 0;
  virtual void hideTextWindow(int winNumber) = 0;
  virtual void hideAllTextWindows() = 0;
  virtual void clearAllTextWindows() = 0;
  virtual TextWindow& textWindow(RLMachine&, int textWindowNumber) = 0;

  /// @}

  void setInPauseState(bool in) { m_inPauseState = in; }

  int defaultWindow() const { return m_defaultTextWindow; }
  void setDefaultWindow(int window) { m_defaultTextWindow = window; }

  /** 
   * Get the active page. This function will always return
   * m_activePage, instead of getting whatever page is currently being
   * rendered to the screen.
   */
  TextPage& currentPage(RLMachine& machine);

  /** 
   * Adds the current page to the backlog, and puts a new TextPage
   * object as the current active page, along with some setup commands.
   */
  void newPage(RLMachine& machine);

  /**
   * @name Backlog management
   * 
   * @{
   */

  /**
   * Cleares the screen, moves back one page and renders it.
   */
  void backPage(RLMachine& machine);
  void forwardPage(RLMachine& machine);

  
  bool isReadingBacklog() const;

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
  void setAutoMode(int i) { m_autoMode = i; }
  int autoMode() const { return m_autoMode; }

  void setAutoBaseTime(int i) { m_autoModeBaseTime = i; }
  int autoBaseTime() const { return m_autoModeBaseTime; }

  void setAutoCharTime(int i) { m_autoModeCharTime = i; }
  int autoCharTime() const { return m_autoModeCharTime; }

  int getAutoTime(int numChars);
  /// @}

  void setKeyCursor(RLMachine& machine, int newCursor);

  void setCtrlKeySkip(int i) { m_ctrlKeySkip = i; }
  int ctrlKeySkip() const { return m_ctrlKeySkip; }

  void setFastTextMode(int i) { m_fastTextMode = i; }
  int fastTextMode() const { return m_fastTextMode; }

  void setMessageNoWait(int i) { m_messageNoWait = i; }
  int messageNoWait() const { return m_messageNoWait; }

  void setMessageSpeed(int i) { m_messageSpeed = i; }
  int messageSpeed() const { return m_messageSpeed; }

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
  std::vector<int> windowAttr() const { return m_windowAttr; }

  int windowAttrR() const { return m_windowAttr.at(0); }
  int windowAttrG() const { return m_windowAttr.at(1); }
  int windowAttrB() const { return m_windowAttr.at(2); }
  int windowAttrA() const { return m_windowAttr.at(3); }
  int windowAttrF() const { return m_windowAttr.at(4); }

  virtual void setWindowAttrR(int i) { m_windowAttr.at(0) = i; }
  virtual void setWindowAttrG(int i) { m_windowAttr.at(1) = i; }
  virtual void setWindowAttrB(int i) { m_windowAttr.at(2) = i; }
  virtual void setWindowAttrA(int i) { m_windowAttr.at(3) = i; }
  virtual void setWindowAttrF(int i) { m_windowAttr.at(4) = i; }
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
  virtual void setMousePosition(RLMachine& machine, int x, int y) = 0;
  virtual bool handleMouseClick(RLMachine& machine, int x, int y,
                                bool pressed) = 0;
  /// @}
};

#endif



