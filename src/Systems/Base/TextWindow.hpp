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
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/ptr_container/ptr_map.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

class Gameexe;
class GameexeInterpretObject;
class GraphicsSystem;
class Point;
class RLMachine;
class SelectionElement;
class Surface;
class System;
class TextWaku;
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
class TextWindow {
 public:
  TextWindow(System& system, int window_num);
  virtual ~TextWindow();

  virtual void execute();

  int windowNumber() const { return window_num_; }

  // TODO: This is nowhere near good enough and handling waku better needs to
  // be put on the long term goal list. Should wakuSet() live on the
  // TextWindow? O RLY?
  int wakuSet() const { return waku_set_; }

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
  void setWindowSizeInCharacters(const std::vector<int>& pos_data);
  int xWindowSizeInChars() const { return x_window_size_in_chars_; }
  int yWindowSizeInChars() const { return y_window_size_in_chars_; }

  /**
   * Sets the size of the spacing between characters. Reprsented by
   * \#WINDOW.xxx.MOJI_REP.
   */
  void setSpacingBetweenCharacters(const std::vector<int>& pos_data);
  int xSpacing() const { return x_spacing_; }
  int ySpacing() const { return y_spacing_; }

  /**
   * Sets the size of the ruby (furigana; pronounciation guide) text
   * in pixels. If zero, ruby text is disabled in this
   * window. Represented by \#WINDOW.xxx.LUBY_SIZE.
   */
  void setRubyTextSize(const int i) { ruby_size_ = i; }
  int rubyTextSize() const { return ruby_size_; }

  /**
   * Sets the size of the font. Reprsented by \#WINDOW.xxx.MOJI.SIZE.
   */
  void setFontSizeInPixels(int i) { font_size_in_pixels_ = i; }
  int fontSizeInPixels() const { return font_size_in_pixels_; }

  void setTextboxPadding(const std::vector<int>& pos_data);

  void setUseIndentation(const int i) { use_indentation_ = i; }
  int useIndentation() const { return use_indentation_; }

  void setIndentation();
  int currentIndentation() const { return current_indentation_in_pixels_; }

  void setDefaultTextColor(const std::vector<int>& colour_data);
  virtual void setFontColor(const std::vector<int>& colour_data);

  /// @}

  /**
   * @name Window Positional
   *
   * @{
   */
  void setWindowPosition(const std::vector<int>& pos_data);

  Size textWindowSize() const;

  int boxX1() const;
  int boxY1() const;
  Size boxSize() const;
  int textX1() const;
  int textY1() const;
  int textX2() const;
  int textY2() const;

  int nameboxX1() const;
  int nameboxY1() const;
  Size nameboxSize();
  /// @}

  // TODO: What's setMousePosition and how does it differ from mouse listeners?
  virtual void setMousePosition(const Point& pos);
  virtual bool handleMouseClick(RLMachine& machine, const Point& pos, bool pressed);

  /**
   * @name Name window settings
   *
   * @{
   */

  /// Sets how the name is displayed
  /// @see name_mod_
  void setNameMod(const int in) { name_mod_ = in; }
  int nameMod() const { return name_mod_; }

  /**
   * Sets the size of the spacing between characters. Reprsented by
   * \#WINDOW.xxx.NAME_MOJI_REP.
   */
  void setNameSpacingBetweenCharacters(const std::vector<int>& pos_data);
  int nameXSpacing() const { return name_x_spacing_; }

  void setNameboxPadding(const std::vector<int>& pos_data);
  void setNameboxPosition(const std::vector<int>& pos_data);

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

  void setWindowWaku(const int waku_no);


  /// @}


  /**
   * @name Window Background Color Attributes
   *
   * Accessors regarding the background colour of the window.
   *
   * Represents the data parsed from \#WINDOW_ATTR,
   * \#WINDOW.index.ATTR_MOD, and \#WINDOW.index.ATTR
   *
   * @{
   */
  void setWindowAttrMod(int i) { window_attr_mod_ = i; }
  int windowAttrMod() const { return window_attr_mod_; }

  void setR(int i) { colour_.setRed(i); }
  void setG(int i) { colour_.setGreen(i); }
  void setB(int i) { colour_.setBlue(i); }
  void setAlpha(int i) { colour_.setAlpha(i); }
  void setFilter(int i) { filter_ = i; }
  void setRGBAF(const std::vector<int>& rgba_values);

  int r() const { return colour_.r(); }
  int g() const { return colour_.g(); }
  int b() const { return colour_.b(); }
  int alpha() const { return colour_.a(); }
  const RGBAColour& colour() const { return colour_; }
  int filter() const { return filter_; }
  /// @}

  void setVisible(int in) { is_visible_ = in; }
  bool isVisible() const { return is_visible_; }

  void setActionOnPause(const int i) { action_on_pause_ = i; }
  bool actionOnPause() const { return action_on_pause_; }

  /**
   * @name Insertion point control
   *
   * @{
   */
  int insertionPointX() const { return text_insertion_point_x_; }
  int insertionPointY() const { return text_insertion_point_y_; }
  void offsetInsertionPointX(int offset) { text_insertion_point_x_ += offset; }
  void offsetInsertionPointY(int offset) { text_insertion_point_y_ += offset; }
  void setInsertionPointX(int x) { text_insertion_point_x_ = x; }
  void setInsertionPointY(int y) { text_insertion_point_y_ = y; }

  int lineHeight() const {
    return font_size_in_pixels_ + y_spacing_ + ruby_size_;
  }
  /// @}

  // ------------------------------------------------ [ Abstract interface ]
  void render(std::ostream* tree);

