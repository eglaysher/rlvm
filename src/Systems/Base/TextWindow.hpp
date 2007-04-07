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

#ifndef __TextWindow_hpp__
#define __TextWindow_hpp__

#include <vector>

class RLMachine;
class Gameexe;
class GraphicsSystem;

/**
 * Abstract representation of a TextWindow. Aggrigated by TextSystem,
 * and rendered in conjunction with GraphicsSystem.
 *
 * This class has all sorts of complex, rarely used text rendering
 * options, including several co-ordinate systems, which I'm sure was
 * done to give me a headache.
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

  /** @name Insertion point 
   * The text insertion point. These two numbers are relative to the
   * text window location. 
   *
   * @{
   */
  int m_textInsertionPointX;
  int m_textInsertionPointY;
  int m_currentLineNumber;
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

  int m_currentIndentationInPixels;

  /// The default color. Initialized to #COLOR_TABLE.000, but can be
  /// changed with the SetFontColour() command
  int m_defaultRed, m_defaultGreen, m_defaultBlue;

  /// The current color. Initialized to the default color on every
  /// clearWin() call. 
  int m_fontRed, m_fontGreen, m_fontBlue;

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

  /// The default window background color.
  /// @{
  int m_r, m_g, m_b, m_alpha, m_filter;
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
  int m_keycursorType, m_keycursorX, m_keycursorY;
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
   * @name Waku (window decorations)
   * 
   * @{
   */
  

  /// @}

protected:

  /// Internal calculations stuff

public:
  TextWindow(RLMachine& machine, int windowNum);

  virtual ~TextWindow() {}

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

  int textWindowWidth() const;
  int textWindowHeight() const;

  int boxX1() const;
  int boxY1() const;

  int textX1() const;
  int textY1() const;
  int textX2() const;
  int textY2() const;

  /// @}

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

  int keycursorX() const;
  int keycursorY() const;
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
   * 
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
  void setR(int i) { m_r = i; }
  void setG(int i) { m_g = i; }
  void setB(int i) { m_b = i; }
  void setAlpha(int i) { m_alpha = i; }
  void setFilter(int i) { m_filter = i; }
  void setRGBAF(const std::vector<int>& rgbaValues);
  
  int r() const { return m_r; }
  int g() const { return m_g; }
  int b() const { return m_b; }
  int alpha() const { return m_alpha; }
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
};

#endif
