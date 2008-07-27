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

#ifndef __TextWindow_hpp__
#define __TextWindow_hpp__

#include "Systems/Base/Rect.hpp"
#include "Systems/Base/Colour.hpp"

#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/ptr_container/ptr_map.hpp>

class Point;
class RLMachine;
class Gameexe;
class GameexeInterpretObject;
class GraphicsSystem;
class TextWindowButton;

/**
 * Abstract representation of a TextWindow. Aggrigated by @c TextSystem,
 * and rendered in conjunction with @c GraphicsSystem.
 *
 * Sets of TextWindows should be reconstructable by the state in @c
 * TextPage , though there are some notable exceptions, specifically
 * @c Select_LongOperation . 
 *
 * This class has all sorts of complex, rarely used text rendering
 * options, including multiple co-ordinate systems, which I'm sure was
 * done to give reverse engineers a headache.
 */
class TextWindow
{
protected:
  /// We cache the size of the screen so we don't need the machine in
  /// some accessors.
  int m_screenWidth, m_screenHeight;

  /**
   * @name Text window Origin
   * Describes the origin point of the window
   * 
   * @{
   */
  int m_windowPositionOrigin;
  int m_windowPositionX;
  int m_windowPositionY;
  /// @}

  /** 
   * @name Insertion point 
   *
   * The text insertion point. These two numbers are relative to the
   * text window location and represent the top left corner of where
   * the next piece of text should be inserted.
   *
   * @{
   */
  int m_textInsertionPointX;
  int m_textInsertionPointY;

  /// The line number in this text window; used to detect whether we
  /// have filled this text box
  int m_currentLineNumber;

  /// The initial value of m_textInsertionPointY on new lines.
  int m_currentIndentationInPixels;

  /// @}

  /**
   * @name Text output properties
   * 
   * @{
   */

  /// The current size of the font
  int m_fontSizeInPixels;

  /// The current size of the ruby text in pixels
  int m_rubySize;

  /// Size of the window in characters
  int m_xWindowSizeInChars, m_yWindowSizeInChars;

  /// Spacing between characters
  int m_xSpacing, m_ySpacing;

  /// Whether to indent (INDENT_USE)
  int m_useIndentation;

  /// The default color. Initialized to \#COLOR_TABLE.000, but can be
  /// changed with the SetFontColour() command.
  RGBColour m_defaultColor;

  /// The current color. Initialized to the default color on every
  /// clearWin() call. 
  RGBColour m_fontColour;

  /// @}

  /// Determines how the window will react to pause()
  /// calls. Initialized to \#WINDOW.x.R_COMMAND_MOD.
  int m_actionOnPause;

  /**
   * @name Positional data
   * 
   * @{
   */
  int m_origin, m_xDistanceFromOrigin, m_yDistanceFromOrigin;

  /// @}

  int m_upperBoxPadding, m_lowerBoxPadding, m_leftBoxPadding, m_rightBoxPadding;

  /// Whether m_r, etc is a per-window color.
  int m_windowAttrMod;

  /// The default window background color.
  /// @{
  RGBAColour m_colour;
  int m_filter;
  /// @}

  int m_isVisible;

  /// Determines the position of the keycursor (the animated cursor
  /// that appears when the game is waiting for a click to move to the
  /// next page of text).
  ///
  /// If type is 0, the cursor appears at the bottom right corner of
  /// the text area; if it is 1, it appears directly after the final
  /// character printed; if it is 2, it appears at (x, y) relative to
  /// the top left of the text area. (x and y are ignored when type is
  /// 0 or 1.)
  ///
  /// @{
  int m_keycursorType;
  Point m_keycursorPos;
  /// @}

  /** 
   * @name Name display options
   * 
   * Options related to the display of the current speaker's name.
   */

  /// Describes how to render character names.
  /// - 0: Display names inline (default)
  /// - 1: Display names in a seperate window
  /// - 2: Do not display names
  int m_nameMod;