  /// Returns a surface that is the text.
  virtual boost::shared_ptr<Surface> textSurface() = 0;
  virtual boost::shared_ptr<Surface> nameSurface() = 0;

  /**
   * Clears the text window of all text and resets the insertion
   * point.
   */
  virtual void clearWin();

  /**
   * Displays one character, and performs line breaking logic based on
   * the next character.
   *
   * @return True if the character fits on the screen. False if it
   *         does not and was not displayed.
   */
  virtual bool displayChar(const std::string& current,
                           const std::string& next) = 0;

  /// Returns the width of the unicode codepoint stored in |character|.
  virtual int charWidth(unsigned short codepoint) const = 0;

  /**
   * Returns whether another character can be placed on the screen.
   */
  bool isFull() const;

  /**
   *  Write this later.
   */
 //  virtual void resetName() = 0;

  /// Called when a PauseLongOperation happens.
  void pause();

  /**
   * Sets (and displays, if appropriate) the name of the current speaker.
   */
  virtual void setName(const std::string& utf8name,
                       const std::string& next_char);
  void setNameWithoutDisplay(const std::string& utf8name);

  virtual void renderNameInBox(const std::string& utf8str) = 0;

  virtual void hardBrake();
  virtual void resetIndentation();
  virtual void markRubyBegin();
  virtual void displayRubyText(const std::string& utf8str) = 0;


  /**
   * @name Selection Mode
   *
   * Text Windows are responsible for presenting the questions from
   * select() and select_s() calls. (select_w() is not done here.)
   *
   * @{
   */
  virtual void startSelectionMode();

  bool inSelectionMode() { return in_selection_mode_; }
  int getNextSelectionID() { return next_id_++; }

  virtual void addSelectionItem(const std::string& utf8str) = 0;
  virtual void setSelectionCallback(const boost::function<void(int)>& func);

  void endSelectionMode();
  /// @}

 protected:
  /**
   * Accessor for the selection_callback_ for TextWindow subclasses
   */
  const boost::function<void(int)>& selectionCallback();

  /// The actual selection items in this TextWindow.
  typedef boost::ptr_vector<SelectionElement> Selections;
  Selections selections_;

  System& system() { return system_; }

 protected:
  /// We cache the size of the screen so we don't need the machine in
  /// some accessors.
  int screen_width_, screen_height_;

  /// Our numeric window identifier.
  int window_num_;

  /// The waku set number.
  int waku_set_;

  /// The window decorations for the text window
  boost::scoped_ptr<TextWaku> textbox_waku_;

  /**
   * @name Insertion point
   *
   * The text insertion point. These two numbers are relative to the
   * text window location and represent the top left corner of where
   * the next piece of text should be inserted.
   *
   * @{
   */
  int text_insertion_point_x_;
  int text_insertion_point_y_;

  /// Current ruby insertion point (or -1 if markRubyBegin() hasn't
  /// been called)
  int ruby_begin_point_;

  /// The line number in this text window; used to detect whether we
  /// have filled this text box
  int current_line_number_;

  /// The initial value of text_insertion_point_y_ on new lines.
  int current_indentation_in_pixels_;

  /// Whether the last token was a setName. This is used to control indentation
  /// for quotes.
  bool last_token_was_name_;

  /// @}

  /**
   * @name Text output properties
   *
   * @{
   */

  /// The current size of the font
  int font_size_in_pixels_;

  /// The current size of the ruby text in pixels
  int ruby_size_;

  /// Size of the window in characters
  int x_window_size_in_chars_, y_window_size_in_chars_;

  /// Spacing between characters
  int x_spacing_, y_spacing_;

  /// Whether to indent (INDENT_USE)
  int use_indentation_;

  /// The default colour. Initialized to \#COLOR_TABLE.000, but can be
  /// changed with the SetFontColour() command.
  RGBColour default_colour_;

  /// The current colour. Initialized to the default colour on every
  /// clearWin() call.
  RGBColour font_colour_;

  /// @}

  /// Determines how the window will react to pause()
  /// calls. Initialized to \#WINDOW.x.R_COMMAND_MOD.
  int action_on_pause_;

  /**
   * @name Positional data
   *
   * @{
   */
  int origin_, x_distance_from_origin_, y_distance_from_origin_;

  /// @}

  int upper_box_padding_, lower_box_padding_, left_box_padding_, right_box_padding_;

  /// Whether r_, etc is a per-window colour.
  int window_attr_mod_;

  /// The default window background colour.
  /// @{
  RGBAColour colour_;
  int filter_;
  /// @}

  int is_visible_;

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
  int keycursor_type_;
  Point keycursor_pos_;
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
  int name_mod_;

  /// Waku set to use for the text box in the case where name_mod_ == 1.
  int name_waku_set_;

  boost::scoped_ptr<TextWaku> namebox_waku_;

  int name_font_size_in_pixels_;

  int name_waku_dir_set_;

  /// Spacing between characters
  int name_x_spacing_;

  int horizontal_namebox_padding_;
  int vertical_namebox_padding_;

  int namebox_x_offset_;
  int namebox_y_offset_;

  int namebox_centering_;

  int minimum_namebox_size_;

  int name_size_;

  // The number of characters in the window (or |minimum_namebox_size_|).
  int namebox_characters_;
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
  bool in_selection_mode_;

  /// Callback function for when item is selected; usually will call a
  /// specific method on Select_LongOperation
  boost::function<void(int)> selection_callback_;

  /// Used to assign a zero based index to all selection elements
  /// added by addSelectionItem().
  int next_id_;
  /// @}

  System& system_;
};

#endif