  /// @}

  /**
   * @name Buttons in this text box
   * 
   * Attached action buttons defined in the
   * \#WAKU.index1.index2.XXX_BOX properties. These actions represent
   * things such as moving the text box, clearing the text box, moving
   * forward or backwards in message history, and farcall()-ing a
   * custom handler (EXBTN_index_BOX).
   *
   * @{
   */
  typedef boost::ptr_map<std::string, TextWindowButton> ButtonMap;
  ButtonMap m_buttonMap;
  /// @}

  /**
   * @name Selection mode data
   * 
   * Text boxes can be in selection mode, in which case a
   * Select_LongOperation is on the top of the RLMachine's call stack
   * and has
   *
   * @{
   */
  /// Whether this text box currently contains
  bool m_inSelectionMode;

  /// Callback function for when item is selected; usually will call a
  /// specific method on Select_LongOperation
  boost::function<void(int)> m_selectionCallback;
  
  /// Used to assign a zero based index to all selection elements
  /// added by addSelectionItem().
  int m_nextId;
  /// @}

protected:
  /**
   * Accessor for the m_selectionCallback for TextWindow subclasses
   */
  const boost::function<void(int)>& selectionCallback();

public:
  TextWindow(RLMachine& machine, int windowNum);
  virtual ~TextWindow();

  virtual void execute(RLMachine& machine);

  /**
   * @name Text size and location
   * 
   * Accessors dealing with the size and location of the text
   * window. 
   *
   * @{
   */

  /**
   * Sets the size of the text window in characters. Reprsented by
   * \#WINDOW.xxx.MOJI_CNT.
   */
  void setWindowSizeInCharacters(const std::vector<int>& posData);
  int xWindowSizeInChars() const { return m_xWindowSizeInChars; }
  int yWindowSizeInChars() const { return m_yWindowSizeInChars; }

  /**
   * Sets the size of the spacing between characters. Reprsented by
   * \#WINDOW.xxx.MOJI_REP.
   */
  void setSpacingBetweenCharacters(const std::vector<int>& posData);
  int xSpacing() const { return m_xSpacing; }
  int ySpacing() const { return m_ySpacing; }

  /**
   * Sets the size of the ruby (furigana; pronounciation guide) text
   * in pixels. If zero, ruby text is disabled in this
   * window. Represented by \#WINDOW.xxx.LUBY_SIZE.
   */
  void setRubyTextSize(const int i) { m_rubySize = i; }
  int rubyTextSize() const { return m_rubySize; }

  /** 
   * Sets the size of the font. Reprsented by \#WINDOW.xxx.MOJI.SIZE.
   */
  void setFontSizeInPixels(int i) { m_fontSizeInPixels = i; }
  int fontSizeInPixels() const { return m_fontSizeInPixels; }

  void setTextboxPadding(const std::vector<int>& posData);

  void setUseIndentation(const int i) { m_useIndentation = i; }

  void setDefaultTextColor(const std::vector<int>& colorData);
  void setFontColor(const std::vector<int>& colorData);

  /// @}

  /**
   * @name Window Positional 
   * 
   * @{
   */
  void setWindowPosition(const std::vector<int>& posData);

  Size textWindowSize() const;

  int boxX1() const;
  int boxY1() const;

  int textX1() const;
  int textY1() const;
  int textX2() const;
  int textY2() const;

  /// @}

  virtual void setMousePosition(RLMachine& machine, const Point& pos);
  virtual bool handleMouseClick(RLMachine& machine, const Point& pos, bool pressed);

  /**
   * @name Name window settings
   * 
   * @{
   */

  /// Sets how the name is displayed
  /// @see m_nameMod
  void setNameMod(const int in) { m_nameMod = in; }

  /// @}

  /**
   * @name Keycursor settings
   * 
   * Set and access the position of the animated icon that appears
   * when waiting for user input. The TextWindow only owns the
   * position which is queried by the TextKeyCursor object which owns
   * the actual image and other properties. This is an artifact of how
   * RealLive handles this.
   *
   * @see TextKeyCursor
   *
   * @{
   */
  void setKeycurMod(const std::vector<int>& keycur);

  Point keycursorPosition() const;
  /// @}

  /**
   * @name Waku (Window decoration) Handling Functions
   * 
   * @{
   */

  void setWindowWaku(RLMachine& machine, Gameexe& gexe, const int wakuNo);


  virtual void setWakuMain(RLMachine& machine, const std::string& name) = 0;

  /** 
   * Loads the graphics file name as the mask for represents the areas
   * of the text window that should be shaded.
   */
  virtual void setWakuBacking(RLMachine& machine, const std::string& name) = 0;

  /**
   * Loads the graphics file name as the image with all the button
   * images used when drawing 
   */
  virtual void setWakuButton(RLMachine& machine, const std::string& name) = 0;  

  /// @}


  /**
   * @name Window Background Color Attributes
   * 
   * Accessors regarding the background color of the window.
   *
   * Represents the data parsed from \#WINDOW_ATTR,
   * \#WINDOW.index.ATTR_MOD, and \#WINDOW.index.ATTR
   *
   * @{
   */
  void setWindowAttrMod(int i) { m_windowAttrMod = i; }
  int windowAttrMod() const { return m_windowAttrMod; }

  void setR(int i) { m_colour.setRed(i); }
  void setG(int i) { m_colour.setGreen(i); }
  void setB(int i) { m_colour.setBlue(i); }
  void setAlpha(int i) { m_colour.setAlpha(i); }
  void setFilter(int i) { m_filter = i; }
  void setRGBAF(const std::vector<int>& rgbaValues);
  
  int r() const { return m_colour.r(); }
  int g() const { return m_colour.g(); }
  int b() const { return m_colour.b(); }
  int alpha() const { return m_colour.a(); }
  int filter() const { return m_filter; }
  /// @}

  void setVisible(int in) { m_isVisible = in; }
  bool isVisible() const { return m_isVisible; }

  void setActionOnPause(const int i) { m_actionOnPause = i; }
  bool actionOnPause() const { return m_actionOnPause; }

  // ------------------------------------------------ [ Abstract interface ]
  virtual void render(RLMachine& machine) = 0;

  /** 
   * Clears the text window of all text and resets the insertion
   * point.
   */
  virtual void clearWin() = 0;

  /**
   * Displays one character, and performs line breaking logic based on
   * the next character.
   *
   * @return True if the character fits on the screen. False if it
   *         does not and was not displayed.
   */
  virtual bool displayChar(RLMachine& machine, const std::string& current,
                           const std::string& next) = 0;

  /**
   * Returns whether another character can be placed on the screen.
   */
  virtual bool isFull() const = 0;

 
  /** 
   *  Write this later.
   */
 //  virtual void resetName() = 0;
 
  /**
   * Sets (and displays, if appropriate) the name of the current speaker.
   */
  virtual void setName(RLMachine& machine, const std::string& utf8name, 
                       const std::string& nextChar) = 0;

  virtual void hardBrake() = 0;
  virtual void resetIndentation() = 0;

  virtual void markRubyBegin() = 0;
  virtual void displayRubyText(RLMachine& machine, const std::string& utf8str) = 0;


  /**
   * @name Selection Mode
   * 
   * Text Windows are responsible for presenting the questions from
   * select() and select_s() calls. (select_w() is not done here.)
   *
   * @{
   */
  virtual void startSelectionMode();

  bool inSelectionMode() { return m_inSelectionMode; }
  int getNextSelectionID() { return m_nextId++; }

  virtual void addSelectionItem(const std::string& utf8str) = 0;
  virtual void setSelectionCallback(const boost::function<void(int)>& func);

  virtual void endSelectionMode();
  /// @}
};

#endif
